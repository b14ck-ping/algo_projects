#ifndef GRAPH_ALGORITHMS_H
#define GRAPH_ALGORITHMS_H

#include <stdint.h>
#include <stddef.h>

#include "graph.h"

typedef enum {
    GA_STATUS_OK = 0,
    GA_ERROR_BAD_ARG,
    GA_ERROR_VERTEX_IS_NOT_EXIST,
    GA_ERROR_BAD_GRAPH,
    GA_ERROR_MEMALLOC,
    GA_ERROR_CANT_FIND_PATH
} ga_status_t;


ga_status_t ga_find_shortest_way(graph_inst_t *gi,  uint32_t from, 
                                                    uint32_t to,
                                                    uint32_t **out_path, 
                                                    size_t *out_length);


ga_status_t ga_find_components(graph_inst_t *graph, uint32_t **out_components, 
                                                    uint32_t *out_vertices_count, 
                                                    uint32_t *out_components_count);


#endif // GRAPH_ALGORITHMS_H