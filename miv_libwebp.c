#include "MIV/plugins/MIV.h"
#include <stdlib.h>

#include <webp/decode.h>
#include <webp/demux.h>

int64_t registration_procedure(Plugin_Registration_Entry *registration) {
	// Magic Numbers from: https://en.wikipedia.org/wiki/List_of_file_signatures
	registration->name_of_filetype = to_string("WebP");
	registration->extension = to_string("WEBP");
	registration->magic_number = to_string("\x52\x49\x46\x46"); //TODO: This is only the "RIFF" part... How do I specify the "WEBP" part?
	registration->procedure_prefix = (string){0};
	registration->has_settings = 0;
	return 0;
}

void add_to_metadata(Pre_Rendering_Info *pre_info, char* key, char* value) {
	pre_info->metadata_count += 1;
	pre_info->metadata = realloc(pre_info->metadata, pre_info->metadata_count*sizeof(string[2]));
	pre_info->metadata[pre_info->metadata_count-1][0] = to_string(key);
	pre_info->metadata[pre_info->metadata_count-1][1] = to_string(value);
}

typedef struct {
	WebPBitstreamFeatures features;
} PluginData;

Log pre_render(Pre_Rendering_Info *pre_info) {
	PluginData* plugin_data = malloc(sizeof(PluginData));
	pre_info->user_ptr = plugin_data;

	const size_t data_size = 32;
	uint8_t file_data[data_size];
	size_t read_size = fread(file_data, 1, data_size, pre_info->fileptr);

	WebPBitstreamFeatures features;
	VP8StatusCode status = WebPGetFeatures(file_data, read_size, &features);
	plugin_data->features = features;
	if (status != VP8_STATUS_OK) {
		char* errs[8] = {
			"Invalid WebP file header: OK (this should never happen!)",
			"Invalid WebP file header: OUT_OF_MEMORY",
			"Invalid WebP file header: INVALID_PARAM",
			"Invalid WebP file header: BITSTREAM_ERROR",
			"Invalid WebP file header: UNSUPPORTED_FEATURE",
			"Invalid WebP file header: SUSPENDED",
			"Invalid WebP file header: USER_ABORT",
			"Invalid WebP file header: NOT_ENOUGH_DATA",
		};
		return (Log){
			.type = LOG_TYPE_ERROR,
			.message = to_string(errs[status]),
		};
	}

	pre_info->width = features.width;
	pre_info->height = features.height;
	pre_info->bit_depth = 8;
	pre_info->channels = features.has_alpha ? 4 : 3;

	add_to_metadata(pre_info, "animated", features.has_animation ? "yes" : "no");

	char* formats[3] = {"undefined (/mixed)", "lossy", "lossless"};
	add_to_metadata(pre_info, "format", formats[features.format]);

	return (Log){0};
}

Log render(Pre_Rendering_Info *pre_info, Rendering_Info *render_info) {
	PluginData* plugin_data = pre_info->user_ptr;
	WebPBitstreamFeatures* features = &plugin_data->features;

	// How big is the file, anyway
	fseek(pre_info->fileptr, 0, SEEK_END);
	size_t file_size = ftell(pre_info->fileptr);
	rewind(pre_info->fileptr);

	// Read the file into memory
	uint8_t *file_data = malloc(file_size);
	size_t bytes_read = fread(file_data, 1, file_size, pre_info->fileptr);

	// Decode file data to pixel data
	if (!features->has_animation) {
		int width, height;
		uint8_t *data = WebPDecodeRGBA(file_data, bytes_read, &width, &height);
		free(file_data);

		// Process data if not NULL
		if (data == NULL) {
			return (Log){
				.type = LOG_TYPE_ERROR,
				.message = to_string("Invalid WebP file data"),
			};
		}

		memcpy(render_info->buffer, data, width * height * 4);

		WebPFree(data);
	} else {
		// Animated WebP files need a different API to be loaded
		WebPAnimDecoderOptions dec_options;
		WebPAnimDecoderOptionsInit(&dec_options);

		// Tuning 'dec_options' as needed:
		dec_options.color_mode = MODE_RGBA;
		dec_options.use_threads = 1;

		WebPAnimDecoder* dec = WebPAnimDecoderNew(&(WebPData){file_data, bytes_read}, &dec_options);
		if (dec == NULL) {
			return (Log){
				.type = LOG_TYPE_ERROR,
				.message = to_string("Parsing error, invalid option or memory error"),
			};
		}

		uint8_t *buf;
		int timestamp;
		if (WebPAnimDecoderGetNext(dec, &buf, &timestamp)) {
			memcpy(render_info->buffer, buf, features->width * features->height * 4);
		}

		WebPAnimDecoderDelete(dec);
		free(file_data);
	}

	return (Log){0};
}

Log cleanup(Pre_Rendering_Info *pre_info) {
	if (pre_info->user_ptr != NULL) {
		free(pre_info->user_ptr);
		pre_info->user_ptr = NULL;
	}
	if (pre_info->metadata != NULL) {
		free(pre_info->metadata);
		pre_info->metadata = NULL;
		pre_info->metadata_count = 0;
	}
	return (Log){0};
}
