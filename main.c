#include <stdbool.h>
#include "geometry.h"
#include "tga_img.c"
#include "model.c"

#define swap(a, b) do {typeof(a) TEMP = a; a = b; b = TEMP;} while (0)

const TGA_Color white = TGA_ColorInit(255, 255, 255, 255);
const TGA_Color red   = TGA_ColorInit(255,   0,   0, 255);
const TGA_Color blue  = TGA_ColorInit(  0, 255,   0, 255);
const TGA_Color green = TGA_ColorInit(  0,   0, 255, 255);
struct model *model_p = NULL;
const int width = 800;
const int height = 800;

static
void
line(TGA_Image *image_p, v2i t0, v2i t1, TGA_Color color)
{
    bool steep = false;
    if (fabs(t0.x - t1.x) < fabs(t0.y - t1.y)) {
        swap(t0.x, t0.y);
        swap(t1.x, t1.y);
        steep = true;
    }

    if (t0.x > t1.x)
        swap(t0, t1);

    int dy = t1.y - t0.y;
    int dx = t1.x - t0.x;
    int derror2 = fabs(dy)*2;
    int error2 = 0;
    int y = t0.y;

    for (int x = t0.x; x <= t1.x; x++) {
        bool imageSet;
        if (steep) 
            imageSet = TGA_ImageSet(image_p, y, x, color);
        else
            imageSet = TGA_ImageSet(image_p, x, y, color);
        if (!imageSet) {
            fprintf(stderr, "Can't set pixel at %d, %d\n", x, y);
        }

        error2 += derror2;
        if (error2 > dx) {
            y += (t1.y > t0.y ? 1 : -1);
            error2 -= dx * 2;
        }
    }
}

static
void
triangle(TGA_Image *image_p, v2i t0, v2i t1, v2i t2, TGA_Color color)
{
    if (t0.y == t1.y && t0.y == t2.y) return;
    if (t0.y > t1.y) swap(t0, t1);
    if (t0.y > t2.y) swap(t0, t2);
    if (t1.y > t2.y) swap(t1, t2);

    int t_height = t2.y - t0.y;
    for (int i = 0; i < t_height; i++) {
        bool second_half = i > t1.y - t0.y || t1.y == t0.y;
        int seg_height = second_half ? t2.y - t1.y : t1.y - t0.y;
        float alpha = (float)i/t_height;
        float beta  = (float)(i - (second_half ? t1.y - t0.y : 0))/seg_height;
        v2i a = AddV2_int(t0, MulV2_int(alpha, SubV2_int(t2, t0)));
        v2i b = second_half 
            ? AddV2_int(t1, MulV2_int(beta, SubV2_int(t2, t1))) 
            : AddV2_int(t0, MulV2_int(beta, SubV2_int(t1, t0)));
        if (a.x > b.x) swap(a,b);
        for (int j = a.x; j <= b.x; j++) {
            TGA_ImageSet(image_p, j, t0.y + i, color);
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

    srand(5);
    TGA_Image image = TGA_ImageInit(width, height, RGB);
    for (struct ll_node_v3i *face = model_p->faces_.first; face; face = face->next) {
        v2i s_coords[3];
        v3f w_coords[3];
        for (int j = 0; j < 3; j++) {
            v3f v = GetLL_v3f(&model_p->verts_, face->data.raw[j])->data;
            int x = (v.x + 1.0f) * width / 2.0f;
            int y = (v.y + 1.0f) * height / 2.0f;
            s_coords[j] = V2_int(x, y);
            w_coords[j] = v;
        }
        v3f n = CrossV3_float(SubV3_float(w_coords[2], w_coords[0]), SubV3_float(w_coords[1], w_coords[0]));
        n = NormV3_float(n);
        float intensity = DotV3_float(n, V3_float(0, 0, -1.0f));
        if (intensity > 0)
            triangle(&image, s_coords[0], s_coords[1], s_coords[2], TGA_ColorInit(intensity*255, intensity*255, intensity*255, 255));
    }
    TGA_ImageFlipVertically(&image);
    TGA_ImageWriteFile(&image, "output.tga", true);

    TGA_Image scene = TGA_ImageInit(width, height, RGB);
    line(&scene, V2_int(20, 34), V2_int(744, 300), red);
    line(&scene, V2_int(120, 434), V2_int(444, 400), green);
    line(&scene, V2_int(330, 463), V2_int(594, 200), blue);
    line(&scene, V2_int(10, 10), V2_int(790, 10), white);
    TGA_ImageFlipVertically(&scene);
    TGA_ImageWriteFile(&scene, "scene.tga", true);

    ModelDelete(model_p);
    return 0;
}
