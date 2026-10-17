#include <stdio.h>
#include "heap.h"


int main(int argc, char **argv){
    (void) argc;
    (void) argv;
    heap_inst_t* heap = heap_allocate();

    for (int i = 0; i < 10; ++i){
        printf("Add task with priority %d.\n", i);
        heap_add_item(heap, NULL, i, NULL);
        // dump_heap(heap);
    }

    printf("Heap size: %zu\n",heap_count(heap) );
    dump_heap(heap);

    printf("Add task with priority %d.\n", 5);
    heap_add_item(heap, NULL, 5, NULL);
    
    printf("Add task with priority %d.\n", 7);
    heap_add_item(heap, NULL, 7, NULL);

    printf("Add task with priority %d.\n", 10);
    heap_add_item(heap, NULL, 10, NULL);

    printf("Add task with priority %d.\n", 15);
    heap_add_item(heap, NULL, 15, NULL);

    heap_node_t out = {0};
    while (1){
        int ret = heap_pop(heap, &out);
        
        // dump_heap(heap);
        if (ret != 0 && ret != -2){
            printf("Pop returned ret = %d", ret);
            return ret;
        } else if (ret == -2)
            break;
        printf("Poped task with priority %d and id %zu.\n", out.prior, out.id);
    }

    heap_add_item(heap, NULL, 1, NULL); // id 0
    heap_add_item(heap, NULL, 1, NULL); // id 1
    heap_add_item(heap, NULL, 2, NULL); // id 2
    heap_add_item(heap, NULL, 1, NULL); // id 3

    printf("Heap size: %zu\n",heap_count(heap) );
    dump_heap(heap);

    out = (heap_node_t){0};
    while (1){
        int ret = heap_pop(heap, &out);
        
        // dump_heap(heap);
        if (ret != 0 && ret != -2){
            printf("Pop returned ret = %d", ret);
            return ret;
        } else if (ret == -2)
            break;
        printf("Poped task with priority %d and id %zu.\n", out.prior, out.id);
    }

    size_t priorities[] = {0, 0, 1, 1, 0, 1};

    for (size_t i = 0; i < 6; ++i){
        heap_add_item(heap, NULL, priorities[i], NULL);
        printf("Add task with priority %zu.\n", priorities[i]);
    }
    dump_heap(heap);    

    out = (heap_node_t){0};
    while (1){
        int ret = heap_pop(heap, &out);
        
        // dump_heap(heap);
        if (ret != 0 && ret != -2){
            printf("Pop returned ret = %d", ret);
            return ret;
        } else if (ret == -2)
            break;
        printf("Poped task with priority %d and id %zu.\n", out.prior, out.id);
    }

    heap_deallocate(heap);

    return 0;

}