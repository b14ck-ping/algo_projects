
#include "heap.h"
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>

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

static inline int _compare_priorities(heap_node_t *x, heap_node_t *y)
{
    if(x->prior > y->prior)
        return 1 ;
    else if (x->prior < y->prior)
        return -1;
    else 
        return x->id < y->id ? 1 : x->id > y->id ? -1 : 0;
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


static void shift_up(heap_inst_t *heap)
{
    if (heap->heap_size < 2) return;
    size_t item_ind = heap->heap_size - 1;

    while (item_ind != 0 ){
        size_t parent = _parent(item_ind);
        if (_compare_priorities(&heap->arr[item_ind], &heap->arr[parent]) == 1 ){
            _swap(heap, item_ind, parent);
            item_ind = parent;
        } else {
            return;
        }
    }

    return;
}


static void shift_down(heap_inst_t *heap)
{
    if (heap->heap_size < 2) return;
    size_t item_ind = 0;
    size_t left_child = _left_child(item_ind);
    size_t right_child = _right_child(item_ind);
    while (item_ind < heap->heap_size && (left_child < heap->heap_size || right_child < heap->heap_size)){
        if (left_child < heap->heap_size && right_child < heap->heap_size){
            int ret = _compare_priorities(&heap->arr[right_child], &heap->arr[left_child]);
            if (ret == 1 ){
                if (_compare_priorities(&heap->arr[item_ind], &heap->arr[right_child]) == -1 ){
                    _swap(heap, item_ind, right_child);
                    item_ind = right_child;
                } else if (_compare_priorities(&heap->arr[item_ind], &heap->arr[left_child]) == -1 ){
                    _swap(heap, item_ind, left_child);
                    item_ind = left_child;
                } else 
                    return;
            } else if (ret == -1) {
                if (_compare_priorities(&heap->arr[item_ind], &heap->arr[left_child]) == -1 ){
                    _swap(heap, item_ind, left_child);
                    item_ind = left_child;
                } else if (_compare_priorities(&heap->arr[item_ind], &heap->arr[right_child]) == -1 ){
                    _swap(heap, item_ind, right_child);
                    item_ind = right_child;
                } else 
                    return;
            } else {
                return;
            }
        }  else if (left_child < heap->heap_size) {
            if (_compare_priorities(&heap->arr[item_ind], &heap->arr[left_child]) == -1 ){
                _swap(heap, item_ind, left_child);
                item_ind = left_child;
            } else {
                return;
            }
        } else {
            return;
        }
        left_child = _left_child(item_ind);
        right_child = _right_child(item_ind);
    }
}


heap_inst_t* heap_allocate(void)
{
    heap_inst_t *buf = (heap_inst_t *)calloc(1, sizeof(heap_inst_t));
    if (!buf)
        return NULL;

    buf->arr = (heap_node_t *)calloc(MIN_HEAP_SIZE, sizeof(heap_node_t));
    if (!buf->arr){
        free(buf);
        return NULL;
    }
        

    buf->max_size = MIN_HEAP_SIZE;

    return buf;
}

static int realloc_array(heap_inst_t *heap)
{
    size_t new_capacity = heap->max_size*sizeof(heap_node_t) > SIZE_MAX/2 ? 
                                                SIZE_MAX/sizeof(heap_node_t) : 
                                                heap->max_size*2;
    size_t new_size =  new_capacity*sizeof(heap_node_t); 

    heap_node_t * new_arr = (heap_node_t*)realloc(heap->arr, new_size);
    if (!new_arr)
        return -3;

    heap->max_size = new_capacity;
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

    heap_node_t buf = {0};
    buf.prior = priority;
    buf.id = heap->next_id++;
    buf.data = item;

    if (heap->heap_size >= heap->max_size){
        int ret = realloc_array(heap);
        if (ret)
            return ret;
    }
    heap->arr[heap->heap_size] = buf;
    heap->heap_size++;
    if (out) *out = buf;
    shift_up(heap);
    return 0;
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
    shift_down(heap);
    return 0;
}

void dump_heap(heap_inst_t *heap){

    if (!heap->heap_size) return;
    if (heap->heap_size == 1){
        printf("[p: %d id: %zu]\n", heap->arr[0].prior, heap->arr[0].id);
        return;
    }
    for (size_t i = 0; i < heap->heap_size; i = i*2 + 1){
        for (size_t j = i; j <= i*2 && j < heap->heap_size; ++j){
            printf("[p: %d id: %zu] ", heap->arr[j].prior, heap->arr[j].id);
        }
        printf("\n");
    }
}