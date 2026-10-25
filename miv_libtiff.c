#include "common.h"

#include <tiff.h>
#include <tiffio.h>
#include <unistd.h>

const int TOTAL_IMAGE_FORMATS = 4;
static int called_n_times;

int64_t registration_procedure(Plugin_Registration_Entry *registration) {
	// Magic Numbers from: https://en.wikipedia.org/wiki/List_of_file_signatures
	registration->name_of_filetype = to_string("Tagged Image File Format");

	// All permutations of the file extension and the magic number:
	switch (called_n_times) {
	case 0:
		registration->extension = to_string("TIF");
		registration->magic_number = to_string("\x49\x49\x2A\x00"); //little-endian
		break;
	case 1:
		registration->extension = to_string("TIF");
		registration->magic_number = to_string("\x4D\x4D\x00\x2A"); //big-endian
		break;
	case 2:
		registration->extension = to_string("TIFF");
		registration->magic_number = to_string("\x49\x49\x2A\x00"); //little-endian
		break;
	case 3:
		registration->extension = to_string("TIFF");
		registration->magic_number = to_string("\x4D\x4D\x00\x2A"); //big-endian
		break;
	}

	registration->procedure_prefix = (string){0};
	registration->has_settings = false;

	called_n_times += 1;
	return TOTAL_IMAGE_FORMATS - called_n_times;
}

typedef struct {
	TIFF *tif;
} PluginData;

Log pre_render(Pre_Rendering_Info *pre_info) {
	PluginData *plugin_data = malloc(sizeof(PluginData));
	pre_info->user_ptr = plugin_data;

	int fd = dup(fileno(pre_info->fileptr));
	TIFF *tif = TIFFFdOpen(fd, "filename actually not needed here", "r");
	if (!tif) return (Log){
		.type = LOG_TYPE_ERROR,
		.message = to_string("Failed to open TIFF"),
	};
	plugin_data->tif = tif;

	uint32_t w, h;
	uint16_t b, n, pm;
	TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
	TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
	TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &b);
	TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &n);

	pre_info->width = w;
	pre_info->height = h;
	pre_info->bit_depth = b;
	pre_info->channels = n;

	TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &pm);
	switch (pm) {
		case PHOTOMETRIC_MINISWHITE: add_to_metadata(pre_info, "photometric", "MINISWHITE"); break;
		case PHOTOMETRIC_MINISBLACK: add_to_metadata(pre_info, "photometric", "MINISBLACK"); break;
		case PHOTOMETRIC_RGB: add_to_metadata(pre_info, "photometric", "RGB"); break;
		case PHOTOMETRIC_PALETTE: add_to_metadata(pre_info, "photometric", "PALETTE"); break;
		case PHOTOMETRIC_MASK: add_to_metadata(pre_info, "photometric", "MASK"); break;
		case PHOTOMETRIC_SEPARATED: add_to_metadata(pre_info, "photometric", "SEPARATED"); break;
		case PHOTOMETRIC_YCBCR: add_to_metadata(pre_info, "photometric", "YCBCR"); break;
		case PHOTOMETRIC_CIELAB: add_to_metadata(pre_info, "photometric", "CIELAB"); break;
		case PHOTOMETRIC_ICCLAB: add_to_metadata(pre_info, "photometric", "ICCLAB"); break;
		case PHOTOMETRIC_ITULAB: add_to_metadata(pre_info, "photometric", "ITULAB"); break;
		case PHOTOMETRIC_CFA: add_to_metadata(pre_info, "photometric", "CFA"); break;
		case PHOTOMETRIC_LOGL: add_to_metadata(pre_info, "photometric", "LOGL"); break;
		case PHOTOMETRIC_LOGLUV: add_to_metadata(pre_info, "photometric", "LOGLUV"); break;
	}

	return (Log){0};
}

Log render(Pre_Rendering_Info *pre_info, Rendering_Info *render_info) {
	PluginData *plugin_data = pre_info->user_ptr;

	size_t npixels = pre_info->width * pre_info->height;
	uint32_t *raster = _TIFFmalloc((tmsize_t)(npixels * sizeof(uint32_t)));
	if (raster == NULL) {
		TIFFClose(plugin_data->tif);
		plugin_data->tif = NULL;
		return (Log){
			.type = LOG_TYPE_ERROR,
			.message = to_string("Failed to allocate for TIFF raster"),
		};
	}
	if (TIFFReadRGBAImageOriented(plugin_data->tif, pre_info->width, pre_info->height, raster, ORIENTATION_TOPLEFT, 0)) {
		memcpy(render_info->buffer, raster, npixels * 4);
	} else {
		TIFFClose(plugin_data->tif);
		plugin_data->tif = NULL;
		return (Log){
			.type = LOG_TYPE_ERROR,
			.message = to_string("Failed to read TIFF raster"),
		};
	}
	_TIFFfree(raster);

	return (Log){0};
}

Log cleanup(Pre_Rendering_Info *pre_info) {
	if (pre_info->user_ptr != NULL) {
		PluginData *plugin_data = pre_info->user_ptr;
		if (plugin_data->tif != NULL) TIFFClose(plugin_data->tif);

		free(pre_info->user_ptr);
		pre_info->user_ptr = NULL;
	}
	free_metadata(pre_info);
	return (Log){0};
}
