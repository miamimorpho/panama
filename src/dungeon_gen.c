#include "space.h"
#include "wfc.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

#define WFC_IMPLEMENTATION
#define WFC_USE_STB
#include "wfc.h"


void dungeonGenerate(struct Dungeon *d)
{
    struct wfc_image *sample = 
        wfc_img_load("wfc/test.png");
    assert(sample && "image does not exist for dungeonGenerate");
    struct wfc *wfc = 
        wfc_overlapping(100, 25, sample, 3, 3, 1, 1, 1, 1);
    wfc_run(wfc, -1);
    struct wfc_image *output = wfc_output_image(wfc);

    for(int y = 0; y < output->height; y++){
        for(int x = 0; x < output->width; x++){
            size_t i = (y * output->width + x) * output->component_cnt;
            struct TerraPos pos = terraPosNew(d->space, (vec16){x, y});
            if(output->data[i] == 0){
                terraPutTile(pos, 0x2593);
                terraPutOpaque(pos, 1);
                terraPutSolid(pos, 1);
            }else{
                terraPutTile(pos, '.');
            }
        }
    }

    wfc_img_destroy(sample);
    wfc_destroy(wfc);
}
