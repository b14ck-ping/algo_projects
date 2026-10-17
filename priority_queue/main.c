#include <stdio.h>
#include "heap.h"


int main(int argc, char **argv){

    heap_inst_t* heap = heap_allocate();

    for (int i = 0; i < 10; ++i){
        printf("Add task with priority %d.\n", i);
        heap_add_item(heap, NULL, i, NULL);
        // dump_heap(heap);
    }

    printf("Heap size: %llu\n",heap_count(heap) );
    dump_heap(heap);

    printf("Add task with priority %d.\n", 5);
    heap_add_item(heap, NULL, 5, NULL);
    
    printf("Add task with priority %d.\n", 7);
    heap_add_item(heap, NULL, 7, NULL);

    printf("Add task with priority %d.\n", 10);
    heap_add_item(heap, NULL, 10, NULL);

    printf("Add task with priority %d.\n", 15);
    heap_add_item(heap, NULL, 15, NULL);

    printf("Heap size: %llu\n",heap_count(heap) );
    dump_heap(heap);

    heap_node_t out = {};
    while (1){
        int ret = heap_pop(heap, &out);
        if (ret != 0 && ret != -2){
            printf("Pop returned ret = %d", ret);
            return ret;
        } else if (ret == -2)
            break;
        printf("Poped task with priority %llu and id %llu.\n", out.prior, out.id);
        // dump_heap(heap);
    }

    return 0;

}