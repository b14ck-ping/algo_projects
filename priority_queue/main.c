#include <assert.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "heap.h"
#include "hash_table.h"
#include "addr_heap.h"



#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "prior_queue.h"

#define PQ_CHECK(expr) do {                                      \
    if (!(expr)) {                                               \
        fprintf(stderr,                                         \
                "PQ FAIL %s:%d: %s\n",                            \
                __FILE__, __LINE__, #expr);                      \
        exit(EXIT_FAILURE);                                     \
    }                                                            \
} while (0)

/* -------- 1. Basic operations and edge cases -------- */

static void test_pq_basic(void)
{
    pri_queue_t *pq = pri_queue_create();
    PQ_CHECK(pq != NULL);

    int data[] = {11, 22, 33, 44};
    pq_item_t out = {0};

    PQ_CHECK(pri_queue_count(pq) == 0);
    PQ_CHECK(pri_queue_peek(pq, &out) != 0);
    PQ_CHECK(pri_queue_pop(pq, &out) != 0);
    PQ_CHECK(pri_queue_remove(pq, 123) != 0);
    PQ_CHECK(pri_queue_set(pq, 123, 5) != 0);

    PQ_CHECK(pri_queue_add(pq, 100, 5, &data[0]) == 0);
    PQ_CHECK(pri_queue_add(pq, 200, 10, &data[1]) == 0);
    PQ_CHECK(pri_queue_add(pq, 300, -5, &data[2]) == 0);

    PQ_CHECK(pri_queue_count(pq) == 3);

    /* Duplicate ID must not overwrite an existing item. */
    PQ_CHECK(pri_queue_add(pq, 100, 999, &data[3]) != 0);
    PQ_CHECK(pri_queue_count(pq) == 3);

    PQ_CHECK(pri_queue_peek(pq, &out) == 0);
    PQ_CHECK(out.id == 200);
    PQ_CHECK(out.priority == 10);
    PQ_CHECK(out.data == &data[1]);

    /* Increase a non-root priority. */
    PQ_CHECK(pri_queue_set(pq, 300, 20) == 0);
    PQ_CHECK(pri_queue_peek(pq, &out) == 0);
    PQ_CHECK(out.id == 300);
    PQ_CHECK(out.priority == 20);

    /* Decrease the root priority. */
    PQ_CHECK(pri_queue_set(pq, 300, INT_MIN) == 0);
    PQ_CHECK(pri_queue_peek(pq, &out) == 0);
    PQ_CHECK(out.id == 200);

    /* Remove an arbitrary item. */
    PQ_CHECK(pri_queue_remove(pq, 200) == 0);
    PQ_CHECK(pri_queue_remove(pq, 200) != 0);
    PQ_CHECK(pri_queue_count(pq) == 2);

    PQ_CHECK(pri_queue_pop(pq, &out) == 0);
    PQ_CHECK(out.id == 100);
    PQ_CHECK(out.priority == 5);
    PQ_CHECK(out.data == &data[0]);

    PQ_CHECK(pri_queue_pop(pq, &out) == 0);
    PQ_CHECK(out.id == 300);
    PQ_CHECK(out.priority == INT_MIN);
    PQ_CHECK(out.data == &data[2]);

    PQ_CHECK(pri_queue_count(pq) == 0);
    PQ_CHECK(pri_queue_pop(pq, &out) != 0);

    /* Reuse an ID after removal. */
    PQ_CHECK(pri_queue_add(pq, 200, INT_MAX, &data[3]) == 0);
    PQ_CHECK(pri_queue_pop(pq, &out) == 0);
    PQ_CHECK(out.id == 200);
    PQ_CHECK(out.data == &data[3]);

    pri_queue_destroy(pq);
    puts("PASS: pq basic");
}

/* -------- 2. FIFO and priority changes -------- */

static void test_pq_fifo(void)
{
    pri_queue_t *pq = pri_queue_create();
    PQ_CHECK(pq != NULL);

    int data[6] = {0};
    const uint32_t ids[] = {50, 10, 90, 20, 70, 30};

    for (size_t i = 0; i < 6; ++i)
        PQ_CHECK(pri_queue_add(pq, ids[i], 5, &data[i]) == 0);

    /* Changing priority must preserve insertion order. */
    PQ_CHECK(pri_queue_set(pq, 20, 15) == 0);
    PQ_CHECK(pri_queue_set(pq, 20, 5) == 0);

    PQ_CHECK(pri_queue_set(pq, 90, -10) == 0);
    PQ_CHECK(pri_queue_set(pq, 90, 5) == 0);

    for (size_t i = 0; i < 6; ++i) {
        pq_item_t out = {0};
        PQ_CHECK(pri_queue_pop(pq, &out) == 0);
        PQ_CHECK(out.id == ids[i]);
        PQ_CHECK(out.priority == 5);
        PQ_CHECK(out.data == &data[i]);
    }

    PQ_CHECK(pri_queue_count(pq) == 0);
    pri_queue_destroy(pq);

    puts("PASS: pq FIFO");
}

/* -------- 3. Large amount of insertions/removals -------- */

static void test_pq_growth(void)
{
    enum { N = 1000 };

    pri_queue_t *pq = pri_queue_create();
    PQ_CHECK(pq != NULL);

    int *data = malloc(N * sizeof(*data));
    PQ_CHECK(data != NULL);

    for (size_t i = 0; i < N; ++i) {
        data[i] = (int)i;
        PQ_CHECK(pri_queue_add(pq, (uint32_t)i,
                              (int)(i % 31), &data[i]) == 0);
    }

    PQ_CHECK(pri_queue_count(pq) == N);

    /* Remove a third of the items, causing heap rearrangements. */
    size_t removed = 0;

    for (size_t i = 0; i < N; i += 3) {
        PQ_CHECK(pri_queue_remove(pq, (uint32_t)i) == 0);
        ++removed;
    }

    PQ_CHECK(pri_queue_count(pq) == N - removed);

    int last_priority = INT_MAX;
    size_t popped = 0;

    while (pri_queue_count(pq) != 0) {
        pq_item_t out = {0};
        PQ_CHECK(pri_queue_pop(pq, &out) == 0);

        PQ_CHECK(out.priority <= last_priority);
        PQ_CHECK(out.id < N);
        PQ_CHECK(out.id % 3 != 0);
        PQ_CHECK(out.data == &data[out.id]);

        last_priority = out.priority;
        ++popped;
    }

    PQ_CHECK(popped == N - removed);

    pri_queue_destroy(pq);
    free(data);

    puts("PASS: pq growth and removals");
}

/* -------- 4. Randomized test with reference model -------- */

#define PQ_MODEL_N 128
#define PQ_RANDOM_OPS 10000

typedef struct {
    int active;
    int priority;
    uint64_t sequence;
} pq_model_item_t;

static uint32_t pq_rng_state = UINT32_C(0x12345678);

static uint32_t pq_random(void)
{
    uint32_t x = pq_rng_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return pq_rng_state = x;
}

/* Linear reference search: deliberately simple and independent. */
static int pq_model_best(const pq_model_item_t *model)
{
    int best = -1;

    for (int i = 0; i < PQ_MODEL_N; ++i) {
        if (!model[i].active)
            continue;

        if (best < 0 ||
            model[i].priority > model[best].priority ||
            (model[i].priority == model[best].priority &&
             model[i].sequence < model[best].sequence)) {
            best = i;
        }
    }

    return best;
}

static void test_pq_randomized(void)
{
    pri_queue_t *pq = pri_queue_create();
    PQ_CHECK(pq != NULL);

    pq_model_item_t model[PQ_MODEL_N] = {0};
    int data[PQ_MODEL_N];

    for (int i = 0; i < PQ_MODEL_N; ++i)
        data[i] = i * 17;

    pq_rng_state = UINT32_C(0x12345678);

    size_t count = 0;
    uint64_t sequence = 0;

    for (size_t step = 0; step < PQ_RANDOM_OPS; ++step) {
        uint32_t id = pq_random() % PQ_MODEL_N;
        int priority = (int)(pq_random() % 41) - 20;
        unsigned op = pq_random() % 5;
        int ret;

        switch (op) {
        case 0: /* ADD */
            ret = pri_queue_add(pq, id, priority, &data[id]);

            if (model[id].active) {
                PQ_CHECK(ret != 0);
            } else {
                PQ_CHECK(ret == 0);
                model[id].active = 1;
                model[id].priority = priority;
                model[id].sequence = sequence++;
                ++count;
            }
            break;

        case 1: /* SET */
            ret = pri_queue_set(pq, id, priority);

            if (model[id].active) {
                PQ_CHECK(ret == 0);
                model[id].priority = priority;
            } else {
                PQ_CHECK(ret != 0);
            }
            break;

        case 2: /* REMOVE */
            ret = pri_queue_remove(pq, id);

            if (model[id].active) {
                PQ_CHECK(ret == 0);
                model[id].active = 0;
                --count;
            } else {
                PQ_CHECK(ret != 0);
            }
            break;

        case 3: /* POP */ {
            int best = pq_model_best(model);
            pq_item_t out = {0};

            ret = pri_queue_pop(pq, &out);

            if (best < 0) {
                PQ_CHECK(ret != 0);
            } else {
                PQ_CHECK(ret == 0);
                PQ_CHECK(out.id == (uint32_t)best);
                PQ_CHECK(out.priority == model[best].priority);
                PQ_CHECK(out.data == &data[best]);

                model[best].active = 0;
                --count;
            }
            break;
        }

        case 4: /* PEEK */
            /* Checked below after every operation. */
            break;
        }

        /* Check count and top after EVERY operation. */
        PQ_CHECK(pri_queue_count(pq) == count);

        int best = pq_model_best(model);
        pq_item_t top = {0};

        ret = pri_queue_peek(pq, &top);

        if (best < 0) {
            PQ_CHECK(ret != 0);
        } else {
            PQ_CHECK(ret == 0);
            PQ_CHECK(top.id == (uint32_t)best);
            PQ_CHECK(top.priority == model[best].priority);
            PQ_CHECK(top.data == &data[best]);
        }
    }

    /* Drain everything in exact reference order. */
    while (count != 0) {
        int best = pq_model_best(model);
        pq_item_t out = {0};

        PQ_CHECK(best >= 0);
        PQ_CHECK(pri_queue_pop(pq, &out) == 0);
        PQ_CHECK(out.id == (uint32_t)best);
        PQ_CHECK(out.priority == model[best].priority);
        PQ_CHECK(out.data == &data[best]);

        model[best].active = 0;
        --count;
    }

    PQ_CHECK(pri_queue_count(pq) == 0);
    pri_queue_destroy(pq);

    puts("PASS: pq randomized (10000 operations)");
}

/* -------- 5. Destroy non-empty queue -------- */

static void test_pq_destroy_nonempty(void)
{
    enum { N = 200 };

    pri_queue_t *pq = pri_queue_create();
    PQ_CHECK(pq != NULL);

    int data[N];

    for (int i = 0; i < N; ++i) {
        data[i] = i;
        PQ_CHECK(pri_queue_add(pq, (uint32_t)i,
                              i % 13, &data[i]) == 0);
    }

    PQ_CHECK(pri_queue_count(pq) == N);

    /* ASan/LeakSanitizer should detect leaked internal objects. */
    pri_queue_destroy(pq);

    puts("PASS: pq destroy non-empty");
}

static void test_priority_queue_all(void)
{
    test_pq_basic();
    test_pq_fifo();
    test_pq_growth();
    test_pq_randomized();
    test_pq_destroy_nonempty();

    puts("\nPASS: ALL PRIORITY QUEUE TESTS");
}

int main(int argc, char **argv){
    (void) argc;
    (void) argv;
    
    test_priority_queue_all();

    return 0;
}