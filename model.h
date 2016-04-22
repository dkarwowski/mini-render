/**
 * Assumes that you've already defined the vectors in order to do this
 * Probably have to move out the linked list definitions.
 */
#ifndef _MODEL_h_

#define DEFINE_LIST(type) \
    struct ll_node_##type { \
        type data; \
        struct ll_node_##type *next; \
        struct ll_node_##type *prev; \
    }; \
    struct ll_##type { \
        struct ll_node_##type *first; \
        struct ll_node_##type *last; \
        int count; \
    }; \
    static inline struct ll_##type InitLL_##type() { \
        struct ll_##type result = {NULL, NULL, 0}; \
        return result; \
    } \
    static inline struct ll_node_##type * GetLL_##type(struct ll_##type *list_p, int idx) { \
        if (idx < 0 || list_p->count <= idx) return NULL; \
        struct ll_node_##type *result = \
            ((list_p->count - idx) > list_p->count / 2) ? list_p->first : list_p->last; \
        int i = ((list_p->count - idx) > list_p->count / 2) ? 0 : list_p->count - 1; \
        while (i != idx) { \
            result = ((list_p->count - idx) > list_p->count / 2) ? result->next : result->prev; \
            i += ((list_p->count - idx) > list_p->count / 2) ? 1 : -1; \
        } \
        return result; \
    } \
    static inline void RemNodeLL_##type(struct ll_##type *list_p, struct ll_node_##type *node_p) { \
        struct ll_node_##type *prev_p = node_p->prev; \
        struct ll_node_##type *next_p = node_p->next; \
        if (list_p->last == node_p) list_p->last = prev_p; \
        if (list_p->first == node_p) list_p->first = next_p; \
        if (prev_p) prev_p->next = next_p; \
        if (next_p) next_p->prev = prev_p; \
        free(node_p); \
        list_p->count--; \
    } \
    static inline void RemIdxLL_##type(struct ll_##type *list_p, int idx) { \
        struct ll_node_##type *torem_p = GetLL_##type(list_p, idx); \
        RemNodeLL_##type(list_p, torem_p); \
    } \
    static inline void AddLL_##type(struct ll_##type *list_p, type data) { \
        struct ll_node_##type *newlast = calloc(sizeof(struct ll_node_##type), 1); \
        newlast->data = data; \
        newlast->prev = list_p->last; \
        if (list_p->last) list_p->last->next = newlast; \
        if (!list_p->first) list_p->first = newlast; \
        list_p->last = newlast; \
        list_p->count++; \
    }

DEFINE_LIST(v3f);
DEFINE_LIST(v3i);

struct model {
    struct ll_v3f verts_;
    struct ll_v3f textures_;
    struct ll_v3f normals_;
    struct ll_v3i faces_;
    struct ll_v3i face_textures_;
    struct ll_v3i face_normals_;
};

#define _MODEL_h_
#endif
