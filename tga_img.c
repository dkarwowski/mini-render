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
    TGA_Image result = {
        .data = NULL,
        .width = w,
        .height = h,
        .bytespp = bpp
    };

    unsigned long nbytes = w * h * bpp;
    result.data = calloc(nbytes, sizeof(unsigned char));

    return result;
}

static
bool
TGA_ImageLoadRLEData(TGA_Image *image_p, FILE *file_p)
{
    unsigned long pixelCount = image_p->width * image_p->height;
    unsigned long currentPixel = 0;
    unsigned long currentByte = 0;
    TGA_Color colorBuffer;

    do {
        unsigned char chunkHeader = 0;
        if (!fscanf(file_p, "%c", &chunkHeader)) {
            fprintf(stderr, "An error occured while reading data\n");
            return false;
        }

        if (chunkHeader < 128) {
            chunkHeader++;
            for (int i = 0; i < chunkHeader; i++) {
                if (!fread((void *)colorBuffer.raw, sizeof(char), image_p->bytespp, file_p)) {
                    fprintf(stderr, "An error occured while reading the header\n");
                    return false;
                }

                for (int t = 0; t < image_p->bytespp; t++)
                    image_p->data[currentByte++] = colorBuffer.raw[t];

                currentPixel++;
                if (currentPixel > pixelCount) {
                    fprintf(stderr, "Too many pixels read\n");
                    return false;
                }
            }
        } else {
            chunkHeader -= 127;
            if (!fread((void *)colorBuffer.raw, sizeof(char), image_p->bytespp, file_p)) {
                fprintf(stderr, "An error occured while reading the header\n");
                return false;
            }

            for (int i = 0; i < chunkHeader; i++) {
                for (int t = 0; t < image_p->bytespp; t++)
                    image_p->data[currentByte++] = colorBuffer.raw[t];

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
TGA_ImageUnloadRLEData(TGA_Image *image_p, FILE *file_p)
{
    const unsigned char maxChunkLength = 128;
    unsigned long npixels = image_p->width * image_p->height;
    unsigned long curpix = 0;

    while (curpix < npixels) {
        unsigned long chunkstart = curpix * image_p->bytespp;
        unsigned long curbyte = curpix * image_p->bytespp;
        unsigned char runLength = 1;
        bool raw = true;
        
        while (curpix + runLength < npixels && runLength < maxChunkLength) {
            bool succEq = true;
            for (int t = 0; succEq && t < image_p->bytespp; t++)
                succEq = (image_p->data[curbyte + t] == image_p->data[curbyte + t + image_p->bytespp]);

            curbyte += image_p->bytespp;
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

        if (fputc(raw ? runLength - 1 : runLength + 127, file_p) == EOF) {
            fprintf(stderr, "%d: Can't dump the runLength to file\n", ferror(file_p));
            return false;
        }

        if (!fwrite(image_p->data + chunkstart, (raw ? runLength * image_p->bytespp : image_p->bytespp), 1, file_p)) {
            fprintf(stderr, "%d: Can't dump the data to file\n", ferror(file_p));
            return false;
        }
    }

    return true;
}

static
bool
TGA_ImageWriteFile(TGA_Image *image_p, const char *filename, bool rle)
{
    unsigned char developer_area_ref[4] = {0};
    unsigned char extension_area_ref[4] = {0};
    unsigned char footer[18] = {'T', 'R', 'U', 'V', 'I', 'S', 'I', 'O', 'N', '-', 'X', 'F', 'I', 'L', 'E', '.', '\0'};

    FILE *file_p = fopen(filename, "w");
    if (file_p == NULL) {
        fprintf(stderr, "Can't open file %s\n", filename);
        fclose(file_p);
        return false;
    }

    TGA_Header header = {
        .bitsperpixel = image_p->bytespp << 3,
        .width = image_p->width,
        .height = image_p->height,
        .datatypecode = (image_p->bytespp == GRAYSCALE ? (rle ? 11 : 3) : (rle ? 10 : 2)),
        .imagedescriptor = 0x20 // top-left is the origin
    };

    if (!fwrite(&header, sizeof(header), 1, file_p)) {
        fprintf(stderr, "Can't open Dump the TGA file\n");
        fclose(file_p);
        return false;
    }

    if (!rle) {
        if (!fwrite(image_p->data, sizeof(char), image_p->width * image_p->height * image_p->bytespp, file_p)) {
            fprintf(stderr, "Can't unload the raw data\n");
            fclose(file_p);
            return false;
        }
    } else {
        if (!TGA_ImageUnloadRLEData(image_p, file_p)) {
            fprintf(stderr, "Can't unload RLE Data\n");
            fclose(file_p);
            return false;
        }
    }

    if (!fwrite(developer_area_ref, sizeof(developer_area_ref), 1, file_p)) {
        fprintf(stderr, "Can't dump the TGA file\n");
        fclose(file_p);
        return false;
    }

    if (!fwrite(extension_area_ref, sizeof(extension_area_ref), 1, file_p)) {
        fprintf(stderr, "Can't dump the TGA file\n");
        fclose(file_p);
        return false;
    }

    if (!fwrite(footer, sizeof(footer), 1, file_p)) {
        fprintf(stderr, "Can't dump the footer\n");
        fclose(file_p);
        return false;
    }

    fclose(file_p);
    return true;
}

static
TGA_Color
TGA_ImageGet(TGA_Image *image_p, int x, int y)
{
    TGA_Color result = {
        .val = 0,
        .bytespp = 1
    };

    if (!image_p->data || x < 0 || y < 0 || x >= image_p->width || y >= image_p->height)
        return result;

    for (int i = 0; i < image_p->bytespp; i++)
        result.raw[i] = (image_p->data + (x + y * image_p->width) * image_p->bytespp)[i];
    result.bytespp = image_p->bytespp;
    return result;
}

static
bool
TGA_ImageSet(TGA_Image *image_p, int x, int y, TGA_Color c)
{
    if (!image_p->data || x < 0 || y < 0 || x >= image_p->width || y >= image_p->height)
        return false;

    memcpy(image_p->data + (x + y * image_p->width) * image_p->bytespp, c.raw, image_p->bytespp);
    return true;
}

static
bool
TGA_ImageFlipHorizontally(TGA_Image *image_p)
{
    if (!image_p->data)
        return false;

    int half = image_p->width >> 1;
    for (int i = 0; i < half; i++) {
        for (int j = 0; j < image_p->height; j++) {
            TGA_Color c1 = TGA_ImageGet(image_p, i, j);
            TGA_Color c2 = TGA_ImageGet(image_p, image_p->width - 1 - i, j);
            TGA_ImageSet(image_p, i, j, c2);
            TGA_ImageSet(image_p, image_p->width - 1 - i, j, c1);
        }
    }

    return true;
}

static
bool
TGA_ImageFlipVertically(TGA_Image *image_p)
{
    if (!image_p->data)
        return false;

    unsigned long bytesPerLine = image_p->width * image_p->bytespp;
    unsigned char *line = calloc(sizeof(char), bytesPerLine);
    int half = image_p->height >> 1;
    for (int j = 0; j < half; j++) {
        unsigned long l1 = j * bytesPerLine;
        unsigned long l2 = (image_p->height - 1 - j) * bytesPerLine;

        memmove((void *)line, (void *)(image_p->data + l1), bytesPerLine);
        memmove((void *)(image_p->data + l1), (void *)(image_p->data + l2), bytesPerLine);
        memmove((void *)(image_p->data + l2), (void *)line, bytesPerLine);
    }

    free(line);
    return true;
}

static
bool
TGA_ImageReadFile(TGA_Image *image_p, const char *filename)
{
    if (image_p->data) free(image_p->data);
    image_p->data = NULL;

    FILE *file_p = fopen(filename, "r");
    if (file_p == NULL) {
        fprintf(stderr, "Can't open file: %s\n", filename);
        fclose(file_p);
        return false;
    }

    TGA_Header header;
    if (fread(&header, sizeof(header), 1, file_p) != sizeof(header)) {
        fprintf(stderr, "Error trying to read the header\n");
        fclose(file_p);
        return false;
    }

    image_p->width = header.width;
    image_p->height = header.height;
    image_p->bytespp = header.bitsperpixel >> 3;
    if (image_p->width <= 0 || image_p->height <= 0 
            || (image_p->bytespp != GRAYSCALE && image_p->bytespp != RGB && image_p->bytespp != RGBA)) {
        fprintf(stderr, "Bad bpp/width/height value\n");
        fclose(file_p);
        return false;
    }

    unsigned long nbytes = image_p->width * image_p->height * image_p->bytespp;
    image_p->data = calloc(nbytes, sizeof(unsigned char));
    if (3 == header.datatypecode || 2 == header.datatypecode) {
        if (fread(image_p->data, sizeof(char), nbytes, file_p) != nbytes) {
            fprintf(stderr, "And error occured while reading the data\n");
            fclose(file_p);
            return false;
        }
    } else if (10 == header.datatypecode || 11 == header.datatypecode) {
        if (!TGA_ImageLoadRLEData(image_p, file_p)) {
            fprintf(stderr, "An error occured while reading the data\n");
            fclose(file_p);
            return false;
        }
    } else {
        fprintf(stderr, "Unknown file format %d\n", (int)header.datatypecode);
        fclose(file_p);
        return false;
    }

    if (!(header.imagedescriptor & 0x20)) {
        TGA_ImageFlipVertically(image_p);
    }
    if (header.imagedescriptor & 0x10) {
        TGA_ImageFlipHorizontally(image_p);
    }

    fprintf(stderr, "%dx%d/%d\n", image_p->width, image_p->height, image_p->bytespp*8);
    fclose(file_p);
    return true;
}

static
void
TGA_ImageClear(TGA_Image *image_p)
{
    memset((void *)image_p->data, 0, image_p->width * image_p->height * image_p->bytespp);
}

static
bool
TGA_ImageScale(TGA_Image *image_p, int w, int h)
{
    if (w <= 0 || h <= 0 || !image_p->data)
        return false;
    unsigned char *tdata = calloc(sizeof(char), w * h * image_p->bytespp);
    int nscanline = 0;
    int oscanline = 0;
    int erry = 0;

    unsigned long nlinebytes = w * image_p->bytespp;
    unsigned long olinebytes = image_p->width * image_p->bytespp;

    for (int j = 0; j < image_p->height; j++) {
        int errx = image_p->width = 2;
        int nx = -image_p->bytespp;
        int ox = -image_p->bytespp;

        for (int i = 0; i < image_p->width; i++) {
            ox += image_p->bytespp;
            errx += w;
            while (errx >= (int)image_p->width) {
                errx -= image_p->width;
                nx += image_p->bytespp;
                memcpy(tdata + nscanline + nx, image_p->data+oscanline+ox, image_p->bytespp);
            }
        }
        erry += h;
        oscanline += olinebytes;
        while (erry >= (int)image_p->height) {
            if (erry >= (int)image_p->height << 1)
                memcpy(tdata + nscanline + nlinebytes, tdata + nscanline, nlinebytes);
            erry -= image_p->height;
            nscanline += nlinebytes;
        }
    }

    free(image_p->data);
    image_p->data = tdata;
    image_p->width = w;
    image_p->height = h;

    return true;
}
