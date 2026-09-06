#include "MIV/plugins/MIV.h"
#include <stdlib.h>

#include <webp/decode.h>

int64_t registration_procedure(Plugin_Registration_Entry *registration) {
	// Magic Numbers from: https://en.wikipedia.org/wiki/List_of_file_signatures
	registration->name_of_filetype = to_string("WebP");
	registration->extension = to_string("WEBP");
	registration->magic_number = to_string("\x52\x49\x46\x46"); //TODO: This is only the "RIFF" part... How do I specify the "WEBP" part?
	registration->procedure_prefix = (string){0};
	registration->has_settings = 0;
	return 0;
}

Log pre_render(Pre_Rendering_Info *pre_info) {
	size_t data_size = 32;
    uint8_t file_data[32];
	size_t read_size = fread(file_data, 1, data_size, pre_info->fileptr);

	int width, height;
	int success = WebPGetInfo(file_data, read_size, &width, &height);
	if (success == 0) {
		return (Log){
			.type = LOG_TYPE_ERROR,
			.message = to_string("Invalid WebP file"),
		};
	}

	pre_info->width = width;
	pre_info->height = height;
	pre_info->bit_depth = 8;
	pre_info->channels = 4; //is that right..?

	return (Log){0};
}

Log render(Pre_Rendering_Info *pre_info, Rendering_Info *render_info) {
	// How big is the file, anyway
	fseek(pre_info->fileptr, 0, SEEK_END);
	size_t file_size = ftell(pre_info->fileptr);
	rewind(pre_info->fileptr);

	// Read the file into memory
	uint8_t *file_data = malloc(file_size);
	size_t bytes_read = fread(file_data, 1, file_size, pre_info->fileptr);

	// Decode file data to pixel data
	int width, height;
	uint8_t *data = WebPDecodeRGBA(file_data, bytes_read, &width, &height);
	free(file_data);

	// Process data if not NULL
	if (data == NULL) {
		return (Log){
			.type = LOG_TYPE_ERROR,
			.message = to_string("Invalid WebP file"),
		};
	}

	memcpy(render_info->buffer, data, width * height * 4);

	WebPFree(data);

	return (Log){0};
}

Log cleanup(Pre_Rendering_Info *pre_info) {
	(void)pre_info; //shut up about the unused variable...
	return (Log){0};
}
