#include <stddef.h>
#include <stdbool.h>
#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "graph_algorithms.h"



struct queue {
    size_t size;
    size_t head;
    size_t tail;
    size_t cnt;
    uint32_t data[];
};

static struct queue* queue_create(size_t size)
{
    struct queue* buff = malloc(sizeof(struct queue) + sizeof(uint32_t)*size);
    if (!buff)
        return NULL;

    buff->head = 0;
    buff->tail = 0;
    buff->cnt = 0;
    buff->size = size;
    return buff;
}

static void queue_free(struct queue *queue)
{
    free(queue);
}

static bool queue_is_empty(struct queue *queue)
{
    return queue->cnt == 0;
}

static void queue_enqueue(struct queue *queue, uint32_t item)
{
    queue->data[queue->head] = item;
    size_t new_head = ++queue->head;
    queue->head = new_head % queue->size;
    queue->cnt++;
}

static uint32_t queue_dequeue(struct queue *queue)
{
    uint32_t ret = queue->data[queue->tail];
    size_t new_tail = ++queue->tail;
    queue->tail = new_tail % queue->size;
    queue->cnt--;
    return ret;
}

ga_status_t ga_find_shortest_way(graph_inst_t *gi, uint32_t from, uint32_t to,
                                    uint32_t **out_path, size_t *out_length)
{
    ga_status_t ret_val = GA_STATUS_OK;
    if (!gi || !out_path || !out_length)
        return GA_ERROR_BAD_ARG;

    *out_path = NULL;
    *out_length = 0;

    uint32_t vert_cnt = 0;
    if (graph_get_vertices_cnt(gi, &vert_cnt))
        return GA_ERROR_BAD_GRAPH;

    if(from >= vert_cnt || to >= vert_cnt)
        return GA_ERROR_VERTEX_IS_NOT_EXIST;

    if (from == to){
        uint32_t *out = calloc(1, sizeof(uint32_t));
        if (!out){
            return GA_ERROR_MEMALLOC;
        }
        out[0] = from;
        *out_path = out;
        *out_length = 1;
        return ret_val;
    }

    bool* visited = calloc(vert_cnt, sizeof(bool));
    if(!visited)
        return GA_ERROR_MEMALLOC;

    uint32_t* parent = calloc(vert_cnt, sizeof(uint32_t));
    if (!parent){
        free(visited);
        return GA_ERROR_MEMALLOC;
    }

    struct queue* queue = queue_create(vert_cnt);
    if (!queue){
        ret_val = GA_ERROR_MEMALLOC;
        goto err_exit;
    }
        
    queue_enqueue(queue, from);
    visited[from] = true;

    while (!queue_is_empty(queue)){
        uint32_t cur_vertex = queue_dequeue(queue);

        const uint32_t *neighbours = NULL;
        uint32_t neghbours_cnt = 0;
        if (graph_get_neighbours(gi, cur_vertex, 
                        &neighbours, &neghbours_cnt) != GRAPH_STATUS_OK){
            ret_val = GA_ERROR_BAD_GRAPH;
            goto err_exit;
        }
            

        for (uint32_t i = 0; i < neghbours_cnt; ++i){
            uint32_t neighbour = neighbours[i];
            if (visited[neighbour])
                continue;

            if (neighbour == to){
                visited[neighbour] = true;
                parent[neighbour] = cur_vertex;
                goto check_path;
            }

            parent[neighbour] = cur_vertex;
            visited[neighbour] = true;
            queue_enqueue(queue, neighbour);
        }
    }

check_path:
    if (!visited[to]){
        ret_val = GA_ERROR_CANT_FIND_PATH;
        goto err_exit;
    }

    uint32_t *out = calloc(vert_cnt, sizeof(uint32_t));
    if (!out){
        ret_val = GA_ERROR_MEMALLOC;
        goto err_exit;
    }

    uint32_t out_size = vert_cnt;
    uint32_t cur_vert = to;
    uint32_t idx = 0;
    while (cur_vert != from) {
        out[idx++] = cur_vert;
        cur_vert = parent[cur_vert];
        assert(visited[cur_vert] == true);
    }
    out[idx++] = cur_vert;


    if (idx < out_size){
        uint32_t *out_b = realloc(out, idx*sizeof(uint32_t));
        if (out_b){
            out = out_b;
            out_size = idx;
        }   
    }
    

    // reverse
    for (uint32_t i = 0; i < idx / 2; ++i){
        uint32_t tmp = out[i];
        out[i] = out[out_size - i - 1];
        out[out_size - i - 1] = tmp;
    }

    *out_path = out;
    *out_length = idx;
    
err_exit:    
    free(visited);
    free(parent);
    queue_free(queue);

    return ret_val;
}