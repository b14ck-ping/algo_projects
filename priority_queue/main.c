#include <assert.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

#define HT_CHECK(expr)                                      \
    do {                                                    \
        if (!(expr)) {                                      \
            fprintf(stderr, "FAIL line %d: %s\n",            \
                    __LINE__, #expr);                       \
            goto fail;                                      \
        }                                                   \
    } while (0)

static int expect_key(hash_table_t *ht, uint32_t key,
                      int exists, size_t expected)
{
    size_t actual = SIZE_MAX;
    int ret = hash_table_get(ht, key, &actual);

    if ((exists && (ret != 0 || actual != expected)) ||
        (!exists && ret != -50)) {
        fprintf(stderr,
                "key=%" PRIu32 ", ret=%d, actual=%zu, "
                "expected=%zu, exists=%d\n",
                key, ret, actual, expected, exists);
        return 0;
    }

    return 1;
}

static int test_random_case(size_t n,
                            uint32_t first_key,
                            uint32_t key_step)
{
    enum { MAX_N = 300 };

    uint32_t keys[MAX_N];
    size_t order[MAX_N];
    unsigned char removed[MAX_N] = {0};

    hash_table_t *ht = hash_table_create();
    HT_CHECK(ht != NULL);
    HT_CHECK(n > 1 && n <= MAX_N);

    // 1. Заполняем таблицу уникальными ключами.
    for (size_t i = 0; i < n; ++i) {
        keys[i] = first_key + (uint32_t)i * key_step;
        order[i] = i;

        HT_CHECK(hash_table_insert(ht, keys[i],
                                   1000 + i) == 0);
    }

    HT_CHECK(hash_table_count(ht) == (ssize_t)n);

    for (size_t i = 0; i < n; ++i)
        HT_CHECK(expect_key(ht, keys[i], 1, 1000 + i));

    // 2. Дубликат не должен перезаписывать значение.
    HT_CHECK(hash_table_insert(ht, keys[0], 99999) != 0);
    HT_CHECK(expect_key(ht, keys[0], 1, 1000));

    // 3. Перемешиваем индексы алгоритмом Fisher-Yates.
    srand(12345);

    for (size_t i = n - 1; i > 0; --i) {
        size_t j = (size_t)rand() % (i + 1);

        size_t tmp = order[i];
        order[i] = order[j];
        order[j] = tmp;
    }

    // 4. Удаляем случайную половину элементов.
    for (size_t i = 0; i < n / 2; ++i) {
        size_t idx = order[i];

        HT_CHECK(hash_table_remove(ht, keys[idx]) == 0);
        removed[idx] = 1;
    }

    HT_CHECK(hash_table_count(ht) ==
             (ssize_t)(n - n / 2));

    // 5. Проверяем ВСЕ ключи, а не только удалённые.
    for (size_t i = 0; i < n; ++i) {
        HT_CHECK(expect_key(ht, keys[i],
                            !removed[i], 1000 + i));
    }

    // 6. Повторное удаление отсутствующего ключа.
    HT_CHECK(hash_table_remove(ht, keys[order[0]]) == -50);

    // 7. Дубликат существующего ключа после удалений.
    size_t survivor = order[n - 1];

    HT_CHECK(hash_table_insert(ht, keys[survivor],
                               99999) != 0);
    HT_CHECK(expect_key(ht, keys[survivor],
                        1, 1000 + survivor));

    // 8. Повторно добавляем удалённые ключи.
    for (size_t i = 0; i < n; ++i) {
        if (removed[i]) {
            HT_CHECK(hash_table_insert(ht, keys[i],
                                       2000 + i) == 0);
        }
    }

    HT_CHECK(hash_table_count(ht) == (ssize_t)n);

    // 9. Проверяем всё после повторной вставки.
    for (size_t i = 0; i < n; ++i) {
        size_t expected = removed[i]
                        ? 2000 + i
                        : 1000 + i;

        HT_CHECK(expect_key(ht, keys[i], 1, expected));
    }

    printf("PASS: n=%zu, first=%" PRIu32
           ", step=%" PRIu32 "\n",
           n, first_key, key_step);

    hash_table_free(ht);
    return 0;

fail:
    fprintf(stderr, "FAILED: n=%zu, first=%" PRIu32
            ", step=%" PRIu32 "\n",
            n, first_key, key_step);

    hash_table_free(ht);
    return -1;
}

// Отдельный сценарий для ключа 0 после удаления.
static int test_zero_key_tombstone(void)
{
    hash_table_t *ht = hash_table_create();
    HT_CHECK(ht != NULL);

    // Оба ключа попадают в один бакет
    // при стартовой вместимости 40.
    HT_CHECK(hash_table_insert(ht, 40, 400) == 0);
    HT_CHECK(hash_table_insert(ht, 0, 123) == 0);

    HT_CHECK(hash_table_remove(ht, 40) == 0);

    // Ключ 0 должен остаться доступным,
    // несмотря на tombstone перед ним.
    HT_CHECK(expect_key(ht, 0, 1, 123));
    HT_CHECK(expect_key(ht, 40, 0, 0));

    puts("PASS: zero key after tombstone");

    hash_table_free(ht);
    return 0;

fail:
    puts("FAILED: zero key after tombstone");
    hash_table_free(ht);
    return -1;
}

int test_hash_table_extra(void)
{
    int failed = 0;

    // Без rehashing, все ключи дают один бакет.
    failed += test_random_case(24, 7, 40) != 0;

    // Много элементов, несколько расширений.
    failed += test_random_case(300, 11, 37) != 0;

    // Специальный граничный случай.
    failed += test_zero_key_tombstone() != 0;

    printf("\nHash table tests: %s (%d failed)\n",
           failed ? "FAILED" : "PASSED", failed);

    return failed ? -1 : 0;
}

int test_hash_table(void){
    int ret = 0;
    hash_table_t* ht = hash_table_create();

    // Check without increasing
    printf("Add values to hash table\n");
    for (uint32_t i = 0; i < 20; ++i){
        ret = hash_table_insert(ht, i, i*5);
        if (ret )
            goto exit;
        // printf("hash_table inserted key %d value %zu\n", i, (size_t)i*5);
    }

    for (uint32_t i = 0; i < 20; ++i){
        size_t val = 0;
        // printf("hash_table get value by key %d\n", i);
        ret = hash_table_get(ht, i, &val);
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
        ret = hash_table_insert(ht, i, i*5);
        if (ret)
            goto exit;
        // printf("hash_table inserted key %d value %zu\n", i, (size_t)i*5);
    }

    for (uint32_t i = 0; i < 60; ++i){
        size_t val = 0;
        // printf("hash_table get value by key %d\n", i);
        ret = hash_table_get(ht, i, &val);
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

    hash_table_insert(ht, 7, 100);
    ret = hash_table_insert(ht, 7, 200);
    assert(ret == -100);
    
    if(ret == -100)
        ret = 0;
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
        
    ret = test_hash_table_extra();
    if (ret)
        return ret;

    return 0;
}