#include "model.h"

struct model ModelInit(const char *filename)
{
    struct model result = {0};
    FILE *file_p = fopen(filename, "r");
    if (file_p == NULL)
        return (struct model){0};

    char line[256];
    while (!feof(file_p)) {
        if (fgets(line, 256, file_p) == NULL) {
            fprintf(stderr, "Can't read line from %s\n", filename);
            return (struct model){0};
        }

        char *tok = strtok(line, " ");
        if (strncmp(tok, "v", 1) == 0) {
            struct ll_node_v3f *vertnode = calloc(sizeof(struct ll_node_v3f), 1);
            for (int i = 0; i < 3; i++) {
                tok = strtok(NULL, " ");
                vertnode->data.raw[i] = atof(tok);
            }
        } else if (strncmp(tok, "f", 1) == 0) {
        }
    }

    return result;
}
