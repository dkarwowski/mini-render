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
__list_add(struct list_head *new, struct list_head *prev, struct list_head *next)
{
    next->prev = new;
    new->next = next;
    new->prev = prev;
    prev->next = new;
}

/**
 * list_add - add a new entry
 * @new: new entry to be added
 * @head: list head to add after
 *
 * Insert a new entry after the specified head.
 */
static inline
void
list_add(struct list_head *new, struct list_head *head)
{
    __list_add(new, head, head->next);
}

/**
 * list_add_tail - add a new entry
 * @new: entry to be added
 * @head: list head to add before
 *
 * Insert a new entry before the specified head.
 */
static inline
void
list_add_tail(struct list_head *new, struct list_head *head)
{
    __list_add(new, head->prev, head);
}

/**
 * Delete a list entry by fixing the prev and next nodes to point
 * to each other
 */
static inline
void
__list_del(struct list_head *prev, struct list_head *next)
{
    next->prev = prev;
    prev->next = next;
}

/**
 * list_del - delete entry from the list
 * @entry: element to delete from the list
 * Note: turns the node into an undefined state
 */
static inline
void
list_del(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
    entry->next = (void *)0;
    entry->prev = (void *)0;
}

/**
 * list_del_init - delete an entry and reinitialize it
 * @entry: element to delete from the list
 */
static inline
void
list_del_init(struct list_head *entry)
{
    __list_del(entry->prev, entry->next);
    INIT_LIST_HEAD(entry);
}

/**
 * list_move - delete from one list and add to head of another
 * @list: the entry to move
 * @head: list head to add after
 */
static inline
void
list_move(struct list_head *list, struct list_head *head)
{
    __list_del(list->prev, list->next);
    list_add(list, head);
}

/**
 * list_move_tail - delete from one list and add as another's tail
 * @list: entry to move
 * @head: list head that will follow our entry
 */
static inline
void
list_move_tail(struct list_head *list, struct list_head *head)
{
    __list_del(list->prev, list->next);
    list_add_tail(list, head);
}

/**
 * list_empty - tests whether a list is empty
 * @head: list to test
 */
static inline
int
list_empty(struct list_head *head)
{
    return head->next == head;
}

/**
 * Join two lists together, assuming they're nonempty when passed in
 */
static inline
void
__list_splice(struct list_head *list, struct list_head *head)
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
 * list_splice - join two lists together
 * @list: the new list to add.
 * @head: the place to add it in the first list
 */
static inline
void
list_splice(struct list_head *list, struct list_head *head)
{
    if (!list_empty(list))
        __list_splice(list, head);
}

/**
 * list_splice_init - join two lists and reinitialize the emptied list
 * @list: new list to add
 * @head: the place to add it in the first list
 *
 * The list at @list is reinitialized
 */
static inline
void
list_splice_init(struct list_head *list, struct list_head *head)
{
    if (!list_empty(list)) {
        __list_splice(list, head);
        INIT_LIST_HEAD(list);
    }
}

/**
 * list_entry - get the struct for this entry
 * @ptr: the (struct list_head *) we're at
 * @type: the type of the struct
 * @member: the name of the list_head in the struct
 */
#define list_entry(ptr, type, member) \
    ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))

/**
 * list_for_each - iterate over a list
 * @pos: the (struct list_head *) to use as counter
 * @head: head of your list
 */
#define list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

/**
 * list_for_each_prev - iterate over a list backwards
 * @pos: the (struct list_head *) to use as a counter
 * @head: head of your list
 */
#define list_for_each_prev(pos, head) \
    for (pos = (head)->prev; pos != (head); pos = pos->prev)

/**
 * list_for_each_safe - iterate over a list safe against removal
 * @pos: the (struct list_head *) to use as a counter
 * @n: a (struct list_head *) to use as temporary storage
 * @head: head of your list
 */
#define list_for_each_safe(pos, n, head) \
    for (pos = (head)->next, n = pos->next; pos != (head); pos = n, n = pos->next)

/**
 * list_for_each_entry - iterate over entries in the list
 * @pos: (type *) to use as a counter
 * @head: head of your list
 * @member: name of the list_head within the struct
 */
#define list_for_each_entry(pos, head, member) \
    for (pos = list_entry((head)->next, typeof(*pos), member); \
         &pos->member != (head); \
         pos = list_entry(pos->member.next, typeof(*pos), member))

/**
 * list_for_each_entry_safe - iterate over entries safe against removal
 * @pos: (type *) to use as a counter
 * @n: (type *) for temporary storage
 * @head: head of your list
 * @member: the name of the list_head within the struct
 */
#define list_for_each_entry_safe(pos, n, head, member) \
    for (pos = list_entry((head)->next, typeof(*pos), member), \
         n = list_entry(pos->member.next, typeof(*pos), member); \
         &pos->member != (head); \
         pos = n, n = list_entry(n->member.next, typeof(*n), member))

#define _LIST_h_
#endif
