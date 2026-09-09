#include <stdlib.h>
#include <stdbool.h>

void add_to_metadata(Pre_Rendering_Info *pre_info, char* key, char* value) {
	pre_info->metadata_count += 1;
	pre_info->metadata = realloc(pre_info->metadata, pre_info->metadata_count*sizeof(string[2]));
	pre_info->metadata[pre_info->metadata_count-1][0] = to_string(key);
	pre_info->metadata[pre_info->metadata_count-1][1] = to_string(value);
}
