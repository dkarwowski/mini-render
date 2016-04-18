#include <stdbool.h>
#include <float.h>
#include "geometry.h"
#include "tga_img.c"
#include "model.c"

#define MAX(a, b) ((a < b) ? b : a)
#define MIN(a, b) ((a < b) ? a : b)
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
v3f
barycentric(v3f A, v3f B, v3f C, v3f P)
{
    v3f s[2];
    for (int i = 2; i--;) {
        s[i].raw[0] = C.raw[i] - A.raw[i];
        s[i].raw[1] = B.raw[i] - A.raw[i];
        s[i].raw[2] = A.raw[i] - P.raw[i];
    }

    v3f u = CrossV3_float(s[0], s[1]);
    if (fabs(u.raw[2]) > 0.001f)
        return V3_float(1.0f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
    return V3_float(-1.0f, 1.0f, 1.0f);
}

static
void
triangle(TGA_Image *image_p, v3f pts[3], float *zbuffer, TGA_Color color)
{
    v2f bboxmin = V2_float(FLT_MAX, FLT_MAX);
    v2f bboxmax = V2_float(FLT_MIN, FLT_MIN);
    v2f clamp = V2_float(image_p->width - 1, image_p->height - 1);

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            bboxmin.raw[j] = MAX(0.0f,         MIN(bboxmin.raw[j], pts[i].raw[j]));
            bboxmax.raw[j] = MIN(clamp.raw[j], MAX(bboxmax.raw[j], pts[i].raw[j]));
        }
    }

    v3f P;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
            v3f bc_screen = barycentric(pts[0], pts[1], pts[2], P);
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;

            P.z = 0;
            for (int i = 0; i < 3; i++)
                P.z += pts[i].raw[2] * bc_screen.raw[i];

            if (zbuffer[(int)(P.x + P.y * width)] < P.z) {
                zbuffer[(int)(P.x + P.y * width)] = P.z;
                TGA_ImageSet(image_p, P.x, P.y, color);
            }
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

    int zbuffer[width*height];

    srand(5);
    TGA_Image image = TGA_ImageInit(width, height, RGB);
    for (struct ll_node_v3i *face = model_p->faces_.first; face; face = face->next) {
        v3f s_coords[3];
        v3f w_coords[3];
        for (int j = 0; j < 3; j++) {
            v3f v = GetLL_v3f(&model_p->verts_, face->data.raw[j])->data;
            int x = (v.x + 1.0f) * width / 2.0f;
            int y = (v.y + 1.0f) * height / 2.0f;
            s_coords[j] = V3_float(x, y, v.z);
            w_coords[j] = v;
        }
        v3f n = CrossV3_float(SubV3_float(w_coords[2], w_coords[0]), SubV3_float(w_coords[1], w_coords[0]));
        n = NormV3_float(n);
        float intensity = DotV3_float(n, V3_float(0, 0, -1.0f));
        if (intensity > 0)
            triangle(&image, s_coords, (float *)zbuffer, TGA_ColorInit(intensity*255, intensity*255, intensity*255, 255));
    }
    TGA_ImageFlipVertically(&image);
    TGA_ImageWriteFile(&image, "output.tga", true);

    ModelDelete(model_p);
    return 0;
}
