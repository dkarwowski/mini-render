#include "model.h"

static
struct model *
ModelInit(const char *filename)
{
    struct model *result = calloc(sizeof(struct model), 1);
    FILE *file_p = fopen(filename, "r");
    if (file_p == NULL)
        return NULL;

    char line[256];
    if (fgets(line, 256, file_p) == NULL) {
        fprintf(stderr, "Can't read line from %s\n", filename);
        return NULL;
    };

    ll_v3f_init(&result->verts_);
    ll_v3f_init(&result->textures_);
    ll_v3f_init(&result->normals_);
    ll_face_init(&result->faces_);

    while (!feof(file_p)) {
        char *tok = strtok(line, " ");
        if (strncmp(tok, "vt", 2) == 0) {
            v3f data = {0};
            for (int i = 0; i < 3; i++) {
                tok = strtok(NULL, " ");
                data.raw[i] = atof(tok);
            }
            ll_v3f_add_entry(&result->textures_, data);
        } else if (strncmp(tok, "vn", 2) == 0) {
            v3f data = {0};
            for (int i = 0; i < 3; i++) {
                tok = strtok(NULL, " ");
                data.raw[i] = atof(tok);
            }
            ll_v3f_add_entry(&result->normals_, data);
        } else if (strncmp(tok, "v", 1) == 0) {
            v3f data = {0};
            for (int i = 0; i < 3; i++) {
                tok = strtok(NULL, " ");
                data.raw[i] = atof(tok);
            }
            ll_v3f_add_entry(&result->verts_, data);
        } else if (strncmp(tok, "f", 1) == 0) {
            v3i data[3];
            for (int i = 0; i < 3; i++) {
                tok = strtok(NULL, " ");

                int j,k;
                char subtok[16];
                for (j = 0; j < 15 && tok[j] != '/'; j++) {
                    subtok[j] = tok[j];
                    subtok[j+1] = '\0';
                }
                data[i].ivert = atoll(subtok);
                for (k = 1; k < 15 && tok[j + k] != '/'; k++) {
                    subtok[k - 1] = tok[j + k];
                    subtok[k] = '\0';
                }
                data[i].iuv = atoll(subtok);
                for (int l = 1; l < 15 && tok[j + k + l] != '\0'; l++) {
                    subtok[l - 1] = tok[j + k + l];
                    subtok[l] = '\0';
                }
                data[i].inorm = atoll(subtok);
            }
            ll_face_add_entry(&result->faces_, data);
        }

        if (fgets(line, 256, file_p) == NULL && !feof(file_p)) {
            fprintf(stderr, "Can't read line from %s\n", filename);
            return NULL;
        }
    }
    
    char texture_filename[128];
    for (int i = 0; i < 127 && filename[i] != '.'; i++) {
        texture_filename[i] = filename[i];
        texture_filename[i+1] = '\0';
    }
    strcat(texture_filename, "_diffuse.tga");

    // quit out if the texture doesn't exist; we only do with textures
    if (access(texture_filename, F_OK) == -1)
        exit(-1);
    TGA_ImageReadFile(&result->texture, texture_filename);
    TGA_ImageFlipVertically(&result->texture);

    fprintf(stderr, "# v# %d vt# %d\n", ll_v3f_len(&result->verts_), ll_v3f_len(&result->textures_));
    return result;
}

static
void
ModelDelete(struct model *model_p)
{
    while (!list_empty(&model_p->verts_.head))
        list_del(model_p->verts_.head.next);
    while (!list_empty(&model_p->textures_.head))
        list_del(model_p->textures_.head.next);
    while (!list_empty(&model_p->normals_.head))
        list_del(model_p->normals_.head.next);
    while (!list_empty(&model_p->faces_.list.head))
        list_del(model_p->faces_.list.head.next);
}
