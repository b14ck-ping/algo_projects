
#include "heap.h"
#include <stddef.h>
#include <stdio.h>

#define MIN_HEAP_SIZE 20

struct heap_inst {
    size_t max_size;
    size_t heap_size;
    size_t next_id;
    heap_node_t *arr;
};

static inline size_t _left_child(size_t i)
{
    return 2 * i + 1;
}

static inline size_t _right_child(size_t i)
{
    return 2 * i + 2;
}

static inline int _compare(size_t x, size_t y)
{
    return x > y ? 1 : x < y ? -1 : 0;
}

static inline size_t _parent(size_t i)
{
    return (i - 1) / 2;
}

static inline void _swap(heap_inst_t *heap, size_t x, size_t y) {
    heap_node_t tmp = heap->arr[x];
    heap->arr[x] = heap->arr[y];
    heap->arr[y] = tmp;
}


static int shift_up(heap_inst_t *heap)
{
    if (heap->heap_size < 2) return 0;
    size_t item_ind = heap->heap_size - 1;
    for(size_t i = heap->heap_size-1; i>=0; --i){
        size_t parent = _parent(item_ind);
        if ((heap->arr[item_ind].prior > heap->arr[parent].prior) ||
            (heap->arr[item_ind].prior == heap->arr[parent].prior &&
             heap->arr[item_ind].id < heap->arr[parent].id)){
            _swap(heap, item_ind, parent);
            item_ind = parent;
        } else {
            return 0;
        }

        if (item_ind == 0) return 0;
    }

    return -4;
}


static int shift_down(heap_inst_t *heap)
{
    size_t item_ind = 0;
    for(size_t i = heap->heap_size-1; i>=0; --i){
        size_t left_child = _left_child(item_ind);
        size_t right_child = _right_child(item_ind);

        if (left_child < heap->heap_size && right_child < heap->heap_size){
            // Node has both children
            if (heap->arr[item_ind].prior < heap->arr[left_child].prior &&
                         heap->arr[item_ind].prior < heap->arr[right_child].prior){
                // Both children has less priority than current node        
                int res = _compare(heap->arr[left_child].prior, 
                                    heap->arr[right_child].prior);
                // Choose the biggest one
                if (res > 0){
                    _swap(heap, item_ind, left_child);
                    item_ind = left_child;
                } else if (res < 0){
                    _swap(heap, item_ind, right_child);
                    item_ind = right_child;
                } else {
                    // if both child is equal search for less id
                    if (heap->arr[right_child].id > heap->arr[left_child].id){
                        _swap(heap, item_ind, left_child);
                        item_ind = left_child;
                    } else {
                        _swap(heap, item_ind, right_child);
                        item_ind = right_child;
                    }
                }
            } else if (heap->arr[item_ind].prior <= heap->arr[left_child].prior){
                // Only left child
                if (heap->arr[item_ind].prior < heap->arr[left_child].prior ||
                    (heap->arr[item_ind].prior == heap->arr[left_child].prior &&
                        heap->arr[item_ind].id > heap->arr[left_child].id)){
                    _swap(heap, item_ind, left_child);
                    item_ind = left_child;
                } else 
                    return 0;
            } else if (heap->arr[item_ind].prior <= heap->arr[right_child].prior){
                // Only left child
                if (heap->arr[item_ind].prior < heap->arr[right_child].prior || 
                    (heap->arr[item_ind].prior == heap->arr[right_child].prior &&
                        heap->arr[item_ind].id > heap->arr[right_child].id)){
                    _swap(heap, item_ind, right_child);
                    item_ind = right_child;
                } else 
                    return 0;
            } else 
                return 0;
        } else if (left_child < heap->heap_size) {
            // Node has only left child
            if (heap->arr[item_ind].prior < heap->arr[left_child].prior ||
                (heap->arr[item_ind].prior == heap->arr[left_child].prior &&
                        heap->arr[item_ind].id > heap->arr[left_child].id)){
                _swap(heap, item_ind, left_child);
                item_ind = left_child;
            } else 
                return 0;
        } else if (right_child < heap->heap_size) {
            // Node has only right child
            if (heap->arr[item_ind].prior < heap->arr[right_child].prior|| 
                    (heap->arr[item_ind].prior == heap->arr[right_child].prior &&
                        heap->arr[item_ind].id > heap->arr[right_child].id)){
                _swap(heap, item_ind, right_child);
                item_ind = right_child;
            } else 

                return 0;
        } else {
            return 0;
        }
    }
    return -100;
}


heap_inst_t* heap_allocate()
{
    heap_inst_t *buf = (heap_inst_t *)calloc(1, sizeof(heap_inst_t));
    if (!buf)
        return NULL;

    buf->arr = (heap_node_t *)calloc(MIN_HEAP_SIZE, sizeof(heap_node_t));
    if (!buf->arr){
        return NULL;
        free(buf);
    }
        

    buf->max_size = MIN_HEAP_SIZE;

    return buf;
}

static int realloc_array(heap_inst_t *heap)
{
    heap_node_t * new_arr = (heap_node_t*)realloc(heap->arr, heap->heap_size + MIN_HEAP_SIZE);
    if (!new_arr)
        return -3;

    heap->heap_size += MIN_HEAP_SIZE;
    heap->arr = new_arr;
    return 0;
}

void heap_deallocate(heap_inst_t *heap)
{
    free(heap->arr);
    free(heap);
}

int heap_add_item(heap_inst_t *heap, void* item, int priority, heap_node_t* out)
{
    if (!heap || !heap->arr)
        return -1;

    heap_node_t buf = {};
    buf.prior = priority;
    buf.id = heap->next_id++;

    if (heap->heap_size > heap->max_size){
        int ret = realloc_array(heap);
        if (ret)
            return ret;
    }
    heap->arr[heap->heap_size] = buf;
    heap->heap_size++;
    if (out) *out = buf;

    return shift_up(heap);
}

int heap_peek_item(heap_inst_t *heap, heap_node_t* out)
{
    if (!heap || !heap->arr || !out)
        return -1;

    if (!heap->heap_size)
        return -2;

    *out = heap->arr[0];
    return 0;
}

size_t heap_count(heap_inst_t *heap)
{
    return heap->heap_size;
}

int heap_pop(heap_inst_t *heap, heap_node_t* out)
{
    if (!heap || !heap->arr || !out)
        return -1;

    if (!heap->heap_size)
        return -2;

    *out = heap->arr[0];
    heap->arr[0] = heap->arr[heap->heap_size-1];
    heap->heap_size--;

    return shift_down(heap);
}

void dump_heap(heap_inst_t *heap){

    if (!heap->heap_size) return;
    if (heap->heap_size == 1){
        printf("[p: %llu id: %llu]\n", heap->arr[0].prior, heap->arr[0].id);
        return;
    }
    for (size_t i = 0; i < heap->heap_size; i = i*2 + 1){
        for (size_t j = i; j <= i*2 && j < heap->heap_size; ++j){
            printf("[p: %llu id: %llu] ", heap->arr[j].prior, heap->arr[j].id);
        }
        printf("\n");
    }
}