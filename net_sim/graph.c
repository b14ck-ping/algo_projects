#include <stdio.h>
#include <stdlib.h>
#include "graph.h" 

typedef struct {
    uint32_t id;
    uint32_t neighbours_cnt;
    uint32_t neighbours_cap;

    uint32_t* neighbours;
} vertex_t;

struct graph_inst {
    uint32_t vert_cnt;
    uint32_t vert_capacity;


    vertex_t* vertices;
};


graph_inst_t* graph_create(void){

    return NULL;
}

void graph_free(graph_inst_t* graph)
{

    
}

graph_status_t graph_add_vertex(graph_inst_t *graph, uint32_t *vertex_id)

{

    return GRAPH_STATUS_OK;
}

graph_status_t graph_add_edge(graph_inst_t *graph, uint32_t vertex_a_id, uint32_t vertex_b_id)
{

    return GRAPH_STATUS_OK;
} 

graph_status_t graph_get_neighbous(graph_inst_t *graph, uint32_t vertex_id, const uint32_t **neighbours_buf)
{

    return GRAPH_STATUS_OK;
}