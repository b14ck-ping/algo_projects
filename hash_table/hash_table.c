#include "hash_table.h"
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
typedef enum {
    SLOT_EMPTY = 0,
    SLOT_OCCUPIED,
    SLOT_DELETED
} slot_state_t;

typedef struct {
    uint32_t key;
    void* value;
    slot_state_t state;
} hash_entry_t;
typedef struct hash_table{
    size_t count;
    size_t capacity;
    double load_factor;
    
    hash_entry_t* buckets;
}hash_table_t ;

typedef struct {
    size_t calls;
    uint64_t total_probes;
    size_t max_probes;
} ht_bench_stats_t;

static void ht_record_probes(ht_bench_stats_t *stats,
                             size_t probes)
{
    stats->calls++;
    stats->total_probes += probes;

    if (probes > stats->max_probes)
        stats->max_probes = probes;
}

static int rehash_table(hash_table_t* ht_old, hash_table_t* ht_new);

static int increase_table(hash_table_t* ht);

static inline uint32_t hash_identity(uint32_t key)
{
    return key;
}

static inline uint32_t hash_multiplicative(uint32_t key)
{
    return key * UINT32_C(2654435761);
}

static inline uint32_t hash_murmur3(uint32_t key)
{
    key ^= key >> 16;
    key *= UINT32_C(0x85ebca6b);
    key ^= key >> 13;
    key *= UINT32_C(0xc2b2ae35);
    key ^= key >> 16;

    return key;
}

static inline size_t get_hash (hash_table_t* ht, uint32_t key)
{
        uint32_t hash;

#if HT_HASH_MODE == 0
    hash = hash_identity(key);
#elif HT_HASH_MODE == 1
    hash = hash_multiplicative(key);
#elif HT_HASH_MODE == 2
    hash = hash_murmur3(key);
#else
#error Unsupported hash mode
#endif

    return (size_t)hash % ht->capacity;
}

static inline size_t get_probe(size_t key)
{
    return ++key;
}

static inline void _insert(hash_table_t* ht, uint32_t key, void* value, size_t i)
{
    ht->buckets[i].value = value;
    ht->buckets[i].key = key;
    ht->buckets[i].state = SLOT_OCCUPIED;
    ht->count++;
    ht->load_factor = (double)ht->count/(double)ht->capacity;
}

static inline void _remove(hash_table_t* ht, size_t i)
{
    ht->buckets[i].value = 0;
    ht->buckets[i].key = 0;
    size_t next = (i + 1) % ht->capacity;
    ht->buckets[i].state = ht->buckets[next].state == SLOT_EMPTY ? SLOT_EMPTY : SLOT_DELETED;
    ht->count--;
    ht->load_factor = (double)ht->count/(double)ht->capacity;
}

static size_t find_key_or_deleted_idx(hash_table_t* ht, size_t start_idx, uint32_t key){
    
    for (size_t i = start_idx; i < ht->capacity; i = get_probe(i)){
        if (ht->buckets[i].state == SLOT_EMPTY ||
            ht->buckets[i].state == SLOT_DELETED ||
            ht->buckets[i].key == key){
            return i;
        }
    }

    for (size_t i = 0; i < start_idx; i = get_probe(i)){
        if (ht->buckets[i].state == SLOT_EMPTY ||
            ht->buckets[i].state == SLOT_DELETED ||
            ht->buckets[i].key == key){
            return i;
        }
    }

    return start_idx;
}

static size_t find_key_idx(hash_table_t* ht, size_t start_idx, uint32_t key){
    
    for (size_t i = start_idx; i < ht->capacity; i = get_probe(i)){
        if (ht->buckets[i].state == SLOT_EMPTY ||
            (ht->buckets[i].state == SLOT_OCCUPIED &&
            ht->buckets[i].key == key)){
            return i;
        }
    }

    for (size_t i = 0; i < start_idx; i = get_probe(i)){
        if (ht->buckets[i].state == SLOT_EMPTY ||
            (ht->buckets[i].state == SLOT_OCCUPIED &&
            ht->buckets[i].key == key)){
            return i;
        }
    }

    return start_idx;
}

static int rehash_table(hash_table_t* ht_old, hash_table_t* ht_new)
{
    for (uint32_t idx = 0; idx < ht_old->capacity; ++idx){
        int ret = 0;
        if (ht_old->buckets[idx].state == SLOT_OCCUPIED && 
            (ret = hash_table_insert(ht_new, ht_old->buckets[idx].key, ht_old->buckets[idx].value)) != 0)
            return ret;
    }
    return 0;
}

static int increase_table(hash_table_t* ht)
{
    if (!ht)
        return -1;

    if (ht->capacity > (size_t)PTRDIFF_MAX /
                       sizeof(*ht->buckets)/2)
        return -3;

    size_t new_capacity = ht->capacity*2;

    hash_table_t ht_new = {0};
    ht_new.capacity = new_capacity;
    ht_new.buckets = (hash_entry_t*)calloc(new_capacity, sizeof(hash_entry_t));
    if (!ht_new.buckets){
        return -3;
    }

    int ret = rehash_table(ht, &ht_new);
    if (ret){
        free(ht_new.buckets);
        return ret;
    }

    free(ht->buckets);
    *ht = ht_new;

    return 0;
}

hash_table_t* hash_table_create(void)
{
    hash_table_t *ht = (hash_table_t*)calloc(1, sizeof(hash_table_t));
    if (!ht)
        return NULL;

    ht->buckets = (hash_entry_t*)calloc(START_CAPACITY, sizeof(hash_entry_t));
    if (!ht->buckets){
        free(ht);
        return NULL;
    }
       
    ht->capacity = START_CAPACITY;
    
    return ht;
}

hash_table_t* hash_table_create_n(size_t N)
{
    hash_table_t *ht = (hash_table_t*)calloc(1, sizeof(hash_table_t));
    if (!ht)
        return NULL;

    ht->buckets = (hash_entry_t*)calloc(N, sizeof(hash_entry_t));
    if (!ht->buckets){
        free(ht);
        return NULL;
    }
       
    ht->capacity = N;
    
    return ht;
}

void hash_table_free(hash_table_t* ht)
{
    if (!ht)
        return;

    if (ht->buckets)
        free(ht->buckets);

    free(ht);
}

int hash_table_insert(hash_table_t* ht, uint32_t key, void* value)
{
    if (!ht)
        return -1;

    
    void* out = 0;
    if (hash_table_get(ht, key, &out) && 
        ((double)(ht->count + 1)/(double)ht->capacity) > MAX_LOAD_FACTOR){
        // printf("Increase table \n");
        int ret = increase_table(ht);
        if (ret){
            printf("Increasing of table returns non-zero status %d\n", ret);
            return ret;
        }
            
    }

    size_t hash = get_hash(ht, key);
    size_t idx = find_key_or_deleted_idx(ht, hash, key);

    if (ht->buckets[idx].state == SLOT_DELETED){
        size_t dup_idx = find_key_idx(ht, idx, key);
        if (ht->buckets[dup_idx].state == SLOT_EMPTY || dup_idx == idx){
            _insert(ht, key, value, idx);
            return 0;
        } else if (ht->buckets[dup_idx].state == SLOT_OCCUPIED &&
                            ht->buckets[dup_idx].key == key) {
            return -100; // duplicate
        } else {
            return -200; // Table is full
        }
    } else if (ht->buckets[idx].state == SLOT_EMPTY) {
        _insert(ht, key, value, idx);
        return 0;
    } else if (ht->buckets[idx].state == SLOT_OCCUPIED &&
                ht->buckets[idx].key == key){
        return -100; // duplicate
    } else {
        return -200; // Table is full
    }
}

int hash_table_get(hash_table_t* ht, uint32_t key, void** out)
{
    if (!ht || !out)
        return -1;

    size_t hash = get_hash(ht, key);
    size_t idx = find_key_idx(ht, hash, key);
    if (ht->buckets[idx].state == SLOT_OCCUPIED &&
        ht->buckets[idx].key == key) {
        *out = ht->buckets[idx].value;
        return 0; 
    } else {
        return -50; //Can't find
    }
}

int hash_table_update(hash_table_t* ht, uint32_t key, void* new_val)
{
    if (!ht)
        return -1;

    size_t hash = get_hash(ht, key);
    size_t idx = find_key_idx(ht, hash, key);
    if (ht->buckets[idx].state == SLOT_OCCUPIED &&
        ht->buckets[idx].key == key) {
        ht->buckets[idx].value = new_val;
        return 0; 
    } else {
        return -50; //Can't find
    }
}

int hash_table_remove(hash_table_t* ht, uint32_t key)
{
    if (!ht)
        return -1;

    size_t hash = get_hash(ht, key);
    size_t idx = find_key_idx(ht, hash, key);
    if (ht->buckets[idx].state == SLOT_OCCUPIED &&
        ht->buckets[idx].key == key) {
        _remove(ht, idx);
        return 0; 
    } else {
        return -50; //Can't find
    }
}

size_t hash_table_count(hash_table_t* ht)
{
    if (ht) 
        return ht->count;

    return 0;
}

int hash_table_debug_get_probes(hash_table_t *ht,
                                uint32_t key,
                                size_t *probes)
{
    if (!ht || !probes)
        return -1;

    *probes = 0;

    size_t idx = get_hash(ht, key);

    for (size_t i = 0; i < ht->capacity; ++i) {
        ++*probes;

        hash_entry_t *entry = &ht->buckets[idx];

        if (entry->state == SLOT_EMPTY)
            return -50;

        if (entry->state == SLOT_OCCUPIED &&
            entry->key == key)
            return 0;

        idx = (idx + 1) % ht->capacity;
    }

    return -50;
}

size_t hash_table_debug_capacity(hash_table_t *ht)
{
    return ht ? ht->capacity : 0;
}