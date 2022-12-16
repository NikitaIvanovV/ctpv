#include <string.h>
#include <sys/types.h>

#include "ulist.h"
#include "error.h"

/*
 * Unrolled linked list
 */

#define DEFAULT_CAP 256
#define NO_LOCK     -1

#define ULIST_BUF(node)             ((void *)&(node).buf)
#define ULIST_BUF_AT(list, node, i) (ULIST_BUF(node) + i * (list).size)

struct UList {
    size_t size;
    ssize_t lock_i;
    struct UListNode *head, *tail;
};

struct UListNode {
    size_t len, cap;
    struct UListNode *next;
    void *buf;
};

static struct UListNode *ulist_node_new(UList *l, size_t cap)
{
    struct UListNode *n;

    if (cap == 0)
        cap = DEFAULT_CAP;

    /* Store buffer and node data in the same chunk */
    if (!(n = malloc(sizeof(*n) - sizeof(n->buf) + (cap * l->size)))) {
        FUNCFAILED("malloc", strerror(errno));
        abort();
    }

    n->cap = cap;
    n->len = 0;
    n->next = NULL;

    return n;
}

UList *ulist_new(size_t size, size_t cap)
{
    UList *l;

    if (!(l = malloc(sizeof(*l)))) {
        FUNCFAILED("malloc", strerror(errno));
        abort();
    }

    l->size = size;
    l->lock_i = NO_LOCK;
    l->head = l->tail = ulist_node_new(l, cap);

    return l;
}

void ulist_free(UList *l)
{
    struct UListNode *node = l->head, *next;

    while (node) {
        next = node->next;
        free(node);
        node = next;
    }

    free(l);
}

static inline int is_locked(UList *l)
{
    return l->lock_i != NO_LOCK;
}

void ulist_append_arr(UList *l, void *arr, size_t len)
{
    struct UListNode *new, *node = l->tail;
    size_t cap = node->cap;

    while (node->len + len > cap)
        cap *= 2;

    if (cap != node->cap) {
        node->next = new = ulist_node_new(l, cap);

        if (is_locked(l)) {
            new->len += node->len - l->lock_i;
            memcpy(ULIST_BUF(*new), ULIST_BUF_AT(*l, *node, l->lock_i),
                   new->len * l->size);

            node->len = l->lock_i;
            l->lock_i = 0;
        }

        node = l->tail = new;
    }

    memcpy(ULIST_BUF_AT(*l, *node, node->len), arr, len * l->size);
    node->len += len;
}

void ulist_append(UList *l, void *val)
{
    ulist_append_arr(l, val, 1);
}

/*
 * Ensure that all the elements appended will be placed in memory
 * one after another. Useful for storing strings.
 */
void ulist_lock(UList *l)
{
    l->lock_i = l->tail->len;
}

void *ulist_unlock(UList *l)
{
    ssize_t i = l->lock_i;
    l->lock_i = NO_LOCK;

    return ULIST_BUF_AT(*l, *l->tail, i);
}
