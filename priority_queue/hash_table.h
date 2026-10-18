#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define START_CAPACITY 40
#define MAX_LOAD_FACTOR 0.7

typedef struct hash_table hash_table_t;

hash_table_t* hash_table_create(void);
void hash_table_free(hash_table_t* ht);

int hash_table_insert(hash_table_t* ht, uint32_t key, size_t value);
int hash_table_get(hash_table_t* ht, uint32_t key, size_t *out);
int hash_table_update(hash_table_t* ht, uint32_t key, size_t new_val);
int hash_table_remove(hash_table_t* ht, uint32_t key);
size_t hash_table_count(hash_table_t* ht);

#endif //HASH_TABLE_H