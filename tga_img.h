#ifndef _TGA_IMAGE_h_

#pragma pack(push,1)
typedef struct TGA_Header {
    char idlength;
    char colormaptype;
    char datatypecode;
    short colormaporigin;
    short colormaplength;
    char colormapdepth;
    short x_origin;
    short y_origin;
    short width;
    short height;
    char bitsperpixel;
    char imagedescriptor;
} TGA_Header;
#pragma pack(pop)

typedef struct TGA_Color {
    union {
        struct {
            unsigned char b, g, r, a;
        };
        unsigned char raw[4];
        unsigned int val;
    };

    int bytespp;
} TGA_Color;

#define TGA_ColorInit(r, g, b, a) (TGA_Color){.raw = {b, g, r, a}, .bytespp = 4}

typedef struct TGA_Image {
    unsigned char *data;
    int width;
    int height;
    int bytespp;
} TGA_Image;

enum TGA_Format {
    GRAYSCALE = 1,
    RGB = 3,
    RGBA = 4
};

#define _TGA_IMAGE_h_
#endif
