#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdlib.h>
#include <stdint.h>

#define START_CAPACITY 32768
#define MAX_LOAD_FACTOR 0.5

#ifndef HT_HASH_MODE
#define HT_HASH_MODE 2
#endif

#ifndef HT_PROBE_MODE
#define HT_PROBE_MODE 0
#endif

typedef struct hash_table hash_table_t;

hash_table_t* hash_table_create(void);
hash_table_t* hash_table_create_n(size_t N);
void hash_table_free(hash_table_t* ht);

int hash_table_insert(hash_table_t* ht, uint32_t key, void* value);
int hash_table_get(hash_table_t* ht, uint32_t key, void** out);
int hash_table_update(hash_table_t* ht, uint32_t key, void* new_val);
int hash_table_remove(hash_table_t* ht, uint32_t key);
size_t hash_table_count(hash_table_t* ht);
int hash_table_debug_get_probes(hash_table_t *ht,
                                uint32_t key,
                                size_t *probes);
size_t hash_table_debug_capacity(hash_table_t *ht);

#endif //HASH_TABLE_H