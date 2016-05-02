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
    if (fgets(line, 256, file) == NULL) {
        fprintf(stderr, "Can't read line from %s\n", filename);
        return -1;
    };

    ll_v3f_init(&model->verts_);
    ll_v3f_init(&model->textures_);
    ll_v3f_init(&model->normals_);
    ll_face_init(&model->faces_);

    while (!feof(file)) {
        char *tok = strtok(line, " ");
        if (strncmp(tok, "vt", 2) == 0) {
            v3f data = {0};
            for (int i = 0; i < 3; i++) {
                tok = strtok(NULL, " ");
                data.raw[i] = atof(tok);
            }
            ll_v3f_add_entry(&model->textures_, data);
        } else if (strncmp(tok, "vn", 2) == 0) {
            v3f data = {0};
            for (int i = 0; i < 3; i++) {
                tok = strtok(NULL, " ");
                data.raw[i] = atof(tok);
            }
            ll_v3f_add_entry(&model->normals_, data);
        } else if (strncmp(tok, "v", 1) == 0) {
            v3f data = {0};
            for (int i = 0; i < 3; i++) {
                tok = strtok(NULL, " ");
                data.raw[i] = atof(tok);
            }
            ll_v3f_add_entry(&model->verts_, data);
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
            ll_face_add_entry(&model->faces_, data);
        }

        if (fgets(line, 256, file) == NULL && !feof(file)) {
            fprintf(stderr, "Can't read line from %s\n", filename);
            fclose(file);
            return -1;
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
    fprintf(stderr, "# v# %d vt# %d\n", ll_v3f_len(&model->verts_), ll_v3f_len(&model->textures_));
    return 0;
}

static
void
ModelDelete(struct model *model)
{
    while (!list_empty(&model->verts_.head)) {
        struct ll_v3f *del = list_entry(model->verts_.head.next, struct ll_v3f, head);
        list_del(&del->head);
        free(del);
    }
    while (!list_empty(&model->textures_.head)) {
        struct ll_v3f *del = list_entry(model->textures_.head.next, struct ll_v3f, head);
        list_del(&del->head);
        free(del);
    }
    while (!list_empty(&model->normals_.head)) {
        struct ll_v3f *del = list_entry(model->normals_.head.next, struct ll_v3f, head);
        list_del(&del->head);
        free(del);
    }
    while (!list_empty(&model->faces_.list.head)) {
        struct ll_face_node *del = list_entry(model->faces_.list.head.next, struct ll_face_node, head);
        list_del(&del->head);
        free(del);
    }

    TGA_ImageDelete(&model->texture);
}
