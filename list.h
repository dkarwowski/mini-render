#ifndef _LIST_h_

/* This is based off of a copy of the Linux Kernel's List implementation,
 * under (include/linux/list.h), and modified a bit for my own use.
 * Credits go to wherever they belong.
 */

/**
 * Circular doubly linked list implementation.
 *
 * Some of the functions are useful even though internal, if you want to
 * directly mess with whole lists instead of single entries.
 */

struct list_head {
    struct list_head *next, *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
    struct list_head name = LIST_HEAD_INIT(name)

#define INIT_LIST_HEAD(ptr) \
    do { \
        (ptr)->next = (ptr); (ptr)->prev = (ptr); \
    } while(0)

/**
 * Insert a new entry between known consecutive entries.
 */
static inline
void
__ListAdd(struct list_head *new, struct list_head *prev, struct list_head *next)
{
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

/**
 * L_ListAdd - add a new entry
 * @new: new entry to be added
 * @head: list head to add after
 *
 * Insert a new entry after the specified head.
 */
static inline
void
L_ListAdd(struct list_head *new, struct list_head *head)
{
    __ListAdd(new, head, head->next);
}

/**
 * L_ListAddTail - add a new entry
 * @new: entry to be added
 * @head: list head to add before
 *
 * Insert a new entry before the specified head.
 */
static inline
void
L_ListAddTail(struct list_head *new, struct list_head *head)
{
    __ListAdd(new, head->prev, head);
}

/**
 * Delete a list entry by fixing the prev and next nodes to point
 * to each other
 */
static inline
void
__ListDel(struct list_head *prev, struct list_head *next)
{
    next->prev = prev;
    prev->next = next;
}

/**
 * L_ListDel - delete entry from the list
 * @entry: element to delete from the list
 * Note: turns the node into an undefined state
 */
static inline
void
L_ListDel(struct list_head *entry)
{
    __ListDel(entry->prev, entry->next);
    entry->next = (void *)0;
    entry->prev = (void *)0;
}

/**
 * L_ListDelInit - delete an entry and reinitialize it
 * @entry: element to delete from the list
 */
static inline
void
L_ListDelInit(struct list_head *entry)
{
    __ListDel(entry->prev, entry->next);
    INIT_LIST_HEAD(entry);
}

/**
 * L_ListMove - delete from one list and add to head of another
 * @list: the entry to move
 * @head: list head to add after
 */
static inline
void
L_ListMove(struct list_head *list, struct list_head *head)
{
    __ListDel(list->prev, list->next);
    L_ListAdd(list, head);
}

/**
 * L_ListMoveTail - delete from one list and add as another's tail
 * @list: entry to move
 * @head: list head that will follow our entry
 */
static inline
void
L_ListMoveTail(struct list_head *list, struct list_head *head)
{
    __ListDel(list->prev, list->next);
    L_ListAddTail(list, head);
}

/**
 * L_ListEmpty - tests whether a list is empty
 * @head: list to test
 */
static inline
int
L_ListEmpty(struct list_head *head)
{
    return head->next == head;
}

/**
 * Join two lists together, assuming they're nonempty when passed in
 */
static inline
void
__ListSplice(struct list_head *list, struct list_head *head)
{
    struct list_head *first = list->next;
    struct list_head *last = list->prev;
    struct list_head *at = head->next;

    first->prev = head;
    head->next = first;

    last->next = at;
    at->prev = last;
}

/**
 * L_ListSplice - join two lists together
 * @list: the new list to add.
 * @head: the place to add it in the first list
 */
static inline
void
L_ListSplice(struct list_head *list, struct list_head *head)
{
    if (!L_ListEmpty(list))
        __ListSplice(list, head);
}

/**
 * L_ListSpliceInit - join two lists and reinitialize the emptied list
 * @list: new list to add
 * @head: the place to add it in the first list
 *
 * The list at @list is reinitialized
 */
static inline
void
L_ListSpliceInit(struct list_head *list, struct list_head *head)
{
    if (!L_ListEmpty(list)) {
        __ListSplice(list, head);
        INIT_LIST_HEAD(list);
    }
}

/**
 * LIST_ENTRY - get the struct for this entry
 * @ptr: the (struct list_head *) we're at
 * @type: the type of the struct
 * @member: the name of the list_head in the struct
 */
#define LIST_ENTRY(ptr, type, member) \
    ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))

/**
 * LIST_FOR_EACH - iterate over a list
 * @pos: the (struct list_head *) to use as counter
 * @head: head of your list
 */
#define LIST_FOR_EACH(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * LIST_FOR_EACH_PREV - iterate over a list backwards
 * @pos: the (struct list_head *) to use as a counter
 * @head: head of your list
 */
#define LIST_FOR_EACH_PREV(pos, head) \
    for (pos = (head)->prev; pos != (head); pos = pos->prev)

/**
 * LIST_FOR_EACH_SAFE - iterate over a list safe against removal
 * @pos: the (struct list_head *) to use as a counter
 * @n: a (struct list_head *) to use as temporary storage
 * @head: head of your list
 */
#define LIST_FOR_EACH_SAFE(pos, n, head) \
    for (pos = (head)->next, n = pos->next; pos != (head); pos = n, n = pos->next)

/**
 * LIST_FOR_EACH_ENTRY - iterate over entries in the list
 * @pos: (type *) to use as a counter
 * @head: head of your list
 * @member: name of the list_head within the struct
 */
#define LIST_FOR_EACH_ENTRY(pos, head, member) \
    for (pos = LIST_ENTRY((head)->next, typeof(*pos), member); \
         &pos->member != (head); \
         pos = LIST_ENTRY(pos->member.next, typeof(*pos), member))

/**
 * LIST_FOR_EACH_ENTRY_SAFE - iterate over entries safe against removal
 * @pos: (type *) to use as a counter
 * @n: (type *) for temporary storage
 * @head: head of your list
 * @member: the name of the list_head within the struct
 */
#define LIST_FOR_EACH_ENTRY_SAFE(pos, n, head, member) \
    for (pos = LIST_ENTRY((head)->next, typeof(*pos), member), \
         n = LIST_ENTRY(pos->member.next, typeof(*pos), member); \
         &pos->member != (head); \
         pos = n, n = LIST_ENTRY(n->member.next, typeof(*n), member))

#define _LIST_h_
#endif
