#include "prior_queue.h"
#include "addr_heap.h"
#include "hash_table.h"
#include <assert.h>


struct pri_queue {
    hash_table_t *ht;
    addr_heap_inst_t *addr_heap;
};

typedef struct {
    uint32_t id;
    void *data;
} pq_entry_t;

pri_queue_t* pri_queue_create(void)
{
    pri_queue_t* pq = malloc(sizeof(pri_queue_t));
    if (!pq)
        return NULL;

    pq->addr_heap = addr_heap_allocate();
    if(!pq->addr_heap){
        free(pq);
        return NULL;
    }

    pq->ht = hash_table_create();
    if(!pq->ht){
        free(pq->addr_heap);
        free(pq);
        return NULL;
    }

    return pq;
}

void pri_queue_destroy(pri_queue_t* pri_q)
{
    if(!pri_q)
        return;

    pq_item_t* item_out = {0};
    while(pri_queue_pop(pri_q, item_out) == 0);

    hash_table_free(pri_q->ht);
    addr_heap_deallocate(pri_q->addr_heap);
    free(pri_q);
}

int pri_queue_add(pri_queue_t* pq, uint32_t id, int priority, void* data)
{
    if (!pq || !data)
        return -1;

    void *out = NULL;
    int ret = hash_table_get(pq->ht, id, &out);
    if (!ret)
        return -200;// duplicate

    addr_heap_handle_t *handle = NULL;
    pq_entry_t* entry = malloc(sizeof(*entry));
    if(!entry)
        return -3;

    entry->id = id;
    entry->data = data;
    ret = addr_heap_add_item(pq->addr_heap, entry, priority, &handle);
    if (ret){
        free(entry);
        return ret;
    }
        
    ret = hash_table_insert(pq->ht, id, (void*)handle);
    if (ret){
        addr_heap_item_t out = {0};
        addr_heap_remove_item(pq->addr_heap, handle, &out);
        addr_heap_free_handler( handle);
        free(entry);
        return ret;
    }

    return 0;
}

int pri_queue_set(pri_queue_t* pq, uint32_t id, int priority)
{
    if (!pq)
        return -1;

    void *out = NULL;
    int ret = hash_table_get(pq->ht, id, &out);
    if (ret || !out)
        return -50;// can't find

    return addr_heap_set_priority(pq->addr_heap, priority, (addr_heap_handle_t*)out);

}

int pri_queue_remove(pri_queue_t *pq, uint32_t id)
{
    if (!pq)
        return -1;

    void *handle = NULL;
    int ret = hash_table_get(pq->ht, id, &handle);
    if (ret || !handle)
        return -50;// can't find

    addr_heap_item_t out_item = {0};
    assert(addr_heap_remove_item(pq->addr_heap, (addr_heap_handle_t*)handle, &out_item) == 0);
    assert(hash_table_remove(pq->ht, id) == 0);

    addr_heap_free_handler(handle);
    free(out_item.data);

    
    return 0;
}

int pri_queue_peek(pri_queue_t* pq, pq_item_t* item_out)
{
    if (!pq || !item_out)
        return -1;

    pq_item_t _item_out = {0};
    addr_heap_item_t out = {0};
    int ret = addr_heap_peek_item(pq->addr_heap, &out);
    if(ret){
        return -50;
    }

    _item_out.data = ((pq_entry_t*)out.data)->data;
    _item_out.id = ((pq_entry_t*)out.data)->id;
    _item_out.priority = out.prior;

    *item_out = _item_out;

    return 0;
}

int pri_queue_pop(pri_queue_t* pq, pq_item_t* item_out)
{
    if (!pq || !item_out)
        return -1;

    pq_item_t _item_out = {0};
    addr_heap_item_t out = {0};
    int ret = addr_heap_pop(pq->addr_heap, &out);
    if(ret == -2){
        return -2; //empty queue
    }

    assert(hash_table_remove(pq->ht, ((pq_entry_t*)out.data)->id) == 0);
    assert(addr_heap_free_handler(out.handle) == 0);

    _item_out.data = ((pq_entry_t*)out.data)->data;
    _item_out.id = ((pq_entry_t*)out.data)->id;
    _item_out.priority = out.prior;

    *item_out = _item_out;
    free(out.data);

    return 0;
}

size_t pri_queue_count(pri_queue_t* pq)
{
    if (!pq) return 0;
    return hash_table_count(pq->ht);
}
