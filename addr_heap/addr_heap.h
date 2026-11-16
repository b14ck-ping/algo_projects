#ifndef ADDR_HEAP_H
#define ADDR_HEAP_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef enum {
    HEAP_MIN,
    HEAP_MAX
} heap_type_t;

typedef struct addr_heap_inst addr_heap_inst_t;
typedef struct addr_heap_handle addr_heap_handle_t;

typedef struct addr_heap_item{
    uint64_t prior;
    addr_heap_handle_t *handle;

    void *data;
} addr_heap_item_t;

addr_heap_inst_t* addr_heap_allocate(heap_type_t type);
void addr_heap_deallocate(addr_heap_inst_t*);

int addr_heap_add_item(addr_heap_inst_t *heap, void* data, uint64_t priority, addr_heap_handle_t** handle);
int addr_heap_peek_item(addr_heap_inst_t *heap, addr_heap_item_t* out);
int addr_heap_pop(addr_heap_inst_t *heap, addr_heap_item_t* out);

int addr_heap_set_priority(addr_heap_inst_t *heap, uint64_t priority, addr_heap_handle_t* handle);
int addr_heap_get_item(addr_heap_inst_t *heap, addr_heap_handle_t* handle, addr_heap_item_t* out);
int addr_heap_remove_item(addr_heap_inst_t *heap, addr_heap_handle_t* handle,  addr_heap_item_t* out);

int addr_heap_free_handler(addr_heap_handle_t* handle);

size_t addr_heap_count(addr_heap_inst_t *heap);

void addr_heap_dump_heap(addr_heap_inst_t *heap);

#endif //ADDR_HEAP_H