#include <stdint.h>
#include <memory.h>
#include <stdlib.h>
#include <stdbool.h>
#include "graph.h" 

#define START_VERTICES_CNT 16
#define START_EDGES_CBT 32
typedef struct {
    uint32_t neighbours_cnt;
    uint32_t neighbours_cap;

    uint32_t* neighbours;
} vertex_t;

struct graph_inst {
    uint32_t vert_cnt;
    uint32_t vert_capacity;

    vertex_t* vertices;
};

static inline int _check_vertex_neighbour_slots(graph_inst_t *graph, uint32_t vertex_id){

    if (!graph->vertices[vertex_id].neighbours){
        graph->vertices[vertex_id].neighbours = calloc(START_EDGES_CBT, sizeof(uint32_t));
        if (!graph->vertices[vertex_id].neighbours)
            return -3;
        graph->vertices[vertex_id].neighbours_cap = START_EDGES_CBT;
        return 0;
    }

    if (graph->vertices[vertex_id].neighbours_cnt == graph->vertices[vertex_id].neighbours_cap){
        if (graph->vertices[vertex_id].neighbours_cap == UINT32_MAX) return GRAPH_ERROR_FULL;
        uint32_t size = graph->vertices[vertex_id].neighbours_cap < UINT32_MAX / 2 ? graph->vertices[vertex_id].neighbours_cap*2 : UINT32_MAX;
        uint32_t *buff = realloc(graph->vertices[vertex_id].neighbours, size*sizeof(uint32_t));
        if (!buff)
            return -3;
        graph->vertices[vertex_id].neighbours_cap = size;
        graph->vertices[vertex_id].neighbours = buff;
    }

    return 0;
}

static inline bool _find_neighbour(const vertex_t *vertex, uint32_t neighbour_id){
    for (uint32_t i = 0; i < vertex->neighbours_cnt; ++i){
        if (vertex->neighbours[i] == neighbour_id)
            return true;
    }
    return false;
}

static inline bool _check_vertices_has_edge(graph_inst_t *graph, uint32_t vertex_a_id, uint32_t vertex_b_id){
    if (!graph->vertices[vertex_a_id].neighbours_cnt ||
        !graph->vertices[vertex_b_id].neighbours_cnt)
            return false;

    if (_find_neighbour(&graph->vertices[vertex_a_id], vertex_b_id))
        return true;

    return false;
}

graph_inst_t* graph_create(void)
{
    graph_inst_t* instance = (graph_inst_t*)malloc(sizeof(graph_inst_t));
    if (!instance)
        return NULL;

    instance->vertices = calloc(START_VERTICES_CNT, sizeof(*instance->vertices));
    if (!instance->vertices){
        free(instance);
        return NULL;
    }
    instance->vert_capacity = START_VERTICES_CNT;
    instance->vert_cnt = 0;

    return instance;
}

void graph_free(graph_inst_t* graph)
{
    if (!graph)
        return;

    for (uint32_t i = 0; i < graph->vert_cnt; ++i){
        if (graph->vertices[i].neighbours)
            free(graph->vertices[i].neighbours);
    }
    free(graph->vertices);
    free(graph);
    
}

graph_status_t graph_add_vertex(graph_inst_t *graph, uint32_t *vertex_id)
{
    if (!graph || !vertex_id)
        return GRAPH_ERROR_BAD_ARG;

    if (graph->vert_capacity == graph->vert_cnt){
        if(graph->vert_capacity == UINT32_MAX)return GRAPH_ERROR_FULL; 
        uint32_t size = graph->vert_capacity < UINT32_MAX / 2 ? graph->vert_capacity * 2 : UINT32_MAX;
        vertex_t *buff = realloc(graph->vertices, size*sizeof(vertex_t));
        if (!buff)
            return GRAPH_ERROR_MEMALLOC_ERROR;
        graph->vert_capacity = size;
        graph->vertices = buff;
    }

    graph->vertices[graph->vert_cnt].neighbours = NULL;
    graph->vertices[graph->vert_cnt].neighbours_cap = 0;
    graph->vertices[graph->vert_cnt].neighbours_cnt = 0;
    *vertex_id = graph->vert_cnt++;
    
    
    return GRAPH_STATUS_OK;
}

graph_status_t graph_add_edge(graph_inst_t *graph, uint32_t vertex_a_id, uint32_t vertex_b_id)
{
    if (!graph)
        return GRAPH_ERROR_BAD_ARG;

    if (vertex_a_id == vertex_b_id)
        return GRAPH_ERROR_LOOP;

    if (vertex_a_id >= graph->vert_cnt 
        || vertex_b_id >= graph->vert_cnt)
        return GRAPH_ERROR_VERTEX_IS_NOT_EXIST;

    if(_check_vertices_has_edge(graph, vertex_a_id, vertex_b_id))
        return GRAPH_STATUS_OK;

    if(_check_vertex_neighbour_slots(graph, vertex_a_id)
        || _check_vertex_neighbour_slots(graph, vertex_b_id))
        return GRAPH_ERROR_MEMALLOC_ERROR;
    

    graph->vertices[vertex_a_id].neighbours[graph->vertices[vertex_a_id].neighbours_cnt++] = vertex_b_id;
    graph->vertices[vertex_b_id].neighbours[graph->vertices[vertex_b_id].neighbours_cnt++] = vertex_a_id;


    return GRAPH_STATUS_OK;
} 

graph_status_t graph_get_neighbous(graph_inst_t *graph, uint32_t vertex_id, 
                                    const uint32_t **neighbours_buf, uint32_t* neghbours_cnt)
{
    if (!graph || !neighbours_buf || !neghbours_cnt)
        return GRAPH_ERROR_BAD_ARG;

    if (vertex_id >= graph->vert_cnt)
        return GRAPH_ERROR_VERTEX_IS_NOT_EXIST;

    if (!graph->vertices[vertex_id].neighbours || graph->vertices[vertex_id].neighbours_cnt == 0){
        *neighbours_buf = NULL;
        *neghbours_cnt = 0;
        return GRAPH_STATUS_OK;
    }

    *neighbours_buf = graph->vertices[vertex_id].neighbours;
    *neghbours_cnt = graph->vertices[vertex_id].neighbours_cnt;

    return GRAPH_STATUS_OK;
}