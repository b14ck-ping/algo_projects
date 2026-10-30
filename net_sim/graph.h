#ifndef GRAPH_H
#define GRAPH_H

#include <stdint.h>

typedef struct graph_inst graph_inst_t;

typedef enum {
    GRAPH_STATUS_OK = 0,
    GRAPH_ERROR_BAD_ARG,
    GRAPG_ERROR_VERTEX_IS_NOT_EXIST,
    GRAPG_ERROR_VERTEX_WITHOUT_NEIGHBOURS,
    GRAPG_ERROR_MEMALLOC_ERROR,
    GRAPG_ERROR_CANT_CREATE_VERTEX,
    GRAPG_ERROR_LOOP,
    GRAPG_ERROR_FULL

} graph_status_t;


graph_inst_t* graph_create(void);
void graph_free(graph_inst_t* graph);

graph_status_t graph_add_vertex(graph_inst_t *graph, uint32_t* vertex_id);

graph_status_t graph_add_edge(graph_inst_t *graph, uint32_t vertex_a_id, uint32_t vertex_b_id); 

graph_status_t graph_get_neighbous(graph_inst_t *graph, uint32_t vertex_id, 
                                    const uint32_t **neighbours_buf, uint32_t* neghbours_cnt);

#endif //GRAPH_H