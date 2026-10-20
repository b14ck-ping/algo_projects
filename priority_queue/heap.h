#ifndef HEAP_H
#define HEAP_H

#include <stdio.h>
#include <stdlib.h>

typedef struct heap_inst heap_inst_t;
typedef struct heap_node{
    int prior;
    size_t id;

    void *data;
} heap_node_t;

heap_inst_t* heap_allocate(void);
void heap_deallocate(heap_inst_t*);
int heap_add_item(heap_inst_t *heap, void* item, int priority, heap_node_t* out);
int heap_peek_item(heap_inst_t *heap, heap_node_t* out);
size_t heap_count(heap_inst_t *heap);
int heap_pop(heap_inst_t *heap, heap_node_t* out);
void dump_heap(heap_inst_t *heap);

#endif //HEAP_H