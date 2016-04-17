#include <stdbool.h>
#include "geometry.h"
#include "tga_img.c"
#include "model.c"

#define swap(a, b) do {typeof(a) TEMP = a; a = b; b = TEMP;} while (0)

const TGA_Color white = TGA_ColorInit(255, 255, 255, 255);
const TGA_Color red   = TGA_ColorInit(255,   0,   0, 255);

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
    TGA_Image image = TGA_ImageInit(100, 100, RGB);
    line(&image, 13, 20, 80, 40, white);
    line(&image, 20, 13, 40, 80, red);
    TGA_ImageFlipVertically(&image);
    TGA_ImageWriteFile(&image, "output.tga", true);

    return 0;
}
