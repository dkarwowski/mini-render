#include <stdbool.h>
#include "geometry.h"
#include "tga_img.c"
#include "model.c"

#define swap(a, b) do {typeof(a) TEMP = a; a = b; b = TEMP;} while (0)

const TGA_Color white = TGA_ColorInit(255, 255, 255, 255);
const TGA_Color red   = TGA_ColorInit(255,   0,   0, 255);
struct model *model_p = NULL;
const int width = 800;
const int height = 800;

static 
void
line(TGA_Image *image_p, int x0, int y0, int x1, int y1, TGA_Color color)
{
    bool steep = false;
    if (fabs(x0 - x1) < fabs(y0 - y1)) {
        swap(x0, y0);
        swap(x1, y1);
        steep = true;
    }

    if (x0 > x1) {
        swap(x0, x1);
        swap(y0, y1);
    }

    int dy = y1 - y0;
    int dx = x1 - x0;
    int derror2 = fabs(dy)*2;
    int error2 = 0;
    int y = y0;

    for (int x = x0; x <= x1; x++) {
        if (steep) 
            TGA_ImageSet(image_p, y, x, color);
        else
            TGA_ImageSet(image_p, x, y, color);

        error2 += derror2;
        if (error2 > dx) {
            y += (y1 > y0 ? 1 : -1);
            error2 -= dx * 2;
        }
    }
}

int
main(int argc, char **argv)
{
    if (2 == argc)
        model_p = ModelInit(argv[1]);
    else
        model_p = ModelInit("obj/african_head.obj");

    TGA_Image image = TGA_ImageInit(width, height, RGB);
    for (struct ll_node_v3i *iter = model_p->faces_.first; iter; iter = iter->next) {
        for (int j = 0; j < 3; j++) {
            v3f v0 = GetLL_v3f(&model_p->verts_, iter->data.raw[j])->data;
            v3f v1 = GetLL_v3f(&model_p->verts_, iter->data.raw[(j + 1) % 3])->data;
            int x0 = (v0.x + 1.0f) * width / 2.0f;
            int y0 = (v0.y + 1.0f) * height / 2.0f;
            int x1 = (v1.x + 1.0f) * width / 2.0f;
            int y1 = (v1.y + 1.0f) * height / 2.0f;
            line(&image, x0, y0, x1, y1, white);
        }
    }
    TGA_ImageFlipVertically(&image);
    TGA_ImageWriteFile(&image, "output.tga", true);

    ModelDelete(model_p);
    return 0;
}
