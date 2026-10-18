#include "hash_table.h"
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
typedef enum {
    SLOT_EMPTY = 0,
    SLOT_OCCUPIED,
    SLOT_DELETED
} slot_state_t;

typedef struct {
    uint32_t key;
    size_t value;
    slot_state_t state;
} hash_entry_t;
typedef struct hash_table{
    size_t count;
    size_t capacity;
    float load_factor;
    
    hash_entry_t* buckets;
}hash_table_t ;

static int rehash_table(hash_table_t* ht_old, hash_table_t* ht_new);

static int increase_table(hash_table_t* ht);

static inline uint32_t get_hash (hash_table_t* ht, uint32_t key)
{
    return (size_t)key % ht->capacity;
}

static inline int get_probe(uint32_t key)
{
    return key++;
}

static inline void _insert(hash_table_t* ht, uint32_t key, size_t value, size_t i)
{
    ht->buckets[i].value = value;
    ht->buckets[i].key = key;
    ht->buckets[i].state = SLOT_OCCUPIED;
    ht->count++;
    ht->load_factor = (float)ht->count/ht->capacity;
    if (ht->load_factor > MAX_LOAD_FACTOR)
        increase_table(ht);
}

static inline void _remove(hash_table_t* ht, size_t i)
{
    ht->buckets[i].value = 0;
    ht->buckets[i].key = 0;
    ht->buckets[i].state = SLOT_DELETED;
    ht->count--;
    ht->load_factor = (float)ht->count/ht->capacity;
}

static size_t find_key_or_deleted_idx(hash_table_t* ht, uint32_t start_idx, uint32_t key){
    
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

static size_t find_key_idx(hash_table_t* ht, uint32_t start_idx, uint32_t key){
    
    for (size_t i = start_idx; i < ht->capacity; i = get_probe(i)){
        if (ht->buckets[i].state == SLOT_EMPTY ||
            ht->buckets[i].key == key){
            return i;
        }
    }

    for (size_t i = 0; i < start_idx; i = get_probe(i)){
        if (ht->buckets[i].state == SLOT_EMPTY ||
            ht->buckets[i].key == key){
            return i;
        }
    }

    return start_idx;
}

static int rehash_table(hash_table_t* ht_old, hash_table_t* ht_new)
{
    for (uint32_t idx = 0; idx < ht_old->capacity; ++idx){
        int ret = 0;
        if (hash_table_insert(ht_new, ht_old->buckets[idx].key, ht_old->buckets[idx].value))
            return ret;
    }
    return 0;
}

static int increase_table(hash_table_t* ht)
{
    if (!ht)
        return -1;

    size_t new_capacity = ht->capacity*sizeof(hash_entry_t) > SIZE_MAX/2 ? 
                                                SIZE_MAX/sizeof(hash_entry_t) : 
                                                ht->capacity*2;

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
    // ht->capacity = ht_new.capacity;
    // ht->count = ht_new.count;
    // ht->load_factor = ht_new.load_factor;

    return 0;
}

hash_table_t* hash_table_create(void)
{
    hash_table_t *ht = (hash_table_t*)calloc(1, sizeof(hash_table_t));
    if (!ht)
        return NULL;

    ht->buckets = (hash_entry_t*)calloc(START_CAPACITY, sizeof(hash_entry_t));
    if (!ht->buckets)
        return NULL;

    ht->capacity = START_CAPACITY;
    
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

int hash_table_insert(hash_table_t* ht, uint32_t key, size_t value)
{
    if (!ht)
        return -1;

    uint32_t hash = get_hash(ht, key);
    size_t idx = find_key_or_deleted_idx(ht, hash, key);

    if (ht->buckets[idx].state == SLOT_DELETED){
        size_t dup_idx = find_key_idx(ht, idx + 1, key);
        if (ht->buckets[dup_idx].state == SLOT_EMPTY){
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
    } else {
        return -200; // Table is full
    }
}

int hash_table_get(hash_table_t* ht, uint32_t key, size_t *out)
{
    if (!ht || !out)
        return -1;

    uint32_t hash = get_hash(ht, key);
    size_t idx = find_key_idx(ht, hash, key);
    if (ht->buckets[idx].state == SLOT_OCCUPIED &&
        ht->buckets[idx].key == key) {
        *out = ht->buckets[idx].value;
        return 0; 
    } else {
        return -50; //Can't find
    }
}

int hash_table_update(hash_table_t* ht, uint32_t key, size_t new_val)
{
    if (!ht)
        return -1;

     uint32_t hash = get_hash(ht, key);
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

    uint32_t hash = get_hash(ht, key);
    size_t idx = find_key_idx(ht, hash, key);
    if (ht->buckets[idx].state == SLOT_OCCUPIED &&
        ht->buckets[idx].key == key) {
        _remove(ht, idx);
        return 0; 
    } else {
        return -50; //Can't find
    }
}

ssize_t hash_table_count(hash_table_t* ht)
{
    if (!ht)
        return -1;

    return ht->count;
}
