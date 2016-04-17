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
        if (strncmp(tok, "v", 1) == 0) {
            v3f data = {0};
            for (int i = 0; i < 3; i++) {
                tok = strtok(NULL, " ");
                data.raw[i] = atof(tok);
            }
            AddLL_v3f(&result->verts_, data);
        } else if (strncmp(tok, "f", 1) == 0) {
            v3i data = {0};
            for (int i = 0; i < 3; i++) {
                tok = strtok(NULL, " ");
                char subtok[16];
                for (int i = 0; i < 15 && tok[i] != '/'; i++) {
                    subtok[i] = tok[i];
                    subtok[i+1] = '\0';
                }
                data.raw[i] = atoll(subtok) - 1;
            }
            AddLL_v3i(&result->faces_, data);
        }

        if (fgets(line, 256, file_p) == NULL && !feof(file_p)) {
            fprintf(stderr, "Can't read line from %s\n", filename);
            return NULL;
        }
    }
    
    fprintf(stderr, "# v# %d f# %d\n", result->verts_.count, result->faces_.count);
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
