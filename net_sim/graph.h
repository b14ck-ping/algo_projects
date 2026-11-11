#ifndef GRAPH_H
#define GRAPH_H

#include <stdint.h>

typedef enum {
    GRAPH_UNDIRECTED,
    GRAPH_DIRECTED
} graph_type_t;

typedef struct {
    uint32_t vertex_id;
    uint32_t weight;
} graph_edge_t;

typedef struct graph_inst graph_inst_t;

typedef enum {
    GRAPH_STATUS_OK = 0,
    GRAPH_ERROR_BAD_ARG,
    GRAPH_ERROR_VERTEX_IS_NOT_EXIST,
    GRAPH_ERROR_VERTEX_WITHOUT_NEIGHBOURS,
    GRAPH_ERROR_MEMALLOC_ERROR,
    GRAPH_ERROR_CANT_CREATE_VERTEX,
    GRAPH_ERROR_LOOP,
    GRAPH_ERROR_FULL

} graph_status_t;


graph_inst_t* graph_create(graph_type_t type);
void graph_free(graph_inst_t* graph);

graph_status_t graph_get_type(graph_inst_t* graph, graph_type_t *type);

graph_status_t graph_add_vertex(graph_inst_t *graph, uint32_t* vertex_id);

graph_status_t graph_add_edge(graph_inst_t *graph, uint32_t vertex_a_id, uint32_t vertex_b_id); 

graph_status_t graph_add_weighted_edge(graph_inst_t *graph, uint32_t vertex_a_id, uint32_t vertex_b_id, uint32_t weight);

graph_status_t graph_get_vertices_cnt(graph_inst_t *graph, uint32_t* vertices_cnt);

graph_status_t graph_get_neighbours(graph_inst_t *graph, uint32_t vertex_id, 
                                    const graph_edge_t **neighbours_buf, uint32_t* neghbours_cnt);

#endif //GRAPH_H