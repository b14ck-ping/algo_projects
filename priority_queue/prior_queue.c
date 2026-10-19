#include "prior_queue.h"
#include "heap.h"
#include "hash_table.h"


typedef struct pri_queue {
    size_t count;

};




pri_queue_t* pri_queue_create(void)
{

}

void pri_queue_destroy(pri_queue_t* pri_q)
{

}

int pri_queue_add(pri_queue_t* pq, uint32_t id, int priority, const pq_item_t* item)
{

}

int pri_queue_set(pri_queue_t* pq, uint32_t id, int priority)
{

}

int pri_queue_remove(pri_queue_t *pq, uint32_t id)
{

}

int pri_queue_peek(pri_queue_t* pq, pq_item_t* item_out)
{

}

int pri_queue_pop(pri_queue_t* pq, pq_item_t* item_out)
{

}

size_t pri_queue_count(pri_queue_t* pq)
{

}
