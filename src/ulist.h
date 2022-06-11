#ifndef ULIST_H
#define ULIST_H

#include <stdlib.h>

/*
 * Unrolled linked list
 */

typedef struct UList UList;

UList *ulist_new(size_t size, size_t cap);
void ulist_free(UList *l);
void ulist_append_arr(UList *l, void *arr, size_t len);
void ulist_append(UList *l, void *val);
void ulist_lock(UList *l);
void *ulist_unlock(UList *l);

#endif
