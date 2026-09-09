#include "MIV/plugins/MIV.h"
#include "common.h"

#include "stb/stb_image.h"

const int TOTAL_IMAGE_FORMATS = 10;
static int called_n_times;

int64_t registration_procedure(Plugin_Registration_Entry *registration) {
	// Magic Numbers from: https://en.wikipedia.org/wiki/List_of_file_signatures
	switch (called_n_times) {
	case 0:
		registration->name_of_filetype = to_string("Joint Photographic Experts Group");
		registration->extension = to_string("JPG");
		registration->magic_number = to_string("\xFF\xD8\xFF");
		break;
	case 1:
		registration->name_of_filetype = to_string("Joint Photographic Experts Group");
		registration->extension = to_string("JPEG");
		registration->magic_number = to_string("\xFF\xD8\xFF");
		break;
	case 2:
		registration->name_of_filetype = to_string("Joint Photographic Experts Group");
		registration->extension = to_string("JFIF");
		registration->magic_number = to_string("\xFF\xD8\xFF");
		break;
	case 3:
		registration->name_of_filetype = to_string("Portable Network Graphics");
		registration->extension = to_string("PNG");
		registration->magic_number = to_string("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A");
		break;
	case 4:
		registration->name_of_filetype = to_string("Microsoft BitMap");
		registration->extension = to_string("BMP");
		registration->magic_number = to_string("\x42\x4D");
		break;
	case 5:
		registration->name_of_filetype = to_string("Photoshop Document");
		registration->extension = to_string("PSD");
		registration->magic_number = to_string("\x38\x42\x50\x53");
		break;
	case 6:
		registration->name_of_filetype = to_string("Truevision TGA");
		registration->extension = to_string("TGA");
		//registration->magic_number = TGA has no magic number...
		break;
	case 7:
		registration->name_of_filetype = to_string("Graphics Interchange Format");
		registration->extension = to_string("GIF");
		registration->magic_number = to_string("\x47\x49\x46\x38");
		break;
	case 8:
		registration->name_of_filetype = to_string("Portable Pixmap Format");
		registration->extension = to_string("PPM");
		registration->magic_number = to_string("\x50\x36\x0A"); //only binary format is supported (P6). Ascii format (P3) is not supported.
		break;
	case 9:
		registration->name_of_filetype = to_string("Portable Graymap Format");
		registration->extension = to_string("PGM");
		registration->magic_number = to_string("\x50\x35\x0A"); //only binary format is supported (P5). Ascii format (P2) is not supported.
		break;
	}

	registration->procedure_prefix = (string){0};
	registration->has_settings = false;

	called_n_times += 1;
	return TOTAL_IMAGE_FORMATS - called_n_times;
}

Log pre_render(Pre_Rendering_Info *pre_info) {
	int x,y,n;
	int ok = stbi_info_from_file(pre_info->fileptr, &x, &y, &n);
	// returns ok=1 and sets x,y,n if image is a supported format, returns 0 otherwise.
	if (ok == false) {
		char *err = (char*)stbi_failure_reason();
		return (Log){
			.type = LOG_TYPE_ERROR,
			.message = to_string(err),
		};
	}

	pre_info->width = x;
	pre_info->height = y;
	pre_info->bit_depth = 8;
	pre_info->channels = n;

	return (Log){0};
}

Log render(Pre_Rendering_Info *pre_info, Rendering_Info *render_info) {
	const int desired_channels = 4;
	int x,y;
	unsigned char *data = stbi_load_from_file(pre_info->fileptr, &x, &y, NULL, desired_channels);
	// ... process data if not NULL ...
	if (data == NULL) {
		char *err = (char*)stbi_failure_reason();
		return (Log){
			.type = LOG_TYPE_ERROR,
			.message = to_string(err),
		};
	}
	// ... x = width, y = height, n = # 8-bit components per pixel ...
	// ... replace '0' with '1'..'4' to force that many components per pixel
	// ... but 'n' will always be the number that it would have been if you said 0

	memcpy(render_info->buffer, data, x * y * desired_channels);

	stbi_image_free(data);

	return (Log){0};
}

Log cleanup(Pre_Rendering_Info *pre_info) {
	(void)pre_info; //shut up about the unused variable...
	return (Log){0};
}
