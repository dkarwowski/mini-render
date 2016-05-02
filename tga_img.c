#include <math.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "tga_img.h"

static
TGA_Image
TGA_ImageInit(int w, int h, int bpp)
{
    TGA_Image result = {    .data = NULL,
                            .width = w,
                            .height = h,
                            .bytespp = bpp };

    unsigned long nbytes = w * h * bpp;
    result.data = (unsigned char *)calloc(nbytes, sizeof(unsigned char));

    return result;
}

static
void
TGA_ImageDelete(TGA_Image *image)
{
    free(image->data);
    image->width = 0;
    image->height = 0;
    image->bytespp = 0;
    image->data = NULL;
}

static
bool
TGA_ImageLoadRLEData(TGA_Image *image, FILE *file)
{
    unsigned long pixelCount   = image->width * image->height;
    unsigned long currentPixel = 0;
    unsigned long currentByte  = 0;
    TGA_Color colorBuffer;

    do {
        unsigned char chunkHeader = 0;
        if (fscanf(file, "%c", &chunkHeader) == 0) {
            fprintf(stderr, "An error occured while reading data\n");
            return false;
        }

        if (chunkHeader < 128) {
            chunkHeader++;
            for (int i = 0; i < chunkHeader; i++) {
                if (fread((void *)colorBuffer.raw, sizeof(char), image->bytespp, file) == 0) {
                    fprintf(stderr, "An error occured while reading the header\n");
                    return false;
                }

                for (int t = 0; t < image->bytespp; t++)
                    image->data[currentByte++] = colorBuffer.raw[t];

                currentPixel++;
                if (currentPixel > pixelCount) {
                    fprintf(stderr, "Too many pixels read\n");
                    return false;
                }
            }
        } else {
            chunkHeader -= 127;
            if (fread((void *)colorBuffer.raw, sizeof(char), image->bytespp, file) == 0) {
                fprintf(stderr, "An error occured while reading the header\n");
                return false;
            }

            for (int i = 0; i < chunkHeader; i++) {
                for (int t = 0; t < image->bytespp; t++)
                    image->data[currentByte++] = colorBuffer.raw[t];

                currentPixel++;
                if (currentPixel > pixelCount) {
                    fprintf(stderr, "Too many pixels read\n");
                    return false;
                }
            }
        }
    } while (currentPixel < pixelCount);

    return true;
}

static
bool
TGA_ImageUnloadRLEData(TGA_Image *image, FILE *file)
{
    const unsigned char maxChunkLength = 128;
    unsigned long       npixels        = image->width * image->height;
    unsigned long       curpix         = 0;

    while (curpix < npixels) {
        unsigned long chunkstart = curpix * image->bytespp;
        unsigned long curbyte    = curpix * image->bytespp;
        unsigned char runLength  = 1;
        bool raw = true;

        while (curpix + runLength < npixels && runLength < maxChunkLength) {
            bool succEq = true;
            for (int t = 0; succEq && t < image->bytespp; t++)
                succEq = (image->data[curbyte + t] == image->data[curbyte + t + image->bytespp]);

            curbyte += image->bytespp;
            if (1 == runLength) {
                raw = !succEq;
            }
            if (raw && succEq) {
                runLength--;
                break;
            }
            if (!raw && !succEq) {
                break;
            }
            runLength++;
        }
        curpix += runLength;

        if (fputc(raw ? runLength - 1 : runLength + 127, file) == EOF) {
            fprintf(stderr, "%d: Can't dump the runLength to file\n", ferror(file));
            return false;
        }

        if (fwrite( image->data + chunkstart,
                    (raw ? runLength * image->bytespp : image->bytespp),
                    1,
                    file) == 0) {
            fprintf(stderr, "%d: Can't dump the data to file\n", ferror(file));
            return false;
        }
    }

    return true;
}

static
bool
TGA_ImageWriteFile(TGA_Image *image, const char *filename, bool rle)
{
    unsigned char developer_area_ref[4] = {0};
    unsigned char extension_area_ref[4] = {0};
    unsigned char footer[18]            = { 'T', 'R', 'U', 'V', 'I', 'S',
                                            'I', 'O', 'N', '-', 'X', 'F',
                                            'I', 'L', 'E', '.', '\0'};

    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        fprintf(stderr, "Can't open file %s\n", filename);
        fclose(file);
        return false;
    }

    TGA_Header header = {
        .bitsperpixel = image->bytespp << 3,
        .width = image->width,
        .height = image->height,
        .datatypecode = (image->bytespp == GRAYSCALE ? (rle ? 11 : 3) : (rle ? 10 : 2)),
        .imagedescriptor = 0x20 // top-left is the origin
    };

    if (fwrite(&header, sizeof(header), 1, file) == 0) {
        fprintf(stderr, "Can't open Dump the TGA file\n");
        fclose(file);
        return false;
    }

    if (!rle) {
        if (fwrite(image->data, sizeof(char), image->width * image->height * image->bytespp, file) == 0) {
            fprintf(stderr, "Can't unload the raw data\n");
            fclose(file);
            return false;
        }
    } else {
        if (!TGA_ImageUnloadRLEData(image, file)) {
            fprintf(stderr, "Can't unload RLE Data\n");
            fclose(file);
            return false;
        }
    }

    if (fwrite(developer_area_ref, sizeof(developer_area_ref), 1, file) == 0) {
        fprintf(stderr, "Can't dump the TGA file\n");
        fclose(file);
        return false;
    }

    if (fwrite(extension_area_ref, sizeof(extension_area_ref), 1, file) == 0) {
        fprintf(stderr, "Can't dump the TGA file\n");
        fclose(file);
        return false;
    }

    if (fwrite(footer, sizeof(footer), 1, file) == 0) {
        fprintf(stderr, "Can't dump the footer\n");
        fclose(file);
        return false;
    }

    fclose(file);
    return true;
}

static
TGA_Color
TGA_ImageGet(TGA_Image *image, int x, int y)
{
    TGA_Color result = {
        .val = 0,
        .bytespp = 1
    };

    if (!image->data || x < 0 || y < 0 || x >= image->width || y >= image->height)
        return result;

    for (int i = 0; i < image->bytespp; i++)
        result.raw[i] = (image->data + (x + y * image->width) * image->bytespp)[i];
    result.bytespp = image->bytespp;
    return result;
}

static
bool
TGA_ImageSet(TGA_Image *image, int x, int y, TGA_Color c)
{
    if (!image->data || x < 0 || y < 0 || x >= image->width || y >= image->height)
        return false;

    memcpy(image->data + (x + y * image->width) * image->bytespp, c.raw, image->bytespp);
    return true;
}

static
bool
TGA_ImageFlipHorizontally(TGA_Image *image)
{
    if (!image->data)
        return false;

    int half = image->width >> 1;
    for (int i = 0; i < half; i++) {
        for (int j = 0; j < image->height; j++) {
            TGA_Color c1 = TGA_ImageGet(image, i, j);
            TGA_Color c2 = TGA_ImageGet(image, image->width - 1 - i, j);
            TGA_ImageSet(image, i, j, c2);
            TGA_ImageSet(image, image->width - 1 - i, j, c1);
        }
    }

    return true;
}

static
bool
TGA_ImageFlipVertically(TGA_Image *image)
{
    if (!image->data)
        return false;

    unsigned long bytesPerLine = image->width * image->bytespp;
    unsigned char *line = calloc(sizeof(char), bytesPerLine);
    int half = image->height >> 1;
    for (int j = 0; j < half; j++) {
        unsigned long l1 = j * bytesPerLine;
        unsigned long l2 = (image->height - 1 - j) * bytesPerLine;

        memmove((void *)line, (void *)(image->data + l1), bytesPerLine);
        memmove((void *)(image->data + l1), (void *)(image->data + l2), bytesPerLine);
        memmove((void *)(image->data + l2), (void *)line, bytesPerLine);
    }

    free(line);
    return true;
}

static
bool
TGA_ImageReadFile(TGA_Image *image, const char *filename)
{
    if (image->data) free(image->data);
    image->data = NULL;

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Can't open file: %s\n", filename);
        fclose(file);
        return false;
    }

    TGA_Header header;
    if (fread(&header, sizeof(header), 1, file) == 0) {
        fprintf(stderr, "Error trying to read the header\n");
        fclose(file);
        return false;
    }

    image->width = header.width;
    image->height = header.height;
    image->bytespp = header.bitsperpixel >> 3;
    if (image->width <= 0 || image->height <= 0
            || (image->bytespp != GRAYSCALE && image->bytespp != RGB && image->bytespp != RGBA)) {
        fprintf(stderr, "Bad bpp/width/height value\n");
        fclose(file);
        return false;
    }

    unsigned long nbytes = image->width * image->height * image->bytespp;
    image->data = (unsigned char *)calloc(nbytes, sizeof(unsigned char));
    if (3 == header.datatypecode || 2 == header.datatypecode) {
        if (fread(image->data, sizeof(char), nbytes, file) != nbytes) {
            fprintf(stderr, "And error occured while reading the data\n");
            fclose(file);
            return false;
        }
    } else if (10 == header.datatypecode || 11 == header.datatypecode) {
        if (!TGA_ImageLoadRLEData(image, file)) {
            fprintf(stderr, "An error occured while reading the data\n");
            fclose(file);
            return false;
        }
    } else {
        fprintf(stderr, "Unknown file format %d\n", (int)header.datatypecode);
        fclose(file);
        return false;
    }

    if (!(header.imagedescriptor & 0x20)) {
        TGA_ImageFlipVertically(image);
    }
    if (header.imagedescriptor & 0x10) {
        TGA_ImageFlipHorizontally(image);
    }

    fprintf(stderr, "%dx%d/%d\n", image->width, image->height, image->bytespp*8);
    fclose(file);
    return true;
}

static
void
TGA_ImageClear(TGA_Image *image)
{
    memset((void *)image->data, 0, image->width * image->height * image->bytespp);
}

static
bool
TGA_ImageScale(TGA_Image *image, int w, int h)
{
    if (w <= 0 || h <= 0 || !image->data)
        return false;
    unsigned char *tdata = calloc(sizeof(char), w * h * image->bytespp);
    int nscanline = 0;
    int oscanline = 0;
    int erry = 0;

    unsigned long nlinebytes = w * image->bytespp;
    unsigned long olinebytes = image->width * image->bytespp;

    for (int j = 0; j < image->height; j++) {
        int errx = image->width = 2;
        int nx = -image->bytespp;
        int ox = -image->bytespp;

        for (int i = 0; i < image->width; i++) {
            ox += image->bytespp;
            errx += w;
            while (errx >= (int)image->width) {
                errx -= image->width;
                nx += image->bytespp;
                memcpy(tdata + nscanline + nx, image->data+oscanline+ox, image->bytespp);
            }
        }
        erry += h;
        oscanline += olinebytes;
        while (erry >= (int)image->height) {
            if (erry >= (int)image->height << 1)
                memcpy(tdata + nscanline + nlinebytes, tdata + nscanline, nlinebytes);
            erry -= image->height;
            nscanline += nlinebytes;
        }
    }

    free(image->data);
    image->data = tdata;
    image->width = w;
    image->height = h;

    return true;
}
