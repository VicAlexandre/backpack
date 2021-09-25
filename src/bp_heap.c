/**
 * @file bp_heap.c
 * @author Matheus T. dos Santos (tenoriomatheus0@gmail.com)
 * @brief
 * @version 0.1.0
 * @date 19/09/2021
 *
 * @copyright Matheus T. dos Santos todos os direitos reservados (c) 2021
 *
 */
#include "bp_heap.h"

#ifdef _MSC_VER
#include <malloc.h>
#define __builtin_popcount _mm_popcnt_u32
#endif

#define BP_HEAP_PARENT(i) ((i) >> 1)

#define BP_HEAP_LEFT_CHILD(i) ((i) << 1)

#define BP_HEAP_RIGHT_CHILD(i) (((i) << 1) + 1)

#define BP_HEAP_LEVEL(i) log2fast(i)

#define BP_HEAP_DFS_END_NODE(size) ((2 << (log2fast((size) + 1) - 1)) - 1)

#define BP_HEAP_GET(heap_ptr, idx) bp_array_get(&(heap_ptr)->_coll, (idx) -1)

static int log2fast(unsigned int n);

static void bp_heap_swap(bp_heap_t *heap, usize idx1, usize idx2);

static void bp_heap_shift_up(bp_heap_t *heap, usize idx);

static void bp_heap_shift_down(bp_heap_t *heap, usize idx);

static void *bp_heap_iter_init(struct bp_iter *self);

static void *bp_heap_iter_get(struct bp_iter *self);

static bool bp_heap_bfs_iter_next(struct bp_iter *self);

static bool bp_heap_dfs_iter_next(struct bp_iter *self);

void *bp_heap_top(bp_heap_t *heap)
{
    if (heap == NULL) {
        return NULL;
    }

    if (heap->_coll._size == 0) {
        return NULL;
    }

    return BP_HEAP_GET(heap, 1);
}

int bp_heap_clear(bp_heap_t *heap)
{
    if (heap == NULL) {
        return -ENODEV;
    }

    heap->_coll._size = 0;

    return 0;
}

int bp_heap_pop(bp_heap_t *heap, void *el)
{
    if (heap == NULL || el == NULL) {
        return -ENODEV;
    }

    if (heap->_coll._size == 0) {
        return -ENOENT;
    }

    if (heap->_cmp == NULL) {
        return -EINVAL;
    }

    void *ptr = bp_heap_top(heap);
    memcpy(el, ptr, heap->_coll._element_size);

    bp_heap_swap(heap, 1, heap->_coll._size);
    heap->_coll._size -= 1;

    bp_heap_shift_down(heap, 1);

    return 0;
}

int bp_heap_push(bp_heap_t *heap, void *el)
{
    if (heap == NULL || el == NULL) {
        return -ENODEV;
    }

    if (heap->_cmp == NULL) {
        return -EINVAL;
    }

    int err;

    err = bp_array_push(&heap->_coll, el);
    if (err) {
        return err;
    }

    bp_heap_shift_up(heap, heap->_coll._size);

    return 0;
}

int bp_heap_del(bp_heap_t *heap, void *param, bool (*cmp)(void *el, void *param))
{
    if (heap == NULL || param == NULL) {
        return -ENODEV;
    }

    if (heap->_coll._size == 0) {
        return -ENOENT;
    }

    if (heap->_cmp == NULL) {
        return -EINVAL;
    }

    usize idx = BP_ARRAY_INVALID_INDEX;
    for (usize i = 1; i <= heap->_coll._size; ++i) {
        if (cmp != NULL) {
            if (cmp(BP_HEAP_GET(heap, i), param)) {
                idx = i;
                break;
            }
        } else {
            if (heap->_cmp(BP_HEAP_GET(heap, i), param) == 0) {
                idx = i;
                break;
            }
        }
    }
    if (idx == BP_ARRAY_INVALID_INDEX) {
        return -ENOENT;
    }

    if (idx == heap->_coll._size) {
        heap->_coll._size -= 1;
    } else {
        bp_heap_swap(heap, idx, heap->_coll._size);
        heap->_coll._size -= 1;

        bp_heap_shift_down(heap, idx);
    }

    return 0;
}

usize bp_heap_find_idx(bp_heap_t *heap, void *param, bool (*cmp)(void *el, void *param))
{
    for (usize i = 1; i <= heap->_coll._size; ++i) {
        void *ptr = BP_HEAP_GET(heap, i);
        if (cmp != NULL) {
            if (cmp(ptr, param)) {
                return i;
            }
        } else {
            if (heap->_cmp(ptr, param) == 0) {
                return i;
            }
        }
    }

    return BP_ARRAY_INVALID_INDEX;
}

void *bp_heap_find(bp_heap_t *heap, void *param, bool (*cmp)(void *el, void *param))
{
    for (usize i = 1; i <= heap->_coll._size; ++i) {
        void *ptr = BP_HEAP_GET(heap, i);
        if (cmp != NULL) {
            if (cmp(ptr, param)) {
                return ptr;
            }
        } else {
            if (heap->_cmp(ptr, param) == 0) {
                return ptr;
            }
        }
    }

    return NULL;
}

bp_iter_t bp_heap_bfs_iter(bp_heap_t *heap)
{
    bp_iter_t iter = {
        .init        = bp_heap_iter_init,
        .next        = bp_heap_bfs_iter_next,
        .get         = bp_heap_iter_get,
        .current.idx = 0,
        .coll        = heap,
    };

    return iter;
}

bp_iter_t bp_heap_dfs_iter(bp_heap_t *heap)
{
    bp_iter_t iter = {
        .init        = bp_heap_iter_init,
        .next        = bp_heap_dfs_iter_next,
        .get         = bp_heap_iter_get,
        .current.idx = 0,
        .coll        = heap,
    };

    return iter;
}

static int log2fast(unsigned int n)
{
    n |= (n >> 1);
    n |= (n >> 2);
    n |= (n >> 3);
    n |= (n >> 4);
    n |= (n >> 8);
    n |= (n >> 16);
    return (__builtin_popcount(n) - 1);
}

static void bp_heap_swap(bp_heap_t *heap, usize idx1, usize idx2)
{
#ifdef _MSC_VER
    u8_t *aux = (u8_t *) alloca(sizeof(u8_t) * heap->_coll._element_size);
#else
    u8_t aux[heap->_coll._element_size];
#endif

    void *el1 = BP_HEAP_GET(heap, idx1);
    void *el2 = BP_HEAP_GET(heap, idx2);

    memcpy(aux, el1, heap->_coll._element_size);
    memcpy(el1, el2, heap->_coll._element_size);
    memcpy(el2, aux, heap->_coll._element_size);
}

static void bp_heap_shift_up(bp_heap_t *heap, usize idx)
{
    void *el     = NULL;
    void *parent = NULL;
    usize swaps  = BP_HEAP_LEVEL(idx);

    for (usize i = 0; i < swaps; ++i) {
        el     = BP_HEAP_GET(heap, idx);
        parent = BP_HEAP_GET(heap, BP_HEAP_PARENT(idx));

        if (heap->_kind == BP_MIN_HEAP) {
            if (heap->_cmp(parent, el) <= 0) {
                break;
            }
        } else {
            if (heap->_cmp(parent, el) >= 0) {
                break;
            }
        }

        bp_heap_swap(heap, idx, BP_HEAP_PARENT(idx));
        idx = BP_HEAP_PARENT(idx);
    }
}

static void bp_heap_shift_down(bp_heap_t *heap, usize idx)
{
    void *el          = NULL;
    void *left_child  = NULL;
    void *right_child = NULL;
    void *max_el      = NULL;
    usize swaps       = BP_HEAP_LEVEL(heap->_coll._size) - BP_HEAP_LEVEL(idx);
    usize max_idx;

    for (usize i = 0; i < swaps; ++i) {
        el          = BP_HEAP_GET(heap, idx);
        left_child  = BP_HEAP_GET(heap, BP_HEAP_LEFT_CHILD(idx));
        right_child = BP_HEAP_GET(heap, BP_HEAP_RIGHT_CHILD(idx));
        max_idx     = idx;
        max_el      = el;

        /* Reach on the leaf. */
        if (left_child == NULL) {
            break;
        }

        if (heap->_kind == BP_MIN_HEAP) {
            if (heap->_cmp(max_el, left_child) > 0) {
                max_idx = BP_HEAP_LEFT_CHILD(idx);
                max_el  = left_child;
            }
        } else {
            if (heap->_cmp(max_el, left_child) < 0) {
                max_idx = BP_HEAP_LEFT_CHILD(idx);
                max_el  = left_child;
            }
        }

        if (right_child != NULL) {
            if (heap->_kind == BP_MIN_HEAP) {
                if (heap->_cmp(max_el, right_child) > 0) {
                    max_idx = BP_HEAP_RIGHT_CHILD(idx);
                    max_el  = right_child;
                }
            } else {
                if (heap->_cmp(max_el, right_child) < 0) {
                    max_idx = BP_HEAP_RIGHT_CHILD(idx);
                    max_el  = right_child;
                }
            }
        }

        if (max_idx != idx) {
            bp_heap_swap(heap, idx, max_idx);
            idx = max_idx;
        } else {
            break;
        }
    }
}

static void *bp_heap_iter_init(struct bp_iter *self)
{
    self->current.idx = 1;
    bp_heap_t *heap   = self->coll;

    return BP_HEAP_GET(heap, 1);
}

static void *bp_heap_iter_get(struct bp_iter *self)
{
    bp_heap_t *heap = self->coll;

    return BP_HEAP_GET(heap, self->current.idx);
}

static bool bp_heap_bfs_iter_next(struct bp_iter *self)
{
    bp_heap_t *heap = self->coll;
    if ((self->current.idx - 1) >= heap->_coll._size) {
        return false;
    }

    self->current.idx += 1;
    return true;
}

static bool bp_heap_dfs_iter_next(struct bp_iter *self)
{
    bp_heap_t *heap = self->coll;

    if (heap->_coll._size <= 3) {
        if ((self->current.idx - 1) >= heap->_coll._size) {
            self->current.idx += 1;
            return true;
        }

        return false;
    } else {
        // stop condition
        usize end_node = BP_HEAP_DFS_END_NODE(heap->_coll._size);
        if (self->current.idx == end_node) {
            return false;
        }

        usize left_idx = BP_HEAP_LEFT_CHILD(self->current.idx);
        void *left     = BP_HEAP_GET(heap, left_idx);
        if (left != NULL) {
            self->current.idx = left_idx;
        } else {
            usize right_idx = BP_HEAP_RIGHT_CHILD(BP_HEAP_PARENT(self->current.idx));
            void *right     = BP_HEAP_GET(heap, right_idx);
            if (right == NULL) {
                self->current.idx = BP_HEAP_RIGHT_CHILD(
                    BP_HEAP_PARENT(BP_HEAP_PARENT(self->current.idx)));
            } else if (right_idx == self->current.idx) {
                usize parent_idx = BP_HEAP_PARENT(self->current.idx);
                while ((parent_idx & 1) == 1) {
                    parent_idx = BP_HEAP_PARENT(parent_idx);
                }
                self->current.idx = BP_HEAP_RIGHT_CHILD(BP_HEAP_PARENT(parent_idx));
            } else {
                self->current.idx = right_idx;
            }
        }

        return true;
    }
}
