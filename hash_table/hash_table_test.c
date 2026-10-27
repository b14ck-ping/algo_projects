
#define _POSIX_C_SOURCE 200809L

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

#include "hash_table.h"

typedef enum {
    KEYS_RANDOM,
    KEYS_SEQUENTIAL,
    KEYS_COLLIDING
} key_pattern_t;

typedef struct {
    size_t calls;
    uint64_t total_probes;
    size_t max_probes;
} ht_bench_stats_t;

static uint32_t rng_state = UINT32_C(0x12345678);
static volatile uintptr_t bench_sink;

/* ---------- Random generator ---------- */

static uint32_t random_u32(void)
{
    uint32_t x = rng_state;

    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;

    rng_state = x;
    return x;
}

/* ---------- Timer ---------- */

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

/* ---------- Probe statistics ---------- */

static void record_probes(ht_bench_stats_t *stats,
                          size_t probes)
{
    stats->calls++;
    stats->total_probes += probes;

    if (probes > stats->max_probes)
        stats->max_probes = probes;
}

static void print_probe_stats(const char *name,
                              const ht_bench_stats_t *stats)
{
    double average = stats->calls
        ? (double)stats->total_probes / stats->calls
        : 0.0;

    printf("  %-13s avg=%9.3f  max=%7zu\n",
           name, average, stats->max_probes);
}

/* ---------- Test data ---------- */

static const char *pattern_name(key_pattern_t pattern)
{
    switch (pattern) {
    case KEYS_RANDOM:     return "RANDOM";
    case KEYS_SEQUENTIAL: return "SEQUENTIAL";
    case KEYS_COLLIDING:  return "COLLIDING";
    }

    return "UNKNOWN";
}

static void generate_keys(uint32_t *keys,
                          size_t n,
                          size_t capacity,
                          key_pattern_t pattern,
                          size_t start)
{
    for (size_t i = 0; i < n; ++i) {
        size_t value = start + i;

        switch (pattern) {
        case KEYS_RANDOM:
            keys[i] = random_u32();
            break;

        case KEYS_SEQUENTIAL:
            keys[i] = (uint32_t)value;
            break;

        case KEYS_COLLIDING:
            keys[i] = (uint32_t)(value * capacity);
            break;
        }
    }
}

/* ---------- Timing benchmark ---------- */

static int compare_double(const void *a, const void *b)
{
    double x = *(const double *)a;
    double y = *(const double *)b;

    return (x > y) - (x < y);
}

static void benchmark_ht_get(hash_table_t *ht,
                             const uint32_t *keys,
                             size_t n,
                             size_t target_operations)
{
    enum { RUNS = 7 };

    size_t repeats =
        target_operations / n +
        (target_operations % n != 0);

    size_t operations = repeats * n;

    double results[RUNS];

    for (size_t run = 0; run <= RUNS; ++run) {
        uintptr_t checksum = 0;

        uint64_t start = now_ns();

        for (size_t r = 0; r < repeats; ++r) {
            for (size_t i = 0; i < n; ++i) {
                void *out = NULL;

                int ret = hash_table_get(ht, keys[i], &out);

                if (ret != 0 || out == NULL) {
                    fprintf(stderr, "Benchmark lookup failed\n");
                    exit(EXIT_FAILURE);
                }

                checksum += (uintptr_t)out;
            }
        }

        uint64_t elapsed = now_ns() - start;

        bench_sink = checksum;

        /* Run 0 is warm-up. */
        if (run == 0)
            continue;

        results[run - 1] =
            (double)elapsed / (double)operations;
    }

    qsort(results, RUNS, sizeof(results[0]), compare_double);

    double median = results[RUNS / 2];

    printf("  Median:      %.2f ns/op\n", median);
    printf("  Throughput:  %.2f Mops/s\n",
           1000.0 / median);
}

/* ---------- Experiment ---------- */

static void test_ht_probes(size_t requested_n,
                           key_pattern_t pattern)
{
     size_t capacity = requested_n * 2;


    if (requested_n == 0 ||
        requested_n > SIZE_MAX / 2 ||
        requested_n > SIZE_MAX / sizeof(uint32_t)) {
        fprintf(stderr, "Invalid N\n");
        exit(EXIT_FAILURE);
    }

    const size_t n =
        pattern == KEYS_COLLIDING
            ? 4096
            : requested_n;

    if (pattern == KEYS_COLLIDING &&
        (2 * n - 1) > UINT32_MAX / capacity) {
        fprintf(stderr, "Colliding keys overflow\n");
        exit(EXIT_FAILURE);
    }

    rng_state = UINT32_C(0x12345678);

    hash_table_t *ht = hash_table_create_n(capacity);
    if (!ht) {
        fprintf(stderr, "Cannot create hash table\n");
        exit(EXIT_FAILURE);
    }

    uint32_t *keys = malloc(n * sizeof(*keys));
    uint32_t *missing_keys = malloc(n * sizeof(*missing_keys));

    if (!keys || !missing_keys) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    generate_keys(keys, n, capacity, pattern, 0);
    generate_keys(missing_keys, n, capacity, pattern, n);

    static int value = 42;

    /* Fill the table without timing. */
    for (size_t i = 0; i < n; ++i) {
        int ret = hash_table_insert(ht, keys[i], &value);

        if (ret != 0) {
            fprintf(stderr,
                    "Insert failed at %zu, status=%d\n",
                    i, ret);
            exit(EXIT_FAILURE);
        }
    }

    capacity = hash_table_debug_capacity(ht);

    printf("\nPattern: %s\n", pattern_name(pattern));
    printf("N=%zu capacity=%zu load=%.3f\n",
           n, capacity, (double)n / capacity);

    /*
     * Worst-case collision scenario:
     * avoid tens of billions of bucket probes.
     */
    size_t target_operations =
        pattern == KEYS_COLLIDING ? 10000 : 10000000;

    printf("Timing %zu lookups per run (minimum)\n",
           target_operations);
    fflush(stdout);

    benchmark_ht_get(ht, keys, n, target_operations);

    ht_bench_stats_t successful = {0};
    ht_bench_stats_t unsuccessful = {0};

    for (size_t i = 0; i < n; ++i) {
        size_t probes = 0;

        int ret = hash_table_debug_get_probes(
            ht, keys[i], &probes);

        if (ret != 0) {
            fprintf(stderr,
                    "Successful lookup failed at %zu\n", i);
            exit(EXIT_FAILURE);
        }

        record_probes(&successful, probes);
    }

    for (size_t i = 0; i < n; ++i) {
        size_t probes = 0;

        int ret = hash_table_debug_get_probes(
            ht, missing_keys[i], &probes);

        if (ret != -50) {
            fprintf(stderr,
                    "Unexpected lookup result at %zu: %d\n",
                    i, ret);
            exit(EXIT_FAILURE);
        }

        record_probes(&unsuccessful, probes);
    }

    print_probe_stats("Successful", &successful);
    print_probe_stats("Unsuccessful", &unsuccessful);

    hash_table_free(ht);
    free(keys);
    free(missing_keys);
}

int main(void)
{
    /*
     * Fixed size and load factor for comparing
     * hash functions on identical conditions.
     */
    const size_t n = 16384;

    const key_pattern_t patterns[] = {
        KEYS_RANDOM,
        KEYS_SEQUENTIAL,
        KEYS_COLLIDING
    };

    for (size_t i = 0;
         i < sizeof(patterns) / sizeof(patterns[0]);
         ++i) {

        test_ht_probes(n, patterns[i]);
    }

    return EXIT_SUCCESS;
}