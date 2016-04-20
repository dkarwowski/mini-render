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

    while (!feof(file_p)) {
        char *tok = strtok(line, " ");
        if (strncmp(tok, "vt", 2) == 0) {
            v3f data = {0};
            for (int i = 0; i < 3; i++) {
                tok = strtok(NULL, " ");
                data.raw[i] = atof(tok);
            }
            AddLL_v3f(&result->textures_, data);
        } else if (strncmp(tok, "v", 1) == 0) {
            v3f data = {0};
            for (int i = 0; i < 3; i++) {
                tok = strtok(NULL, " ");
                data.raw[i] = atof(tok);
            }
            AddLL_v3f(&result->verts_, data);
        } else if (strncmp(tok, "f", 1) == 0) {
            v3i data_model = {0};
            v3i data_texture = {0};
            for (int i = 0; i < 3; i++) {
                tok = strtok(NULL, " ");

                int j;
                char subtok[16];
                for (j = 0; j < 15 && tok[j] != '/'; j++) {
                    subtok[j] = tok[j];
                    subtok[j+1] = '\0';
                }
                data_model.raw[i] = atoll(subtok) - 1;
                for (int k = 1; k < 15 - j && tok[j + k] != '/'; k++) {
                    subtok[k - 1] = tok[j + k];
                    subtok[k] = '\0';
                }
                data_texture.raw[i] = atoll(subtok) - 1;
            }
            AddLL_v3i(&result->faces_, data_model);
            AddLL_v3i(&result->faces_textures_, data_texture);
        }

        if (fgets(line, 256, file_p) == NULL && !feof(file_p)) {
            fprintf(stderr, "Can't read line from %s\n", filename);
            return NULL;
        }
    }
    
    if (result->faces_.count != result->faces_textures_.count)
        fprintf(stderr, "Invalid counts, %d != %d\n", result->faces_.count, result->faces_textures_.count);

    fprintf(stderr, "# v# %d vt# %d f# %d\n", result->verts_.count, result->textures_.count, result->faces_.count);
    return result;
}

static
void
ModelDelete(struct model *model_p)
{
    while (model_p->verts_.count)
        RemIdxLL_v3f(&model_p->verts_, 0);
    while (model_p->faces_.count)
        RemIdxLL_v3i(&model_p->faces_, 0);
}
