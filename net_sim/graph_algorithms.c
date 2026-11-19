#include <stdbool.h>
#include <stddef.h>
#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "graph_algorithms.h"
#include "graph.h"
#include "addr_heap.h"


#define COMPONENT_UNVISITED UINT32_MAX
#define WITHOUT_PARENT UINT32_MAX

struct queue {
    size_t size;
    size_t head;
    size_t tail;
    size_t cnt;
    uint32_t data[];
};

static ga_status_t ga_has_cycle_directed(
    graph_inst_t *graph,
    bool *out_has_cycle
);

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

        const graph_edge_t *neighbours = NULL;
        uint32_t neghbours_cnt = 0;
        if (graph_get_neighbours(gi, cur_vertex, 
                        &neighbours, &neghbours_cnt) != GRAPH_STATUS_OK){
            ret_val = GA_ERROR_BAD_GRAPH;
            goto err_exit;
        }
            

        for (uint32_t i = 0; i < neghbours_cnt; ++i){
            uint32_t neighbour = neighbours[i].vertex_id;
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
        out[i] = out[idx - i - 1];
        out[idx - i - 1] = tmp;
    }

    *out_path = out;
    *out_length = idx;
    
err_exit:    
    free(visited);
    free(parent);
    queue_free(queue);

    return ret_val;
}

struct stack {
    uint32_t stack_capacity;
    uint32_t stack_hight;
    void* stack[];
};

static struct stack *stack_create(uint32_t cap)
{
    struct stack *stack = malloc(sizeof(struct stack) + cap*sizeof(stack->stack[0]));
    if (!stack)
        return NULL;

    stack->stack_capacity = cap;
    stack->stack_hight = 0;
    return stack;
}

static void stack_free(struct stack *stack)
{
    free(stack);
}

static void stack_push(struct stack *stack, void* item)
{
    stack->stack[stack->stack_hight++] = item;
}

static void* stack_pop(struct stack *stack)
{
    return stack->stack[--stack->stack_hight];
}

static void* stack_peek(struct stack *stack)
{
    return stack->stack[stack->stack_hight-1];
}

static bool stack_is_empty(struct stack *stack)
{
    return stack->stack_hight == 0;
}


ga_status_t ga_find_components(graph_inst_t *graph, uint32_t **out_components, uint32_t *out_vertices_count, uint32_t *out_components_count)
{
    ga_status_t ret_val = GA_STATUS_OK;
    if (!graph || !out_components || !out_vertices_count || !out_components_count)
        return GA_ERROR_BAD_ARG;

    *out_components = NULL;
    *out_vertices_count = 0;
    *out_components_count = 0;

    uint32_t vertices_cnt = 0;
    if(graph_get_vertices_cnt(graph, &vertices_cnt))
        return GA_ERROR_BAD_GRAPH;

    if(!vertices_cnt)
        return GA_STATUS_OK;

    if(vertices_cnt == 1){
        uint32_t *out = malloc(sizeof(uint32_t));
        if (!out)
            return GA_ERROR_MEMALLOC;
        out[0] = 0;
        *out_components = out;
        *out_vertices_count = 1;
        *out_components_count = 1;
        return GA_STATUS_OK;
    }

    uint32_t *components = calloc(vertices_cnt, sizeof(uint32_t));
    if(!components)
        return GA_ERROR_MEMALLOC;

    uint32_t *vtx_ids = calloc(vertices_cnt, sizeof(*vtx_ids));
    if(!vtx_ids){
        free(components);
        return GA_ERROR_MEMALLOC;
    }

    for(uint32_t i = 0; i < vertices_cnt; i++)
        vtx_ids[i] = i;

    struct stack *stk = stack_create(vertices_cnt);
    if(!stk){
        free(vtx_ids);
        free(components);
        return GA_ERROR_MEMALLOC;
    }

    memset(components, 0xFF, vertices_cnt*sizeof(*components));    

    uint32_t component_idx = 0;
    for(uint32_t i = 0; i < vertices_cnt; i++){
        if (components[i] != COMPONENT_UNVISITED)
            continue;

        stack_push(stk, &vtx_ids[i]);
        components[i] = component_idx;

        while(!stack_is_empty(stk)){
            uint32_t cur_vertex = *(uint32_t*)stack_pop(stk);

            const graph_edge_t *neighbours = NULL;
            uint32_t neghbours_cnt = 0;
            if (graph_get_neighbours(graph, cur_vertex, 
                            &neighbours, &neghbours_cnt) != GRAPH_STATUS_OK){
                free(components);
                free(vtx_ids);
                stack_free(stk);
                return GA_ERROR_BAD_GRAPH;
            }
                

            for (uint32_t j = 0; j < neghbours_cnt; ++j){
                uint32_t neighbour = neighbours[j].vertex_id;
                if (components[neighbour] != COMPONENT_UNVISITED)
                    continue;

                components[neighbour] = component_idx;
                stack_push(stk, &vtx_ids[neighbour]);
            }
        }
        component_idx++;
    }

    assert(component_idx <= vertices_cnt);
    *out_components = components;
    *out_vertices_count = vertices_cnt;
    *out_components_count = component_idx;

    stack_free(stk);
    free(vtx_ids);

    return ret_val;
}

ga_status_t ga_has_cycle(graph_inst_t *graph, bool *out_has_cycle)
{
    ga_status_t ret_val = GA_STATUS_OK;
    bool out = false;

    if (!graph || !out_has_cycle )
        return GA_ERROR_BAD_ARG;

    *out_has_cycle = false;

    graph_type_t type;
    graph_get_type(graph, &type);

    if (type != GRAPH_UNDIRECTED)
        return ga_has_cycle_directed(graph, out_has_cycle);

    uint32_t vertices_cnt = 0;
    if(graph_get_vertices_cnt(graph, &vertices_cnt))
        return GA_ERROR_BAD_GRAPH;

    if(vertices_cnt < 3){
        return GA_STATUS_OK;
    }

    uint32_t *vtx_ids = calloc(vertices_cnt, sizeof(*vtx_ids));
    if(!vtx_ids){
        return GA_ERROR_MEMALLOC;
    }

    for(uint32_t i = 0; i < vertices_cnt; i++)
        vtx_ids[i] = i;

    bool *visited = calloc(vertices_cnt, sizeof(*visited));
    if(!visited){
        free(vtx_ids);
        return GA_ERROR_MEMALLOC;
    }
        

    uint32_t *parents = calloc(vertices_cnt, sizeof(*parents));
    if(!parents){
        free(visited);
        free(vtx_ids);
        return GA_ERROR_MEMALLOC;
    }
        
    memset(parents, 0xFF, vertices_cnt*sizeof(*parents));

    struct stack *stk = stack_create(vertices_cnt);
    if(!stk){
        free(visited);
        free(parents);
        free(vtx_ids);
        return GA_ERROR_MEMALLOC;
    }

    for(uint32_t i = 0; i < vertices_cnt; i++){
        if (visited[i])
            continue;

        stack_push(stk, &vtx_ids[i]);
        visited[i] = true;

        while(!stack_is_empty(stk)){
            uint32_t cur_vertex = *(uint32_t*)stack_pop(stk);

            const graph_edge_t *neighbours = NULL;
            uint32_t neghbours_cnt = 0;
            if (graph_get_neighbours(graph, cur_vertex, 
                            &neighbours, &neghbours_cnt) != GRAPH_STATUS_OK){
                ret_val = GA_ERROR_BAD_GRAPH;
                goto free_exit;
            }
                

            for (uint32_t j = 0; j < neghbours_cnt; ++j){
                uint32_t neighbour = neighbours[j].vertex_id;
                if (visited[neighbour] && parents[cur_vertex] != neighbour){
                    out = true;
                    goto free_exit;
                } else if (visited[neighbour])
                    continue;

                visited[neighbour] = true;
                parents[neighbour] = cur_vertex;
                stack_push(stk, &vtx_ids[neighbour]);
            }
        }
    }



free_exit:
    stack_free(stk);
    free(visited);
    free(parents);
    free(vtx_ids);
    *out_has_cycle = out;
    return ret_val;
}


ga_status_t ga_dijkstra(graph_inst_t *graph, uint32_t from,
                        uint32_t to, uint32_t **out_path,
                        size_t *out_length, uint64_t *out_total_weight)
{
    ga_status_t ret_val = GA_STATUS_OK;
    if (!graph || !out_path || !out_length || !out_total_weight)
        return GA_ERROR_BAD_ARG;

    *out_path = NULL;
    *out_length = 0;
    *out_total_weight = 0;

    uint32_t vertices_cnt = 0;
    if(graph_get_vertices_cnt(graph, &vertices_cnt))
        return GA_ERROR_BAD_GRAPH;

    if(to >= vertices_cnt || from >= vertices_cnt)
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

    uint64_t *dist = malloc(vertices_cnt*sizeof(*dist));
    if (!dist)
        return GA_ERROR_MEMALLOC;

    memset(dist, 0xFF, vertices_cnt*sizeof(*dist));

    uint32_t *parent = calloc(vertices_cnt, sizeof(*parent));
    if (!parent){
        free(dist);
        return GA_ERROR_MEMALLOC;
    }
        
    bool *settled = calloc(vertices_cnt, sizeof(*settled));
    if (!settled){
        free(dist);
        free(parent);
        return GA_ERROR_MEMALLOC;
    }

    addr_heap_handle_t **handles = calloc(vertices_cnt, sizeof(*handles));
    if (!handles){
        free(dist);
        free(parent);
        free(settled);
        return GA_ERROR_MEMALLOC;
    }
        
    addr_heap_inst_t *heap = addr_heap_allocate(HEAP_MIN);
    if (!heap){
        free(dist);
        free(parent);
        free(settled);
        free(handles);
        return GA_ERROR_MEMALLOC;
    }

    uint32_t *vertex_ids = malloc(vertices_cnt*sizeof(*vertex_ids));
    if (!vertex_ids){
        free(dist);
        free(parent);
        free(settled);
        free(handles);
        addr_heap_deallocate(heap);
        return GA_ERROR_MEMALLOC;
    }
    for (uint32_t i = 0; i < vertices_cnt; i++){
        vertex_ids[i]= i;
    }

    int stat = addr_heap_add_item(heap, &vertex_ids[from], 0, &handles[from]);
    if(stat){
        assert(stat != -1);
        ret_val = GA_ERROR_MEMALLOC;
        goto free_exit;
    }
    dist[from] = 0;
        

    addr_heap_item_t out = {0};
    while(!addr_heap_pop(heap, &out)){
        uint32_t curr_vtx = *(uint32_t*)out.data;

        settled[curr_vtx] = true;
        addr_heap_free_handler(handles[curr_vtx]);
        handles[curr_vtx] = NULL;

        if (curr_vtx == to)
            goto check_path;

        const graph_edge_t *neighbours = NULL;
        uint32_t neghbours_cnt = 0;
        if (graph_get_neighbours(graph, curr_vtx, 
                        &neighbours, &neghbours_cnt) != GRAPH_STATUS_OK){
            ret_val = GA_ERROR_BAD_GRAPH;
            goto free_exit;
        }
            

        for (uint32_t j = 0; j < neghbours_cnt; ++j){
            uint32_t neighbour = neighbours[j].vertex_id;
            uint64_t weight = neighbours[j].weight;
            if (settled[neighbour])
                continue;

            uint64_t new_dist = dist[curr_vtx] + weight;

            if (new_dist < dist[neighbour]){
                dist[neighbour] = new_dist;
                parent[neighbour] = curr_vtx;

                if (handles[neighbour]){
                    int status = addr_heap_set_priority(
                        heap, new_dist, handles[neighbour]);

                    if (status != 0) {
                        ret_val = GA_ERROR_BAD_GRAPH;
                        goto free_exit;
                    }
                } else {
                    if (addr_heap_add_item(heap, &vertex_ids[neighbour], new_dist, &handles[neighbour])){
                        ret_val = GA_ERROR_MEMALLOC;
                        goto free_exit;
                    }
                }
            }
        }
    }

check_path:
    if (!settled[to]){
        ret_val = GA_ERROR_CANT_FIND_PATH;
        goto free_exit;
    }

    uint32_t *bout_path = calloc(vertices_cnt, sizeof(*bout_path));
    if (!bout_path){
        ret_val = GA_ERROR_MEMALLOC;
        goto free_exit;
    }

    uint32_t out_size = vertices_cnt;
    uint32_t cur_vert = to;
    uint32_t idx = 0;
    while (cur_vert != from) {
        bout_path[idx++] = cur_vert;
        cur_vert = parent[cur_vert];
        assert(settled[cur_vert] == true);
    }
    bout_path[idx++] = cur_vert;

    if (idx < out_size){
        uint32_t *out_b = realloc(bout_path, idx*sizeof(uint32_t));
        if (out_b){
            bout_path = out_b;
            out_size = idx;
        }   
    }
    

    // reverse
    for (uint32_t i = 0; i < idx / 2; ++i){
        uint32_t tmp = bout_path[i];
        bout_path[i] = bout_path[idx - i - 1];
        bout_path[idx - i - 1] = tmp;
    }

    *out_path = bout_path;
    *out_length = idx; 
    *out_total_weight = dist[to];   


free_exit:
    free(dist);
    free(parent);
    free(settled);
    addr_heap_deallocate(heap);
    for (uint32_t i = 0; i < vertices_cnt; i++){
        addr_heap_free_handler(handles[i]);
    }
    free(handles);   
    free(vertex_ids);

    return ret_val;
}


ga_status_t ga_topological_sort(graph_inst_t *graph, uint32_t **out_order, size_t *out_count)
{
    ga_status_t ret_val = GA_STATUS_OK;

    if (!graph || !out_order || !out_count)
        return GA_ERROR_BAD_ARG;

    *out_order = NULL;
    *out_count = 0;

    graph_type_t type;
    graph_get_type(graph, &type);
    if (type != GRAPH_DIRECTED)
        return GA_ERROR_UNSUPPORTED_GRAPH;

    uint32_t vertices_cnt = 0;
    if(graph_get_vertices_cnt(graph, &vertices_cnt))
        return GA_ERROR_BAD_GRAPH;

    if(!vertices_cnt){
        return GA_STATUS_OK;
    }

    uint32_t *indegree = calloc(vertices_cnt, sizeof(*indegree));
    if (!indegree){
        return GA_ERROR_MEMALLOC;
    }

    uint32_t *result = calloc(vertices_cnt, sizeof(*result));
    if (!result){
        free(indegree);
        return GA_ERROR_MEMALLOC;
    }

    uint32_t idx = 0;

    struct queue* queue = queue_create(vertices_cnt);
    if (!queue){
        ret_val = GA_ERROR_MEMALLOC;
        free(result);
        goto free_exit;
    }

    for (uint32_t i = 0; i < vertices_cnt; i++){
        const graph_edge_t *neighbours = NULL;
        uint32_t neighbours_cnt = 0;
        if (graph_get_neighbours(graph, i, 
                        &neighbours, &neighbours_cnt) != GRAPH_STATUS_OK){
            ret_val = GA_ERROR_BAD_GRAPH;
            free(result);
            goto free_exit;
        }
            

        for (uint32_t j = 0; j < neighbours_cnt; ++j){
            uint32_t neighbour = neighbours[j].vertex_id;
            indegree[neighbour]++;
        }
    }

    for (uint32_t i = 0; i < vertices_cnt; i++){
        if (indegree[i] == 0)
            queue_enqueue(queue, i);
    }

    while(!queue_is_empty(queue)){
        uint32_t cur_vert = queue_dequeue(queue);
        result[idx++] = cur_vert;

        const graph_edge_t *neighbours = NULL;
        uint32_t neighbours_cnt = 0;
        if (graph_get_neighbours(graph, cur_vert, 
                        &neighbours, &neighbours_cnt) != GRAPH_STATUS_OK){
            ret_val = GA_ERROR_BAD_GRAPH;
            free(result);
            goto free_exit;
        }
            
        for (uint32_t j = 0; j < neighbours_cnt; ++j){
            uint32_t neighbour = neighbours[j].vertex_id;
            indegree[neighbour]--;
            if (indegree[neighbour] == 0)
                queue_enqueue(queue, neighbour);
        }
    }

    if (idx < vertices_cnt){
        ret_val = GA_ERROR_CYCLE;
        free(result);
        goto free_exit;
    }

    *out_order = result;
    *out_count = idx;

free_exit:
    free(indegree);
    queue_free(queue);

    return ret_val;
}

typedef enum {DFS_WHITE, DFS_GRAY, DFS_BLACK} vtx_color_t;
typedef struct {
    uint32_t vertex;
    uint32_t next_neighbour;
} dfs_frame_t;

static ga_status_t ga_has_cycle_directed(graph_inst_t *graph, bool *out_has_cycle)
{
    ga_status_t ret_val = GA_STATUS_OK;

    if (!graph || !out_has_cycle )
        return GA_ERROR_BAD_ARG;

    *out_has_cycle = false;

    uint32_t vertices_cnt = 0;
    if(graph_get_vertices_cnt(graph, &vertices_cnt))
        return GA_ERROR_BAD_GRAPH;

    if(!vertices_cnt){
        return GA_STATUS_OK;
    }

    struct stack *stk = stack_create(vertices_cnt);
    if (!stk)
        return GA_ERROR_MEMALLOC;

    vtx_color_t *colors = calloc(vertices_cnt, sizeof(*colors));
    if(!colors){
        stack_free(stk);
        return GA_ERROR_MEMALLOC;
    }

    dfs_frame_t *frames = calloc(vertices_cnt, sizeof(*frames));
    if(!frames){
        stack_free(stk);
        free(colors);
        return GA_ERROR_MEMALLOC;
    }

    bool has_cycle = false;
    for (uint32_t i = 0; i < vertices_cnt; i++){
        if (colors[i] != DFS_WHITE)
            continue;

        colors[i] = DFS_GRAY;
        frames[i].vertex = i;
        frames[i].next_neighbour = 0;
        stack_push(stk, &frames[i]);

        while (!stack_is_empty(stk)){
            dfs_frame_t *frame = stack_peek(stk);

            const graph_edge_t *neighbours = NULL;
            uint32_t neighbours_cnt = 0;
            if (graph_get_neighbours(graph, frame->vertex, 
                            &neighbours, &neighbours_cnt) != GRAPH_STATUS_OK){
                ret_val = GA_ERROR_BAD_GRAPH;
                goto free_exit;
            }

            if (frame->next_neighbour == neighbours_cnt){
                colors[frame->vertex] = DFS_BLACK;
                stack_pop(stk);
                continue;
            }

            uint32_t neighbour = neighbours[frame->next_neighbour].vertex_id;
            frame->next_neighbour++;
             
            if(colors[neighbour] == DFS_GRAY){
                has_cycle = true;
                goto early_exit;
            }

            if(colors[neighbour] == DFS_WHITE){
                colors[neighbour] = DFS_GRAY;
                frames[neighbour].vertex = neighbour;
                frames[neighbour].next_neighbour = 0;
                stack_push(stk, &frames[neighbour]);
            }
        }
    }

early_exit:
    *out_has_cycle = has_cycle;

free_exit:
    stack_free(stk);  
    free(colors);  
    free(frames);

    return ret_val;
}

ga_status_t ga_find_scc(graph_inst_t *graph, uint32_t **out_components,
                            uint32_t *out_vertices_count, uint32_t *out_components_count)
{
    ga_status_t ret_val = GA_STATUS_OK;
    if (!graph || !out_components || !out_vertices_count || !out_components_count)
        return GA_ERROR_BAD_ARG;

    *out_components = NULL;
    *out_vertices_count = 0;
    *out_components_count = 0;

    graph_type_t type;
    graph_get_type(graph, &type);

    if (type != GRAPH_DIRECTED)
        return GA_ERROR_UNSUPPORTED_GRAPH;

    uint32_t vertices_cnt = 0;
    if(graph_get_vertices_cnt(graph, &vertices_cnt))
        return GA_ERROR_BAD_GRAPH;

    if(!vertices_cnt)
        return GA_STATUS_OK;

    if(vertices_cnt == 1){
        uint32_t *out = malloc(sizeof(uint32_t));
        if (!out)
            return GA_ERROR_MEMALLOC;
        out[0] = 0;
        *out_components = out;
        *out_vertices_count = 1;
        *out_components_count = 1;
        return GA_STATUS_OK;
    }

    uint32_t *components = calloc(vertices_cnt, sizeof(uint32_t));
    if(!components)
        return GA_ERROR_MEMALLOC;

    uint32_t *vtx_ids = calloc(vertices_cnt, sizeof(*vtx_ids));
    if(!vtx_ids){
        free(components);
        return GA_ERROR_MEMALLOC;
    }

    for(uint32_t i = 0; i < vertices_cnt; i++)
        vtx_ids[i] = i;

    struct stack *stk = stack_create(vertices_cnt);
    if(!stk){
        free(vtx_ids);
        free(components);
        return GA_ERROR_MEMALLOC;
    }

    memset(components, 0xFF, vertices_cnt*sizeof(*components));  

    bool *visited = calloc(vertices_cnt, sizeof(*visited));
    if(!visited){
        free(vtx_ids);
        free(components);
        stack_free(stk);
        return GA_ERROR_MEMALLOC;
    }

    uint32_t current_order_idx = 0;
    uint32_t *finish_order = malloc(vertices_cnt * sizeof(*finish_order));
    if(!finish_order){
        free(vtx_ids);
        free(components);
        stack_free(stk);
        free(visited);
        return GA_ERROR_MEMALLOC;
    }

    dfs_frame_t *frames = calloc(vertices_cnt, sizeof(*frames));
    if(!frames){
        free(vtx_ids);
        free(components);
        stack_free(stk);
        free(visited);
        free(finish_order);
        return GA_ERROR_MEMALLOC;
    }

    // First DFS
    for (uint32_t i = 0; i < vertices_cnt; i++){
        if (visited[i])
            continue;

        frames[i].vertex = i;
        frames[i].next_neighbour = 0;
        visited[i] = true;
        stack_push(stk, &frames[i]);

        while (!stack_is_empty(stk)){
            dfs_frame_t *frame = stack_peek(stk);

            const graph_edge_t *neighbours = NULL;
            uint32_t neighbours_cnt = 0;
            if (graph_get_neighbours(graph, frame->vertex, 
                            &neighbours, &neighbours_cnt) != GRAPH_STATUS_OK){
                ret_val = GA_ERROR_BAD_GRAPH;
                goto free_exit;
            }

            if (frame->next_neighbour == neighbours_cnt){
                finish_order[current_order_idx++] = frame->vertex;
                stack_pop(stk);
                continue;
            }

            uint32_t neighbour = neighbours[frame->next_neighbour].vertex_id;
            frame->next_neighbour++;
             
            if(!visited[neighbour]){
                frames[neighbour].vertex = neighbour;
                frames[neighbour].next_neighbour = 0;
                visited[neighbour] = true;
                stack_push(stk, &frames[neighbour]);
            }
        }
    }

    // Transposing
    graph_inst_t *t_graph = graph_create(GRAPH_DIRECTED);
    for (uint32_t i = 0; i < vertices_cnt; i++){
        uint32_t vertex_id = 0;
        if (graph_add_vertex(t_graph, &vertex_id)){
            ret_val = GA_ERROR_MEMALLOC;
            graph_free(t_graph);
            goto free_exit;
        }
    }

    for (uint32_t u = 0; u < vertices_cnt; ++u) {
        const graph_edge_t *neighbours = NULL;
        uint32_t count = 0;

        graph_get_neighbours(graph, u, &neighbours, &count);

        for (uint32_t j = 0; j < count; ++j) {
            uint32_t neighbour = neighbours[j].vertex_id;

            if (graph_add_edge(t_graph, neighbour, u)){
                ret_val = GA_ERROR_MEMALLOC;
                graph_free(t_graph);
                goto free_exit;
            }
        }
    }

    stack_free(stk);
    stk = stack_create(vertices_cnt);

    // Second DFS
    uint32_t component_id = 0;
    for (size_t i = vertices_cnt; i > 0; --i) {
        uint32_t start = finish_order[i - 1];

        if (components[start] != UINT32_MAX)
            continue;

        
        stack_push(stk, &vtx_ids[start]);
        components[start] = component_id;
        while (!stack_is_empty(stk)){
            uint32_t vertex = *(uint32_t*)stack_pop(stk);

            const graph_edge_t *neighbours = NULL;
            uint32_t neighbours_cnt = 0;
            if (graph_get_neighbours(t_graph, vertex, 
                            &neighbours, &neighbours_cnt) != GRAPH_STATUS_OK){
                ret_val = GA_ERROR_BAD_GRAPH;
                goto free_exit;
            }

            for (uint32_t j = 0; j < neighbours_cnt; ++j){
                uint32_t neighbour = neighbours[j].vertex_id;
                if (components[neighbour] == UINT32_MAX){
                    components[neighbour] = component_id;
                    stack_push(stk, &vtx_ids[neighbour]);
                }
            }
        }
        component_id++;
    }

    *out_components = components;
    *out_vertices_count = vertices_cnt;
    *out_components_count = component_id;

    graph_free(t_graph);

free_exit:
    free(vtx_ids);
    stack_free(stk);
    free(visited);
    free(finish_order);
    free(frames);
    

    return ret_val;
}