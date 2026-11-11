
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "addr_heap.h"

#define CHECK(expr)                                             \
    do {                                                        \
        if (!(expr)) {                                          \
            fprintf(stderr, "[FAIL] %s:%d: %s\n",               \
                    __FILE__, __LINE__, #expr);                \
            exit(EXIT_FAILURE);                                 \
        }                                                       \
    } while (0)

#define RUN_TEST(test)                                          \
    do {                                                        \
        printf("[RUN ] %s\n", #test);                            \
        test();                                                 \
        printf("[PASS] %s\n", #test);                            \
    } while (0)


static addr_heap_inst_t *make_heap(heap_type_t type)
{
    addr_heap_inst_t *heap = addr_heap_allocate(type);
    CHECK(heap != NULL);
    return heap;
}

static void push_checked(addr_heap_inst_t *heap,
                         void *data,
                         uint64_t priority,
                         addr_heap_handle_t **handle)
{
    CHECK(addr_heap_add_item(heap, data, priority, handle) == 0);
    CHECK(*handle != NULL);
}

static void pop_checked(addr_heap_inst_t *heap,
                        void *expected_data,
                        uint64_t expected_priority)
{
    addr_heap_item_t item = {0};

    CHECK(addr_heap_pop(heap, &item) == 0);
    CHECK(item.data == expected_data);
    CHECK(item.prior == expected_priority);

    // pop() инвалидирует handle, но не освобождает его
    CHECK(addr_heap_free_handler(item.handle) == 0);
}


/* ---------- Empty heap ---------- */

static void test_empty_heap(void)
{
    for (int mode = HEAP_MIN; mode <= HEAP_MAX; ++mode) {
        addr_heap_inst_t *heap = make_heap((heap_type_t)mode);
        addr_heap_item_t item = {0};

        CHECK(addr_heap_count(heap) == 0);
        CHECK(addr_heap_peek_item(heap, &item) == -2);
        CHECK(addr_heap_pop(heap, &item) == -2);

        addr_heap_deallocate(heap);
    }
}


/* ---------- Basic MIN order ---------- */

static void test_min_order(void)
{
    addr_heap_inst_t *heap = make_heap(HEAP_MIN);

    int values[] = {0, 1, 2, 3, 4};
    uint64_t priorities[] = {40, 10, 30, 20, 50};
    addr_heap_handle_t *handles[5] = {0};

    for (int i = 0; i < 5; ++i)
        push_checked(heap, &values[i], priorities[i], &handles[i]);

    CHECK(addr_heap_count(heap) == 5);

    addr_heap_item_t item = {0};
    CHECK(addr_heap_peek_item(heap, &item) == 0);
    CHECK(item.data == &values[1]);
    CHECK(item.prior == 10);

    const int order[] = {1, 3, 2, 0, 4};

    for (int i = 0; i < 5; ++i)
        pop_checked(heap, &values[order[i]], priorities[order[i]]);

    CHECK(addr_heap_count(heap) == 0);

    addr_heap_deallocate(heap);
}


/* ---------- Basic MAX order ---------- */

static void test_max_order(void)
{
    addr_heap_inst_t *heap = make_heap(HEAP_MAX);

    int values[] = {0, 1, 2, 3, 4};
    uint64_t priorities[] = {40, 10, 30, 20, 50};
    addr_heap_handle_t *handles[5] = {0};

    for (int i = 0; i < 5; ++i)
        push_checked(heap, &values[i], priorities[i], &handles[i]);

    CHECK(addr_heap_count(heap) == 5);

    const int order[] = {4, 0, 2, 3, 1};

    for (int i = 0; i < 5; ++i)
        pop_checked(heap, &values[order[i]], priorities[order[i]]);

    CHECK(addr_heap_count(heap) == 0);

    addr_heap_deallocate(heap);
}


/* ---------- Stable order with equal priorities ---------- */

static void check_stability(heap_type_t type)
{
    enum { COUNT = 64 };

    addr_heap_inst_t *heap = make_heap(type);

    int values[COUNT];
    addr_heap_handle_t *handles[COUNT] = {0};

    // Все элементы имеют одинаковый приоритет.
    // Заодно проверяем несколько расширений массива.
    for (int i = 0; i < COUNT; ++i) {
        values[i] = i;
        push_checked(heap, &values[i], 100, &handles[i]);
    }

    CHECK(addr_heap_count(heap) == COUNT);

    // В обоих режимах должен сохраняться FIFO.
    for (int i = 0; i < COUNT; ++i)
        pop_checked(heap, &values[i], 100);

    CHECK(addr_heap_count(heap) == 0);

    addr_heap_deallocate(heap);
}

static void test_min_stability(void)
{
    check_stability(HEAP_MIN);
}

static void test_max_stability(void)
{
    check_stability(HEAP_MAX);
}


/* ---------- MIN: decrease-key ---------- */

static void test_min_decrease_key(void)
{
    addr_heap_inst_t *heap = make_heap(HEAP_MIN);

    int values[] = {0, 1, 2};
    addr_heap_handle_t *handles[3] = {0};

    push_checked(heap, &values[0], 10, &handles[0]);
    push_checked(heap, &values[1], 20, &handles[1]);
    push_checked(heap, &values[2], 30, &handles[2]);

    // Третий элемент должен подняться в корень.
    CHECK(addr_heap_set_priority(heap, 5, handles[2]) == 0);

    pop_checked(heap, &values[2], 5);
    pop_checked(heap, &values[0], 10);
    pop_checked(heap, &values[1], 20);

    addr_heap_deallocate(heap);
}


/* ---------- MIN: increase-key ---------- */

static void test_min_increase_key(void)
{
    addr_heap_inst_t *heap = make_heap(HEAP_MIN);

    int values[] = {0, 1, 2};
    addr_heap_handle_t *handles[3] = {0};

    push_checked(heap, &values[0], 10, &handles[0]);
    push_checked(heap, &values[1], 20, &handles[1]);
    push_checked(heap, &values[2], 30, &handles[2]);

    // Корень должен опуститься вниз.
    CHECK(addr_heap_set_priority(heap, 40, handles[0]) == 0);

    pop_checked(heap, &values[1], 20);
    pop_checked(heap, &values[2], 30);
    pop_checked(heap, &values[0], 40);

    addr_heap_deallocate(heap);
}


/* ---------- MAX: increase-key ---------- */

static void test_max_increase_key(void)
{
    addr_heap_inst_t *heap = make_heap(HEAP_MAX);

    int values[] = {0, 1, 2};
    addr_heap_handle_t *handles[3] = {0};

    push_checked(heap, &values[0], 10, &handles[0]);
    push_checked(heap, &values[1], 20, &handles[1]);
    push_checked(heap, &values[2], 30, &handles[2]);

    CHECK(addr_heap_set_priority(heap, 40, handles[0]) == 0);

    pop_checked(heap, &values[0], 40);
    pop_checked(heap, &values[2], 30);
    pop_checked(heap, &values[1], 20);

    addr_heap_deallocate(heap);
}


/* ---------- MAX: decrease-key ---------- */

static void test_max_decrease_key(void)
{
    addr_heap_inst_t *heap = make_heap(HEAP_MAX);

    int values[] = {0, 1, 2};
    addr_heap_handle_t *handles[3] = {0};

    push_checked(heap, &values[0], 10, &handles[0]);
    push_checked(heap, &values[1], 20, &handles[1]);
    push_checked(heap, &values[2], 30, &handles[2]);

    CHECK(addr_heap_set_priority(heap, 5, handles[2]) == 0);

    pop_checked(heap, &values[1], 20);
    pop_checked(heap, &values[0], 10);
    pop_checked(heap, &values[2], 5);

    addr_heap_deallocate(heap);
}


/* ---------- Stability after changing priority ---------- */

static void test_stability_after_update(void)
{
    addr_heap_inst_t *heap = make_heap(HEAP_MIN);

    int values[] = {0, 1, 2};
    addr_heap_handle_t *handles[3] = {0};

    for (int i = 0; i < 3; ++i)
        push_checked(heap, &values[i], 10, &handles[i]);

    // Временно меняем приоритет первого элемента.
    CHECK(addr_heap_set_priority(heap, 20, handles[0]) == 0);
    CHECK(addr_heap_set_priority(heap, 10, handles[0]) == 0);

    // Приоритеты снова равны.
    // Порядок первоначального добавления должен сохраниться.
    pop_checked(heap, &values[0], 10);
    pop_checked(heap, &values[1], 10);
    pop_checked(heap, &values[2], 10);

    addr_heap_deallocate(heap);
}


/* ---------- Handle lifetime ---------- */

static void test_handle_after_destroy(void)
{
    addr_heap_inst_t *heap = make_heap(HEAP_MIN);

    int value = 123;
    addr_heap_handle_t *handle = NULL;

    push_checked(heap, &value, 10, &handle);

    // Деструктор инвалидирует оставшийся handle.
    addr_heap_deallocate(heap);

    CHECK(addr_heap_free_handler(handle) == 0);
}


int main(void)
{
    RUN_TEST(test_empty_heap);

    RUN_TEST(test_min_order);
    RUN_TEST(test_max_order);

    RUN_TEST(test_min_stability);
    RUN_TEST(test_max_stability);

    RUN_TEST(test_min_decrease_key);
    RUN_TEST(test_min_increase_key);

    RUN_TEST(test_max_increase_key);
    RUN_TEST(test_max_decrease_key);

    RUN_TEST(test_stability_after_update);
    RUN_TEST(test_handle_after_destroy);

    puts("\nAll addressable heap tests passed!");
    return EXIT_SUCCESS;
}
