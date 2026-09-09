#include <stdlib.h>
#include <stdbool.h>

/// Adds metadata to the pre_info.<br>
/// <c>value</c> will be copied internally, so you should free your input once you're done with it.<br>
/// Call <see cref="free_metadata"/> in MIV's cleanup() function to free the duplicated string.
void add_to_metadata(Pre_Rendering_Info *pre_info, char* key, char* value) {
	pre_info->metadata_count += 1;
	pre_info->metadata = realloc(pre_info->metadata, pre_info->metadata_count*sizeof(string[2]));
	pre_info->metadata[pre_info->metadata_count-1][0] = to_string(key);
	pre_info->metadata[pre_info->metadata_count-1][1] = to_string(strdup(value)); //copy string, will be freed↓
}

void free_metadata(Pre_Rendering_Info* pre_info) {
	if (pre_info->metadata != NULL) {
		for (int64_t i = 0; i < pre_info->metadata_count; i++) {
			free(pre_info->metadata[i][1].data); //free the copied↑ string
		}
		free(pre_info->metadata);
		pre_info->metadata = NULL;
		pre_info->metadata_count = 0;
	}
}
