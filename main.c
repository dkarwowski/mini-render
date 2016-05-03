#include <stdbool.h>
#include <float.h>
#include <unistd.h>
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

static
void
line(TGA_Image *image, v2i t0, v2i t1, TGA_Color color)
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
            imageSet = TGA_ImageSet(image, y, x, color);
        else
            imageSet = TGA_ImageSet(image, x, y, color);
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
triangle(TGA_Image *image, v3f pts[3], float *zbuffer, TGA_Color color)
{
    v2f bboxmin = V2_float(FLT_MAX, FLT_MAX);
    v2f bboxmax = V2_float(FLT_MIN, FLT_MIN);
    v2f clamp = V2_float(image->width - 1, image->height - 1);

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

            // compute a normal value
            v3f w_pts[3];
            for (int i = 0; i < 3; i++)
                w_pts[i] = V3_float(pts[i].x * 2.0f / image->width - 1.0f, pts[i].y * 2.0f / image->height - 1.0f, pts[i].z);
            v3f normal = CrossV3_float(SubV3_float(w_pts[2], w_pts[0]), SubV3_float(w_pts[1], w_pts[0]));
            normal = NormV3_float(normal);
            float intensity = DotV3_float(normal, V3_float(0.0, 0.0, -0.95f));
            
            TGA_Color c = color;
            if (intensity > 0.0f) {
                c = TGA_ColorInit(
                        intensity * color.r,
                        intensity * color.g,
                        intensity * color.b,
                        color.a);
            }

            if (zbuffer[(int)(P.x + P.y * image->width)] < P.z) {
                zbuffer[(int)(P.x + P.y * image->width)] = P.z;
                TGA_ImageSet(image, P.x, P.y, c);
            }
        }
    }
}

static
void
textureMap(struct model *model, TGA_Image *image, v3f s_pts[3], v2f t_pts[3], float *zbuffer)
{
    v2f bboxmin = V2_float(FLT_MAX, FLT_MAX);
    v2f bboxmax = V2_float(FLT_MIN, FLT_MIN);
    v2f clamp = V2_float(image->width - 1, image->height - 1);

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 2; j++) {
            bboxmin.raw[j] = MAX(0.0f,         MIN(bboxmin.raw[j], s_pts[i].raw[j]));
            bboxmax.raw[j] = MIN(clamp.raw[j], MAX(bboxmax.raw[j], s_pts[i].raw[j]));
        }
    }

    v3f P;
    TGA_Color color;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
            v3f bc_screen = barycentric(s_pts[0], s_pts[1], s_pts[2], P);
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;

            P.z = 0;
            for (int i = 0; i < 3; i++)
                P.z += s_pts[i].z * bc_screen.raw[i];

            v2i texture_pts = V2_int(
                    bc_screen.x * t_pts[0].x + bc_screen.y * t_pts[1].x + bc_screen.z * t_pts[2].x,
                    bc_screen.x * t_pts[0].y + bc_screen.y * t_pts[1].y + bc_screen.z * t_pts[2].y);
            color = TGA_ImageGet(&model->texture, texture_pts.x, texture_pts.y);

            // compute a normal value
            v3f w_pts[3];
            for (int i = 0; i < 3; i++)
                w_pts[i] = V3_float(s_pts[i].x * 2.0f / image->width - 1.0f, s_pts[i].y * 2.0f / image->height - 1.0f, s_pts[i].z);
            v3f normal = CrossV3_float(SubV3_float(w_pts[2], w_pts[0]), SubV3_float(w_pts[1], w_pts[0]));
            normal = NormV3_float(normal);
            float intensity = DotV3_float(normal, V3_float(0.0, 0.0, -0.95f));
            
            if (intensity > 0.0f) {
                color = TGA_ColorInit(
                        intensity * color.r,
                        intensity * color.g,
                        intensity * color.b,
                        color.a);
            }

            if ((int)(P.x + P.y * image->width) < 0 || (int)(P.x + P.y * image->width) >= image->width * image->height)
                fprintf(stdout, "out of bounds\n");
            if (zbuffer[(int)(P.x + P.y * image->width)] < P.z) {
                zbuffer[(int)(P.x + P.y * image->width)] = P.z;
                TGA_ImageSet(image, P.x, P.y, color);
            }
        }
    }
}

static
void
render(struct model *model, TGA_Image *image)
{
    int width = image->width;
    int height = image->height;

    float *zbuffer = (float *)malloc(sizeof(float)*width*height);
    for (int i = width * height; i-- ; zbuffer[i] = -FLT_MAX);

    struct ll_face_node *face, *temp;
    LIST_FOR_EACH_ENTRY_SAFE(face, temp, &model->faces_.list.head, head) {
        v2f t_coords[3];
        v3f s_coords[3];
        for (int j = 0; j < 3; j++) {
            v3f *v = LL_V3F_GetIndex(&model->verts_, face->indexes[j].ivert);
            int x = (v->x + 1.0f) * width / 2.0f;
            int y = (v->y + 1.0f) * height / 2.0f;
            s_coords[j] = V3_float(x, y, v->z);
            v3f *t = LL_V3F_GetIndex(&model->textures_, face->indexes[j].iuv);
            t_coords[j] = V2_float(t->x * model->texture.width, t->y * model->texture.height);
        }
        textureMap(model, image, s_coords, t_coords, (float *)zbuffer);
    }
    TGA_ImageFlipVertically(image);

    free(zbuffer);
}

int
main(int argc, char **argv)
{
    struct model model = {0};
    const int width = 800;
    const int height = 800;

    if (2 == argc)
        ModelInit(&model, argv[1]);
    else
        ModelInit(&model, "obj/african_head.obj");

    TGA_Image image = TGA_ImageInit(width, height, RGB);
    render(&model, &image);
    TGA_ImageWriteFile(&image, "output.tga", true);

    TGA_ImageDelete(&image);
    ModelDelete(&model);
    return 0;
}
