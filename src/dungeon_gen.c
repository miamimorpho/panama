
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../extern/stb_image.h"
#include "../extern/stb_image_write.h"

#define WFC_IMPLEMENTATION
#define WFC_USE_STB
#include "../extern/wfc.h"

#include "entity.h"
#include "terminal_types.h"
#include "terra.h"
#include "json.h"

int
dungeonGenerate(struct Dungeon *d, const char *level_file)
{

	cJSON *json = readJson(level_file);
	if (!json)
		return 1;

	cJSON *json_wfc = cJSON_GetObjectItemCaseSensitive(json, "wfc");
	if (!json_wfc || !cJSON_IsString(json_wfc)) {
		return 1;
	}

	char filepath[256];
	snprintf(filepath, sizeof(filepath), "./wfc/%s", json_wfc->valuestring);

	cJSON *json_width = cJSON_GetObjectItemCaseSensitive(json, "width");
	if (!json_width || !cJSON_IsNumber(json_width)) {
		return 1;
	}

	cJSON *json_height = cJSON_GetObjectItemCaseSensitive(json, "height");
	if (!json_height || !cJSON_IsNumber(json_height)) {
		return 1;
	}

	int width = json_width->valueint;
	int height = json_height->valueint;

	struct wfc_image *sample = wfc_img_load(filepath);
	assert(sample && "image does not exist for dungeonGenerate");
	struct wfc *wfc = wfc_overlapping(width, height, sample, 3, 3, 1, 1, 1, 1);
	wfc_run(wfc, -1);
	struct wfc_image *output = wfc_output_image(wfc);

	struct TermTile tile;
	tile.utf = utf8Code(0);
	tile.fg = (Color) {0, 0, 0};
	tile.bg = (Color) {200, 200, 200};

	for (int y = 0; y < output->height; y++) {
		for (int x = 0; x < output->width; x++) {
			size_t i = (y * output->width + x) * output->component_cnt;
			struct TerraPos pos = terraPosNew(d->terrain, (vec16) {x, y});
			if (output->data[i] == 0) {
				tile.utf = utf8Code(0x2593);
				terraPutTile(pos, tile);
				terraPutOpaque(pos, 1);
				terraPutSolid(pos, 1);
			} else {
				tile.utf = utf8Code('.');
				terraPutTile(pos, tile);
			}
		}
	}

	wfc_img_destroy(sample);
	wfc_destroy(wfc);
}
