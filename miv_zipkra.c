#include "common.h"
#include <unistd.h>

#include <zip.h>
#include <libxml/parser.h>
#include "stb/stb_image.h"

int64_t registration_procedure(Plugin_Registration_Entry *registration) {
	// Magic Numbers from: https://en.wikipedia.org/wiki/List_of_file_signatures
	registration->name_of_filetype = to_string("Krita Project");
	registration->extension = to_string("KRA");
	registration->magic_number = to_string("\x50\x4B\x03\x04"); //This is a ZIP: https://docs.krita.org/en/general_concepts/file_formats/file_kra.html
	registration->procedure_prefix = (string){0};
	registration->has_settings = false;
	return 0;
}

bool different(const xmlChar *a, char* b) {
	return xmlStrcmp(a, (const xmlChar *) b);
}

bool same(const xmlChar *a, char* b) {
	return !different(a, b);
}

char* attr(xmlNodePtr cur, char* key) {
	return (char*)xmlGetProp(cur, (const xmlChar *)key);
}

typedef struct {
	zip_t *zip;
} PluginData;

Log pre_render(Pre_Rendering_Info *pre_info) {
	PluginData *plugin_data = malloc(sizeof(PluginData));
	pre_info->user_ptr = plugin_data;

	zip_t *zip;
	/* Open ZIP file */ {
		int fd = dup(fileno(pre_info->fileptr));
		int zip_err=0;
		zip = zip_fdopen(fd, 0, &zip_err);
		if (zip == NULL) {
			plugin_data->zip = NULL;
			static char msg[128];
			snprintf(msg, sizeof(msg), "Failed to open KRA file as ZIP (libzip:%d)", zip_err);
			return (Log){
				.type = LOG_TYPE_ERROR,
				.message = to_string(msg),
			};
		}
		plugin_data->zip = zip;
	}

	/* Load metadata from maindoc.xml */ {
		const char* maindocName = "maindoc.xml";
		// How big is the file, anyway
		zip_stat_t sb;
		if (zip_stat(zip, maindocName, 0, &sb)) {
			return (Log){
				.type = LOG_TYPE_ERROR,
				.message = to_string("There is no maindoc.xml in this KRA file (it's probably just a normal ZIP)"),
			};
		}
		size_t file_size = sb.size;

		// Open the file
		zip_file_t *maindocFile = zip_fopen(zip, maindocName, 0);
		// Read the file into memory
		char *file_data = malloc(file_size);
		zip_int64_t bytes_read = zip_fread(maindocFile, file_data, file_size);
		// Close the file
		zip_fclose(maindocFile);

		xmlDocPtr doc = xmlReadMemory(file_data, (int)bytes_read, maindocName, NULL, 0);
		free(file_data);
		if (doc == NULL) {
			return (Log){
				.type = LOG_TYPE_ERROR,
				.message = to_string("Failed to parse maindoc.xml"),
			};
		}

		xmlNodePtr cur = xmlDocGetRootElement(doc);
		if (cur == NULL) {
			xmlFreeDoc(doc);
			return (Log){
				.type = LOG_TYPE_ERROR,
				.message = to_string("maindoc.xml is empty"),
			};
		}

		/* Validate document */ {
			if (different(cur->name, "DOC")) {
				xmlFreeDoc(doc);
				return (Log){
					.type = LOG_TYPE_ERROR,
					.message = to_string("document of the wrong type, root node != DOC"),
				};
			}

			xmlNsPtr rootNamespace = cur->ns;
			if (rootNamespace == NULL || rootNamespace->href == NULL) {
				xmlFreeDoc(doc);
				return (Log){
					.type = LOG_TYPE_ERROR,
					.message = to_string("maindoc.xml's root <DOC> has no namespace"),
				};
			}
			if (different(rootNamespace->href, "http://www.calligra.org/DTD/krita") &&
			    different(rootNamespace->href, "https://www.calligra.org/DTD/krita")) {
				xmlFreeDoc(doc);
				return (Log){
					.type = LOG_TYPE_ERROR,
					.message = to_string("maindoc.xml's root <DOC> has wrong namespace"),
				};
			}
		}

		cur = cur->children;
		while (cur != NULL) {
			if (same(cur->name, "IMAGE")) {
				break;
			}
			cur = cur->next;
		}

		// Width & Height, and other required data
		char* width = attr(cur, "width");
		char* height = attr(cur, "height");

		pre_info->width = atoi(width);
		pre_info->height = atoi(height);
		pre_info->bit_depth = 8;
		pre_info->channels = 4;

		xmlFree(width);
		xmlFree(height);

		// Metadata
		char* colorspacename = attr(cur, "colorspacename");
		char* profile = attr(cur, "profile");

		add_to_metadata(pre_info, "colourspace name", colorspacename);
		add_to_metadata(pre_info, "profile", profile);
		xmlFree(colorspacename);
		xmlFree(profile);

		xmlFreeDoc(doc);
	}

	return (Log){0};
}

Log render(Pre_Rendering_Info *pre_info, Rendering_Info *render_info) {
	PluginData *plugin_data = pre_info->user_ptr;

	const char* mergedimageName = "mergedimage.png";

	// How big is the file, anyway
	zip_stat_t sb;
	if (zip_stat(plugin_data->zip, mergedimageName, 0, &sb)) {
		return (Log){
			.type = LOG_TYPE_ERROR,
			.message = to_string("There is no mergedimage.png in this KRA file (it's probably just a normal ZIP)"),
		};
	}
	size_t file_size = sb.size;

	// Open the file
	zip_file_t *mergedImageFile = zip_fopen(plugin_data->zip, mergedimageName, 0);
	// Read the file into memory
	unsigned char *file_data = malloc(file_size);
	zip_int64_t bytes_read = zip_fread(mergedImageFile, file_data, file_size);
	// Close the file
	zip_fclose(mergedImageFile);

	const int desired_channels = 4;
	int x=0,y=0;
	unsigned char *data = stbi_load_from_memory(file_data, (int)bytes_read, &x, &y, NULL, 4);
	free(file_data);
	if (data == NULL) {
		char *err = (char*)stbi_failure_reason();
		return (Log){
			.type = LOG_TYPE_ERROR,
			.message = to_string(err),
		};
	}

	memcpy(render_info->buffer, data, x * y * desired_channels);

	stbi_image_free(data);

	return (Log){0};
}

Log cleanup(Pre_Rendering_Info *pre_info) {
	if (pre_info->user_ptr != NULL) {
		PluginData *plugin_data = pre_info->user_ptr;
		if (plugin_data->zip != NULL) zip_close(plugin_data->zip);

		free(pre_info->user_ptr);
		pre_info->user_ptr = NULL;
	}
	free_metadata(pre_info);
	return (Log){0};
}
