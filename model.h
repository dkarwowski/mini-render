/**
 * Assumes that you've already defined the vectors in order to do this
 * Probably have to move out the linked list definitions.
 */
#ifndef _MODEL_h_
#include "list.h"

struct ll_v3f {
    struct list_head head;
    v3f vec;
    int n;
};

static inline
void
ll_v3f_init(struct ll_v3f *list)
{
    INIT_LIST_HEAD(&list->head);
    list->n = 0;
}

static inline
int
ll_v3f_add_entry(struct ll_v3f *list, v3f vec)
{
    struct ll_v3f *temp, *last;
    if ((temp = (struct ll_v3f *)malloc(sizeof(struct ll_v3f))) == 0)
        return -1;
    memcpy(&temp->vec, &vec, sizeof(v3f));
    last = LIST_ENTRY(list->head.prev, struct ll_v3f, head);
    temp->n = last->n + 1;
    L_ListAddTail(&temp->head, &list->head);
    return 0;
}

static inline
int
ll_v3f_len(struct ll_v3f *list)
{
    struct ll_v3f *last = LIST_ENTRY(list->head.prev, struct ll_v3f, head);
    int result = last->n;
    return result;
}

static inline
v3f *
ll_v3f_get_index(struct ll_v3f *list, int index)
{
    struct ll_v3f *result, *temp;
    LIST_FOR_EACH_ENTRY_SAFE(result, temp, &list->head, head) {
        if (result->n == index)
            return &result->vec;
    }

    return NULL;
}

struct ll_face_node {
    struct list_head head;
    v3i indexes[3];
};

struct ll_face {
    struct ll_face_node list;
    int count;
};

static inline
void
ll_face_init(struct ll_face *list)
{
    INIT_LIST_HEAD(&list->list.head);
}

static inline
int
ll_face_add_entry(struct ll_face *list, v3i data[3])
{
    struct ll_face_node *temp;
    if ((temp = (struct ll_face_node *)malloc(sizeof(struct ll_face_node))) == 0)
        return -1;
    for (int i = 0; i < 3; i++) {
        temp->indexes[i].ivert = data[i].ivert;
        temp->indexes[i].iuv = data[i].iuv;
        temp->indexes[i].inorm = data[i].inorm;
    }
    L_ListAddTail(&temp->head, &list->list.head);
    return 0;
}

struct model {
    struct ll_v3f verts_;
    struct ll_v3f textures_;
    struct ll_v3f normals_;
    struct ll_face faces_;

    TGA_Image texture;
};

#define _MODEL_h_
#endif
