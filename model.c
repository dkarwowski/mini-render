#include "model.h"

static
int
ModelInit(struct model *model, const char *filename)
{
    memset(model, 0, sizeof(struct model));
    FILE *file = fopen(filename, "r");
    if (file == NULL)
        return -1;

    char line[256];

    LL_V3F_Init(&model->verts_);
    LL_V3F_Init(&model->textures_);
    LL_V3F_Init(&model->normals_);
    LL_Face_Init(&model->faces_);

    while (fgets(line, 256, file)) {
        char *tok = strtok(line, " ");
        if (strncmp(tok, "vt", 2) == 0) {
            v3f data = {0};
            for (int i = 0; i < 3; i++) {
                tok = strtok(NULL, " ");
                data.raw[i] = atof(tok);
            }
            LL_V3F_AddEntry(&model->textures_, data);
        } else if (strncmp(tok, "vn", 2) == 0) {
            v3f data = {0};
            for (int i = 0; i < 3; i++) {
                tok = strtok(NULL, " ");
                data.raw[i] = atof(tok);
            }
            LL_V3F_AddEntry(&model->normals_, data);
        } else if (strncmp(tok, "v", 1) == 0) {
            v3f data = {0};
            for (int i = 0; i < 3; i++) {
                tok = strtok(NULL, " ");
                data.raw[i] = atof(tok);
            }
            LL_V3F_AddEntry(&model->verts_, data);
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
            LL_Face_AddEntry(&model->faces_, data);
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
    TGA_ImageReadFile(&model->texture, texture_filename);
    TGA_ImageFlipVertically(&model->texture);

    fclose(file);
    fprintf(stderr, "# v# %d vt# %d\n", LL_V3F_Len(&model->verts_), LL_V3F_Len(&model->textures_));
    return 0;
}

static
void
ModelDelete(struct model *model)
{
    while (!L_ListEmpty(&model->verts_.head)) {
        struct ll_v3f *del = LIST_ENTRY(model->verts_.head.next, struct ll_v3f, head);
        L_ListDel(&del->head);
        free(del);
    }
    while (!L_ListEmpty(&model->textures_.head)) {
        struct ll_v3f *del = LIST_ENTRY(model->textures_.head.next, struct ll_v3f, head);
        L_ListDel(&del->head);
        free(del);
    }
    while (!L_ListEmpty(&model->normals_.head)) {
        struct ll_v3f *del = LIST_ENTRY(model->normals_.head.next, struct ll_v3f, head);
        L_ListDel(&del->head);
        free(del);
    }
    while (!L_ListEmpty(&model->faces_.list.head)) {
        struct ll_face_node *del = LIST_ENTRY(model->faces_.list.head.next, struct ll_face_node, head);
        L_ListDel(&del->head);
        free(del);
    }

    TGA_ImageDelete(&model->texture);
}
