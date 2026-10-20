
#include "addr_heap.h"
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>

#define MIN_count 20

typedef enum {
    HANDLE_VALID,
    HANDLE_INVALID
} handle_state_t;

typedef struct addr_heap_node{
    int prior;
    size_t sequence;
    addr_heap_handle_t *handle;

    void *data;
} addr_heap_node_t;

struct addr_heap_handle {
    addr_heap_inst_t *owner;
    size_t idx;
    handle_state_t state;
};

struct addr_heap_inst {
    size_t capacity;
    size_t count;
    size_t next_seq;
    addr_heap_node_t *arr;
};



static inline size_t _left_child(size_t i)
{
    return 2 * i + 1;
}

static inline size_t _right_child(size_t i)
{
    return 2 * i + 2;
}

static inline int _compare_priorities(addr_heap_node_t *x, addr_heap_node_t *y)
{
    if(x->prior > y->prior)
        return 1 ;
    else if (x->prior < y->prior)
        return -1;
    else 
        return x->sequence < y->sequence ? 1 : x->sequence > y->sequence ? -1 : 0;
}

static inline size_t _parent(size_t i)
{
    return (i - 1) / 2;
}

static inline void _swap(addr_heap_inst_t *heap, size_t x, size_t y) {
    addr_heap_node_t tmp = heap->arr[x];
    heap->arr[x] = heap->arr[y];
    heap->arr[x].handle->idx = x; 
    heap->arr[y] = tmp;
    heap->arr[y].handle->idx = y; 
}


static void shift_up(addr_heap_inst_t *heap, size_t item_ind)
{
    if (heap->count < 2 || !item_ind) return;

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


static void shift_down(addr_heap_inst_t *heap, size_t item_ind)
{
    if (heap->count < 2) return;
    size_t left_child = _left_child(item_ind);
    size_t right_child = _right_child(item_ind);
    while (item_ind < heap->count && (left_child < heap->count || right_child < heap->count)){
        if (left_child < heap->count && right_child < heap->count){
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
        }  else if (left_child < heap->count) {
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

static int realloc_array(addr_heap_inst_t *heap)
{
    size_t new_capacity = heap->capacity*sizeof(addr_heap_node_t) > SIZE_MAX/2 ? 
                                                SIZE_MAX/sizeof(addr_heap_node_t) : 
                                                heap->capacity*2;
    size_t new_size =  new_capacity*sizeof(addr_heap_node_t); 

    addr_heap_node_t * new_arr = (addr_heap_node_t*)realloc(heap->arr, new_size);
    if (!new_arr)
        return -3;

    heap->capacity = new_capacity;
    heap->arr = new_arr;
    return 0;
}

static inline void _invalidate_handle(addr_heap_handle_t* handle)
{
    if (!handle)
        return;
    handle->owner = NULL;
    handle->state = HANDLE_INVALID;
}

addr_heap_inst_t* addr_heap_allocate(void)
{
    addr_heap_inst_t *buf = (addr_heap_inst_t *)calloc(1, sizeof(addr_heap_inst_t));
    if (!buf)
        return NULL;

    buf->arr = (addr_heap_node_t *)calloc(MIN_count, sizeof(addr_heap_node_t));
    if (!buf->arr){
        free(buf);
        return NULL;
    }
        

    buf->capacity = MIN_count;

    return buf;
}

void addr_heap_deallocate(addr_heap_inst_t *heap)
{
    if (!heap)
        return;

    if (heap->arr){
        for (size_t i = 0;  i < heap->count; ++i)
            _invalidate_handle(heap->arr[i].handle);
        free(heap->arr);
    }

    free(heap);
}

int addr_heap_add_item(addr_heap_inst_t *heap, void* data, int priority, addr_heap_handle_t** out)
{
    if (!heap || !heap->arr || !out)
        return -1;

    addr_heap_handle_t *handle = (addr_heap_handle_t *)malloc(sizeof(addr_heap_handle_t));
    if (!handle)
        return -3;

    if (heap->count >= heap->capacity){
        int ret = realloc_array(heap);
        if (ret){
            free(handle);
            return ret;
        }
    }

    handle->owner = heap;
    handle->state = HANDLE_VALID;

    addr_heap_node_t buf = {0};
    buf.prior = priority;
    buf.sequence = heap->next_seq++;
    buf.data = data;
    buf.handle = handle;

    heap->arr[heap->count] = buf;
    handle->idx = heap->count;
    heap->count++;
    if (out){
        *out = handle;
    }
    shift_up(heap, heap->count -1);
    return 0;
}

int addr_heap_peek_item(addr_heap_inst_t *heap, addr_heap_item_t* out)
{
    if (!heap || !heap->arr || !out)
        return -1;

    if (!heap->count)
        return -2;

    out->handle = heap->arr[0].handle;
    out->prior = heap->arr[0].prior;
    out->data = heap->arr[0].data;

    return 0;
}

size_t addr_heap_count(addr_heap_inst_t *heap)
{
    return heap->count;
}

int addr_heap_pop(addr_heap_inst_t *heap, addr_heap_item_t* out)
{
    if (!heap || !heap->arr || !out)
        return -1;

    if (!heap->count)
        return -2;

    out->handle = heap->arr[0].handle;
    out->prior = heap->arr[0].prior;
    out->data = heap->arr[0].data;
    _invalidate_handle(out->handle);

    if (heap->count > 1){
        heap->arr[0] = heap->arr[heap->count-1];
        heap->arr[0].handle->idx = 0;
        heap->count--;
        shift_down(heap, 0);
        return 0;
    }
    heap->count--;

    return 0;
}

void addr_heap_dump_heap(addr_heap_inst_t *heap){

    if (!heap->count) return;
    if (heap->count == 1){
        printf("[p: %d id: %zu]\n", heap->arr[0].prior, heap->arr[0].sequence);
        return;
    }
    for (size_t i = 0; i < heap->count; i = i*2 + 1){
        for (size_t j = i; j <= i*2 && j < heap->count; ++j){
            printf("[p: %d id: %zu] ", heap->arr[j].prior, heap->arr[j].sequence);
        }
        printf("\n");
    }
}

static void shift_specific_item(addr_heap_inst_t *heap, size_t idx)
{
    if (heap->count < 2 ) return;

    size_t left_child = _left_child(idx);
    size_t right_child = _right_child(idx);
    if (idx == 0 || (left_child < heap->count && _compare_priorities(&heap->arr[idx], &heap->arr[left_child]) == -1) ||
        (right_child < heap->count && _compare_priorities(&heap->arr[idx], &heap->arr[right_child]) == -1)){
        
        shift_down(heap, idx);
        return;
    }
        
    size_t parent = _parent(idx);
    int ret = _compare_priorities(&heap->arr[idx], &heap->arr[parent]);
    if (ret == 1){
        shift_up(heap, idx);
    }
} 

int addr_heap_set_priority(addr_heap_inst_t *heap, int priority, addr_heap_handle_t* handle)
{
    if (!heap || !handle)
        return -1;

    if(handle->state == HANDLE_INVALID)
        return -6;

    if (handle->owner != heap)
        return -7;
    
    if (handle->idx >= heap->count)
        return -8;

    heap->arr[handle->idx].prior = priority;
    shift_specific_item(heap, handle->idx);
    return 0;

}

int addr_heap_get_item(addr_heap_inst_t *heap, addr_heap_handle_t* handle, addr_heap_item_t* out)
{
    if (!heap || !handle || !out)
        return -1;

    if(handle->state == HANDLE_INVALID)
        return -6;

    if (handle->owner != heap)
        return -7;
    
    if (handle->idx >= heap->count)
        return -8;

    out->prior = heap->arr[handle->idx].prior;
    out->handle = heap->arr[handle->idx].handle;
    out->data = heap->arr[handle->idx].data;

    return 0;
}

int addr_heap_remove_item(addr_heap_inst_t *heap, addr_heap_handle_t* handle,  addr_heap_item_t* out)
{
    if (!heap || !handle || !out)
        return -1;

    if(handle->state == HANDLE_INVALID)
        return -6;

    if (handle->owner != heap)
        return -7;
    
    if (handle->idx >= heap->count)
        return -8;

    out->prior = heap->arr[handle->idx].prior;
    out->handle = heap->arr[handle->idx].handle;
    out->data = heap->arr[handle->idx].data;

    if (heap->count < 2){
        _invalidate_handle(out->handle);
        heap->count--;
        return 0;
    } 

    _invalidate_handle(out->handle);
    heap->arr[handle->idx] = heap->arr[heap->count-1];
    heap->arr[handle->idx].handle->idx = handle->idx;
    heap->count--;
    shift_specific_item(heap, handle->idx);
    
    return 0;
}

int addr_heap_free_handler(addr_heap_handle_t* handle)
{
    if (!handle || handle->owner || handle->state != HANDLE_INVALID)
        return -30;

    free(handle);
    return 0;
}
