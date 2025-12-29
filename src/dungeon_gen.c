
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
#include "json_wrapper.h"

int
dungeonGenerate(struct Dungeon *d, const char *level_file)
{
	json_value *root = jsonReadFile(level_file);
	if (!root)
		return 1;

	const char* filepath_ptr = jsonGetString(root, "wfc");

	char filepath[256];
	snprintf(filepath, sizeof(filepath), "./wfc/%s", filepath_ptr);

	int width = *jsonGetInt(root, "width");
	int height = *jsonGetInt(root, "height");

	struct wfc_image *sample = wfc_img_load(filepath);
	assert(sample && "image does not exist for dungeonGenerate");
	struct wfc *wfc = wfc_overlapping(width, height, sample, 3, 3, 1, 1, 1, 1);
	wfc_run(wfc, -1);
	struct wfc_image *output = wfc_output_image(wfc);

	struct TerraTile air = {
		utf8Decomp("."),
		COLOR_FG,
		COLOR_BG,
	};
	
	struct TerraTile wall = {
		utf8Code32(0x2593),
		COLOR_FG,
		COLOR_BG,
	};
	
	for (int y = 0; y < output->height; y++) {
		for (int x = 0; x < output->width; x++) {
			size_t i = (y * output->width + x) * output->component_cnt;
			struct TerraPos pos = terraPosNew(d->terrain, (vec16) {x, y});
			if (output->data[i] == 0) {
				terraPutTile(pos, wall);
				terraPutOpaque(pos, 1);
				terraPutSolid(pos, 1);
			} else {
				terraPutTile(pos, air);
			}
		}
	}

	wfc_img_destroy(sample);
	wfc_destroy(wfc);
}
