#include <stdio.h>
#include "heap.h"
#include "hash_table.h"

int test_heap(void)
{
    heap_inst_t* heap = heap_allocate();
    int ret = 0;
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
        ret = heap_pop(heap, &out);
        
        // dump_heap(heap);
        if (ret != 0 && ret != -2){
            printf("Pop returned ret = %d", ret);
            goto exit;
        } else if (ret == -2){
            ret = 0;
            break;
        }
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
        ret = heap_pop(heap, &out);
        
        // dump_heap(heap);
        if (ret != 0 && ret != -2){
            printf("Pop returned ret = %d", ret);
            goto exit;
        } else if (ret == -2){
            ret = 0;
            break;
        }
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
        ret = heap_pop(heap, &out);
        
        // dump_heap(heap);
        if (ret != 0 && ret != -2){
            printf("Pop returned ret = %d", ret);
            goto exit;
        } else if (ret == -2){
            ret = 0;
            break;
        }
        printf("Poped task with priority %d and id %zu.\n", out.prior, out.id);
    }

exit:
    heap_deallocate(heap);
    return ret;
}

int test_hash_table(void){
    int ret = 0;
    hash_table_t* ht = hash_table_create();

    // Check without increasing
    printf("Add values to hash table\n");
    for (uint32_t i = 0; i < 20; ++i){
        int ret = hash_table_insert(ht, i, i*5);
        if (ret)
            goto exit;
        // printf("hash_table inserted key %d value %zu\n", i, (size_t)i*5);
    }

    for (uint32_t i = 0; i < 20; ++i){
        size_t val = 0;
        // printf("hash_table get value by key %d\n", i);
        int ret = hash_table_get(ht, i, &val);
        if (ret && ret != -50){
            printf("hash_table_get returned %d\n", ret);
            goto exit;
        } else if (ret == -50) {
            printf("Can't find key %d\n", i);
        }
        if (val != i*5){
            printf("Wrong value %zu at key %d\n", val, i);
            goto exit;
        }  
        // printf("hash_table got value %zu\n", val);
    }

    // check with realocation 
    printf("Add values to hash table\n");
    for (uint32_t i = 20; i < 60; ++i){
        int ret = hash_table_insert(ht, i, i*5);
        if (ret)
            goto exit;
        printf("hash_table inserted key %d value %zu\n", i, (size_t)i*5);
    }

    for (uint32_t i = 0; i < 60; ++i){
        size_t val = 0;
        printf("hash_table get value by key %d\n", i);
        int ret = hash_table_get(ht, i, &val);
        if (ret && ret != -50){
            printf("hash_table_get returned %d\n", ret);
            goto exit;
        } else if (ret == -50) {
            printf("Can't find key %d\n", i);
        }
        if (val != i*5){
            printf("Wrong value %zu at key %d\n", val, i);
            goto exit;
        }  
        printf("hash_table got value %zu\n", val);
    }


exit:
    hash_table_free(ht);
    return ret;
}

int main(int argc, char **argv){
    (void) argc;
    (void) argv;
    
    int ret = test_heap();
    if (ret){
        printf("test_heap returned %d\n", ret);
        return ret;
    }
       

    ret = test_hash_table();
    if (ret){
        printf("test_hash_table returned %d\n", ret);
        return ret;
    }
        


    return 0;
}