

#define _POSIX_C_SOURCE 200809L

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <time.h>

#include "hash_table.h"



typedef struct {
    size_t calls;
    uint64_t total_probes;
    size_t max_probes;
} ht_bench_stats_t;

static uint32_t rng_state = 0x12345678;

static uint32_t random_u32(void)
{
    uint32_t x = rng_state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    rng_state = x;
    return x;
}

static uint64_t now_ns(void)
{
    struct timespec ts;

    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        perror("clock_gettime");
        exit(EXIT_FAILURE);
    }

    return (uint64_t)ts.tv_sec * UINT64_C(1000000000)
         + (uint64_t)ts.tv_nsec;
}

static void ht_record_probes(ht_bench_stats_t *s, size_t probes)
{
    s->calls++;
    s->total_probes += probes;

    if (probes > s->max_probes)
        s->max_probes = probes;
}

static void ht_print_stats(const char *name,
                           const ht_bench_stats_t *s)
{
    double avg = s->calls
        ? (double)s->total_probes / (double)s->calls
        : 0.0;

    printf("  %-12s avg=%8.3f  max=%6zu\n",
           name, avg, s->max_probes);
}

static volatile uintptr_t bench_sink;

static void benchmark_ht_get(hash_table_t *ht,
                             const uint32_t *keys,
                             size_t n)
{
    const size_t target_operations = 10000000;
    const size_t runs = 7;

    size_t repeats = (target_operations + n - 1) / n;
    size_t operations = repeats * n;

    double results[7];

    for (size_t run = 0; run < runs + 1; ++run) {
        uintptr_t checksum = 0;

        uint64_t start = now_ns();

        for (size_t r = 0; r < repeats; ++r) {
            for (size_t i = 0; i < n; ++i) {
                void *out = NULL;
                hash_table_get(ht, keys[i], &out);
                checksum += (uintptr_t)out;
            }
        }

        uint64_t elapsed = now_ns() - start;

        bench_sink = checksum;

        // Первый запуск — прогревочный.
        if (run == 0)
            continue;

        results[run - 1] =
            (double)elapsed / (double)operations;
    }

    // Сортировка семи результатов по возрастанию.
    for (size_t i = 1; i < runs; ++i) {
        double value = results[i];
        size_t j = i;

        while (j > 0 && results[j - 1] > value) {
            results[j] = results[j - 1];
            --j;
        }

        results[j] = value;
    }

    double median = results[runs / 2];

    printf("  Median:     %.2f ns/op\n", median);
    printf("  Throughput: %.2f Mops/s\n", 1000.0 / median);
}

static void test_ht_probes(size_t n)
{
    hash_table_t *ht = hash_table_create_n(n*2);
    if (!ht) {
        fprintf(stderr, "Cannot create hash table\n");
        exit(EXIT_FAILURE);
    }

    uint32_t *keys = calloc(n, sizeof(uint32_t));
    if (!keys) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    for (size_t i = 0; i < n; i++){
        keys[i] = random_u32();
    }

    uint32_t *missing_keys = malloc(n * sizeof(*missing_keys));
    if (!missing_keys) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    for (size_t i = 0; i < n; i++){
        missing_keys[i] = random_u32();
    }

    static int value = 42;


    /* Fill the table. Not measured. */
    for (size_t i = 0; i < n; ++i) {
        uint32_t key = keys[i];//ht_bench_key((uint32_t)i);

        if (hash_table_insert(ht, key, &value) != 0) {
            fprintf(stderr, "Insert failed at %zu\n", i);
            exit(EXIT_FAILURE);
        }
    }

    size_t capacity = hash_table_debug_capacity(ht);

    printf("\nN=%zu capacity=%zu load=%.3f\n",
        n,
        capacity,
        (double)n / (double)capacity);

    benchmark_ht_get(ht, keys, n);


    ht_bench_stats_t successful = {0};
    ht_bench_stats_t unsuccessful = {0};

    /* Search every existing key. */
    for (size_t i = 0; i < n; ++i) {
        uint32_t key = keys[i];//ht_bench_key((uint32_t)i);
        size_t probes = 0;

        int ret = hash_table_debug_get_probes(
            ht, key, &probes);

        if (ret != 0) {
            fprintf(stderr, "Lookup failed at %zu\n", i);
            exit(EXIT_FAILURE);
        }

        ht_record_probes(&successful, probes);
    }

    /* Search an equal number of absent keys. */
    for (size_t i = 0; i < n; ++i) {
        uint32_t key = missing_keys[i];//ht_bench_key((uint32_t)i) | 1u;
        size_t probes = 0;

        int ret = hash_table_debug_get_probes(
            ht, key, &probes);

        if (ret != -50) {
            fprintf(stderr, "Unexpected lookup result\n");
            exit(EXIT_FAILURE);
        }

        ht_record_probes(&unsuccessful, probes);
    }

    // printf("\nN = %zu\n", n);
    ht_print_stats("Successful", &successful);
    ht_print_stats("Unsuccessful", &unsuccessful);

    hash_table_free(ht);
    free(keys);
    free(missing_keys);
}

static void test_ht_scaling(void)
{
    const size_t sizes[] = {
        1000,
        10000,
        100000,
        1000000,
        5000000
    };

    /* const size_t sizes[] = {
        16384,
        49152,
        81920,
        98304,
        114688
    }; */

    /* for (size_t i = 0;
         i < sizeof(sizes) / sizeof(sizes[0]);
         ++i) {
        test_ht_probes(sizes[i]);
    } */
    test_ht_probes(sizes[2]);
}


int main(void){
 

    test_ht_scaling();
    

    return 0;
}