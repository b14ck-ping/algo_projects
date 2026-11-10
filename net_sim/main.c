
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

#include "graph.h"
#include "graph_algorithms.h"

#define CHECK(expr)                                                   \
    do {                                                              \
        if (!(expr)) {                                                \
            fprintf(stderr,                                          \
                    "[FAIL] %s:%d: %s\n",                             \
                    __FILE__, __LINE__, #expr);                      \
            exit(EXIT_FAILURE);                                      \
        }                                                             \
    } while (0)

#define RUN_TEST(test)                                                \
    do {                                                              \
        printf("[RUN ] %s\n", #test);                                  \
        test();                                                       \
        printf("[PASS] %s\n", #test);                                  \
    } while (0)

static uint32_t add_vertex(graph_inst_t *graph)
{
    uint32_t id = UINT32_MAX;

    CHECK(graph_add_vertex(graph, &id) == GRAPH_STATUS_OK);
    CHECK(id != UINT32_MAX);

    return id;
}

static uint32_t count_neighbour(graph_inst_t *graph,
                                uint32_t vertex,
                                uint32_t neighbour)
{
    const graph_edge_t *list = NULL;
    uint32_t count = UINT32_MAX;
    uint32_t occurrences = 0;

    CHECK(graph_get_neighbours(graph, vertex,
                              &list, &count) == GRAPH_STATUS_OK);

    CHECK(count == 0 || list != NULL);

    for (uint32_t i = 0; i < count; ++i) {
        if (list[i].vertex_id == neighbour)
            ++occurrences;
    }

    return occurrences;
}

static void check_degree(graph_inst_t *graph,
                         uint32_t vertex,
                         uint32_t expected)
{
    const graph_edge_t *list = NULL;
    uint32_t count = UINT32_MAX;

    CHECK(graph_get_neighbours(graph, vertex,
                              &list, &count) == GRAPH_STATUS_OK);
    CHECK(count == expected);
    CHECK(count == 0 || list != NULL);
}


static void check_edge(graph_inst_t *graph,
                       uint32_t from,
                       uint32_t to,
                       uint32_t expected_weight)
{
    const graph_edge_t *edges = NULL;
    uint32_t count = 0;
    uint32_t matches = 0;

    CHECK(graph_get_neighbours(graph, from, &edges, &count)
          == GRAPH_STATUS_OK);

    for (uint32_t i = 0; i < count; ++i) {
        if (edges[i].vertex_id == to) {
            CHECK(edges[i].weight == expected_weight);
            ++matches;
        }
    }

    // Ребро существует ровно в одном экземпляре
    CHECK(matches == 1);
}

static void check_no_edge(graph_inst_t *graph,
                          uint32_t from,
                          uint32_t to)
{
    const graph_edge_t *edges = NULL;
    uint32_t count = 0;

    CHECK(graph_get_neighbours(graph, from, &edges, &count)
          == GRAPH_STATUS_OK);

    for (uint32_t i = 0; i < count; ++i)
        CHECK(edges[i].vertex_id != to);
}

/* ---------- Test 1: creation and empty graph ---------- */

static void test_create_free(void)
{
    graph_inst_t *graph = graph_create(GRAPH_UNDIRECTED);

    CHECK(graph != NULL);

    graph_free(graph);
    graph_free(NULL);
}

/* ---------- Test 2: vertex IDs ---------- */

static void test_vertex_ids(void)
{
    graph_inst_t *graph = graph_create(GRAPH_UNDIRECTED);
    CHECK(graph != NULL);

    /*
     * Initial capacity is 16.
     * Creating 40 vertices forces expansion.
     */
    for (uint32_t i = 0; i < 40; ++i) {
        uint32_t id = add_vertex(graph);
        CHECK(id == i);
        check_degree(graph, id, 0);
    }

    graph_free(graph);
}

/* ---------- Test 3: undirected edges ---------- */

static void test_undirected_edges(void)
{
    graph_inst_t *graph = graph_create(GRAPH_UNDIRECTED);
    CHECK(graph != NULL);

    uint32_t a = add_vertex(graph);
    uint32_t b = add_vertex(graph);
    uint32_t c = add_vertex(graph);

    CHECK(graph_add_edge(graph, a, b) == GRAPH_STATUS_OK);
    CHECK(graph_add_edge(graph, b, c) == GRAPH_STATUS_OK);

    check_degree(graph, a, 1);
    check_degree(graph, b, 2);
    check_degree(graph, c, 1);

    CHECK(count_neighbour(graph, a, b) == 1);
    CHECK(count_neighbour(graph, b, a) == 1);
    CHECK(count_neighbour(graph, b, c) == 1);
    CHECK(count_neighbour(graph, c, b) == 1);

    CHECK(count_neighbour(graph, a, c) == 0);

    graph_free(graph);
}

/* ---------- Test 4: duplicate edges ---------- */

static void test_duplicate_edges(void)
{
    graph_inst_t *graph = graph_create(GRAPH_UNDIRECTED);
    CHECK(graph != NULL);

    uint32_t a = add_vertex(graph);
    uint32_t b = add_vertex(graph);

    CHECK(graph_add_edge(graph, a, b) == GRAPH_STATUS_OK);
    CHECK(graph_add_edge(graph, a, b) == GRAPH_STATUS_OK);
    CHECK(graph_add_edge(graph, b, a) == GRAPH_STATUS_OK);

    check_degree(graph, a, 1);
    check_degree(graph, b, 1);

    CHECK(count_neighbour(graph, a, b) == 1);
    CHECK(count_neighbour(graph, b, a) == 1);

    graph_free(graph);
}

/* ---------- Test 5: self-loop ---------- */

static void test_self_loop(void)
{
    graph_inst_t *graph = graph_create(GRAPH_UNDIRECTED);
    CHECK(graph != NULL);

    uint32_t a = add_vertex(graph);

    CHECK(graph_add_edge(graph, a, a) == GRAPH_ERROR_LOOP);

    check_degree(graph, a, 0);

    graph_free(graph);
}

/* ---------- Test 6: isolated vertices ---------- */

static void test_isolated_vertices(void)
{
    graph_inst_t *graph = graph_create(GRAPH_UNDIRECTED);
    CHECK(graph != NULL);

    uint32_t a = add_vertex(graph);
    uint32_t b = add_vertex(graph);
    uint32_t c = add_vertex(graph);

    CHECK(graph_add_edge(graph, a, b) == GRAPH_STATUS_OK);

    check_degree(graph, c, 0);

    const graph_edge_t *list = NULL;
    uint32_t count = UINT32_MAX;

    CHECK(graph_get_neighbours(graph, c,
                              &list, &count) == GRAPH_STATUS_OK);
    CHECK(list == NULL);
    CHECK(count == 0);

    graph_free(graph);
}

/* ---------- Test 7: invalid arguments ---------- */

static void test_invalid_arguments(void)
{
    graph_inst_t *graph = graph_create(GRAPH_UNDIRECTED);
    CHECK(graph != NULL);

    uint32_t id = add_vertex(graph);

    CHECK(graph_add_vertex(NULL, &id) == GRAPH_ERROR_BAD_ARG);
    CHECK(graph_add_vertex(graph, NULL) == GRAPH_ERROR_BAD_ARG);

    CHECK(graph_add_edge(NULL, 0, 1) == GRAPH_ERROR_BAD_ARG);

    CHECK(graph_add_edge(graph, 0, 1)
          == GRAPH_ERROR_VERTEX_IS_NOT_EXIST);

    CHECK(graph_add_edge(graph, UINT32_MAX, 0)
          == GRAPH_ERROR_VERTEX_IS_NOT_EXIST);

    const graph_edge_t *list = NULL;
    uint32_t count = 0;

    CHECK(graph_get_neighbours(NULL, 0, &list, &count)
          == GRAPH_ERROR_BAD_ARG);

    CHECK(graph_get_neighbours(graph, 0, NULL, &count)
          == GRAPH_ERROR_BAD_ARG);

    CHECK(graph_get_neighbours(graph, 0, &list, NULL)
          == GRAPH_ERROR_BAD_ARG);

    CHECK(graph_get_neighbours(graph, 1, &list, &count)
          == GRAPH_ERROR_VERTEX_IS_NOT_EXIST);

    graph_free(graph);
}

/* ---------- Test 8: adjacency array expansion ---------- */

static void test_many_neighbours(void)
{
    graph_inst_t *graph = graph_create(GRAPH_UNDIRECTED);
    CHECK(graph != NULL);

    enum { VERTICES = 100 };

    uint32_t ids[VERTICES];

    for (uint32_t i = 0; i < VERTICES; ++i)
        ids[i] = add_vertex(graph);

    /*
     * Star topology:
     *
     * 1 --+
     * 2 --+
     * 3 --+-- 0
     * ... |
     * 99 -+
     *
     * Vertex 0 has 99 neighbours.
     * This forces multiple adjacency array expansions.
     */
    for (uint32_t i = 1; i < VERTICES; ++i) {
        CHECK(graph_add_edge(graph, ids[0], ids[i])
              == GRAPH_STATUS_OK);
    }

    check_degree(graph, ids[0], VERTICES - 1);

    for (uint32_t i = 1; i < VERTICES; ++i) {
        check_degree(graph, ids[i], 1);

        CHECK(count_neighbour(graph, ids[0], ids[i]) == 1);
        CHECK(count_neighbour(graph, ids[i], ids[0]) == 1);
    }

    graph_free(graph);
}

/* ---------- Test 9: larger graph ---------- */

static void test_large_graph(void)
{
    graph_inst_t *graph = graph_create(GRAPH_UNDIRECTED);
    CHECK(graph != NULL);

    enum { VERTICES = 10000 };

    for (uint32_t i = 0; i < VERTICES; ++i) {
        CHECK(add_vertex(graph) == i);
    }

    /*
     * Chain topology:
     * 0 -- 1 -- 2 -- ... -- 9999
     */
    for (uint32_t i = 0; i < VERTICES - 1; ++i) {
        CHECK(graph_add_edge(graph, i, i + 1)
              == GRAPH_STATUS_OK);
    }

    check_degree(graph, 0, 1);
    check_degree(graph, VERTICES - 1, 1);

    for (uint32_t i = 1; i < VERTICES - 1; ++i) {
        check_degree(graph, i, 2);
        CHECK(count_neighbour(graph, i, i - 1) == 1);
        CHECK(count_neighbour(graph, i, i + 1) == 1);
    }

    graph_free(graph);
}

static graph_inst_t *create_graph(uint32_t count)
{
    graph_inst_t *g = graph_create(GRAPH_UNDIRECTED);
    assert(g);

    for (uint32_t i = 0; i < count; ++i) {
        uint32_t id = UINT32_MAX;

        assert(graph_add_vertex(g, &id) == GRAPH_STATUS_OK);
        assert(id == i);
    }

    return g;
}

static void add_edge(graph_inst_t *g, uint32_t a, uint32_t b)
{
    assert(graph_add_edge(g, a, b) == GRAPH_STATUS_OK);
}

/*
 * Test 1: Simple chain
 *
 * 0 --- 1 --- 2 --- 3
 *
 * Expected path: 0 -> 1 -> 2 -> 3
 */
static void test_bfs_simple(void)
{
    graph_inst_t *g = create_graph(4);

    add_edge(g, 0, 1);
    add_edge(g, 1, 2);
    add_edge(g, 2, 3);

    uint32_t *path = NULL;
    size_t length = 0;

    assert(ga_find_shortest_way(
        g, 0, 3, &path, &length) == GA_STATUS_OK);

    assert(path != NULL);
    assert(length == 4);

    const uint32_t expected[] = {0, 1, 2, 3};

    for (size_t i = 0; i < length; ++i)
        assert(path[i] == expected[i]);

    free(path);
    graph_free(g);

    printf("[PASS] test_bfs_simple\n");
}

/*
 * Test 2: Shortest path, not first available path
 *
 * 0 --- 1 --- 2 --- 3
 * |                 |
 * +------- 4 ------+
 *
 * From 0 to 3:
 * 0 -> 1 -> 2 -> 3  (3 edges)
 * 0 -> 4 -> 3       (2 edges)
 *
 * Expected: 0 -> 4 -> 3
 */
static void test_bfs_shortest(void)
{
    graph_inst_t *g = create_graph(5);

    add_edge(g, 0, 1);
    add_edge(g, 1, 2);
    add_edge(g, 2, 3);

    /* Shorter alternative added last. */
    add_edge(g, 0, 4);
    add_edge(g, 4, 3);

    uint32_t *path = NULL;
    size_t length = 0;

    assert(ga_find_shortest_way(
        g, 0, 3, &path, &length) == GA_STATUS_OK);

    assert(path != NULL);
    assert(length == 3);

    const uint32_t expected[] = {0, 4, 3};

    for (size_t i = 0; i < length; ++i)
        assert(path[i] == expected[i]);

    free(path);
    graph_free(g);

    printf("[PASS] test_bfs_shortest\n");
}


/* BFS: start and destination are the same vertex */
static void test_bfs_same_vertex(void)
{
    graph_inst_t *g = graph_create(GRAPH_UNDIRECTED);
    CHECK(g != NULL);

    CHECK(add_vertex(g) == 0);

    uint32_t *path = NULL;
    size_t length = 0;

    CHECK(ga_find_shortest_way(g, 0, 0, &path, &length)
          == GA_STATUS_OK);

    CHECK(path != NULL);
    CHECK(length == 1);
    CHECK(path[0] == 0);

    free(path);
    graph_free(g);
}

/* BFS: no connection between two components */
static void test_bfs_unreachable(void)
{
    graph_inst_t *g = graph_create(GRAPH_UNDIRECTED);
    CHECK(g != NULL);

    for (int i = 0; i < 5; ++i)
        add_vertex(g);

    /* 0 -- 1 -- 2     3 -- 4 */
    CHECK(graph_add_edge(g, 0, 1) == GRAPH_STATUS_OK);
    CHECK(graph_add_edge(g, 1, 2) == GRAPH_STATUS_OK);
    CHECK(graph_add_edge(g, 3, 4) == GRAPH_STATUS_OK);

    uint32_t *path = NULL;
    size_t length = 0;

    CHECK(ga_find_shortest_way(g, 0, 4, &path, &length)
          == GA_ERROR_CANT_FIND_PATH);

    CHECK(path == NULL);
    CHECK(length == 0);

    graph_free(g);
}

/* BFS: invalid vertex IDs and empty graph */
static void test_bfs_invalid_vertices(void)
{
    graph_inst_t *g = graph_create(GRAPH_UNDIRECTED);
    CHECK(g != NULL);

    uint32_t *path = NULL;
    size_t length = 0;

    /* Empty graph */
    CHECK(ga_find_shortest_way(g, 0, 0, &path, &length)
          == GA_ERROR_VERTEX_IS_NOT_EXIST);

    add_vertex(g);

    CHECK(ga_find_shortest_way(g, 0, 1, &path, &length)
          == GA_ERROR_VERTEX_IS_NOT_EXIST);

    CHECK(ga_find_shortest_way(g, UINT32_MAX, 0,
                               &path, &length)
          == GA_ERROR_VERTEX_IS_NOT_EXIST);

    CHECK(ga_find_shortest_way(NULL, 0, 0, &path, &length)
          == GA_ERROR_BAD_ARG);

    graph_free(g);
}

/*
 * BFS: multiple routes with equal shortest distance
 *
 *     1 --- 3
 *    /       \
 *   0         5
 *    \       /
 *     2 --- 4
 *
 * Correct result:
 * 0 -> 1 -> 3 -> 5
 * or
 * 0 -> 2 -> 4 -> 5
 */
static void test_bfs_multiple_paths(void)
{
    graph_inst_t *g = graph_create(GRAPH_UNDIRECTED);
    CHECK(g != NULL);

    for (int i = 0; i < 6; ++i)
        add_vertex(g);

    CHECK(graph_add_edge(g, 0, 1) == GRAPH_STATUS_OK);
    CHECK(graph_add_edge(g, 0, 2) == GRAPH_STATUS_OK);
    CHECK(graph_add_edge(g, 1, 3) == GRAPH_STATUS_OK);
    CHECK(graph_add_edge(g, 2, 4) == GRAPH_STATUS_OK);
    CHECK(graph_add_edge(g, 3, 5) == GRAPH_STATUS_OK);
    CHECK(graph_add_edge(g, 4, 5) == GRAPH_STATUS_OK);

    uint32_t *path = NULL;
    size_t length = 0;

    CHECK(ga_find_shortest_way(g, 0, 5, &path, &length)
          == GA_STATUS_OK);

    CHECK(path != NULL);
    CHECK(length == 4);
    CHECK(path[0] == 0);
    CHECK(path[3] == 5);

    bool first_route =
        path[1] == 1 && path[2] == 3;

    bool second_route =
        path[1] == 2 && path[2] == 4;

    CHECK(first_route || second_route);

    free(path);
    graph_free(g);
}


/* ---------- Helpers for component tests ---------- */

static graph_inst_t *make_graph_checked(uint32_t count)
{
    graph_inst_t *graph = graph_create(GRAPH_UNDIRECTED);
    CHECK(graph != NULL);

    for (uint32_t i = 0; i < count; ++i)
        CHECK(add_vertex(graph) == i);

    return graph;
}

static void connect(graph_inst_t *graph, uint32_t a, uint32_t b)
{
    CHECK(graph_add_edge(graph, a, b) == GRAPH_STATUS_OK);
}


static void check_component_partition(
    graph_inst_t *graph,
    const uint32_t *expected_groups,
    uint32_t vertices_count,
    uint32_t expected_count)
{
    uint32_t *actual_ids = NULL;
    uint32_t actual_comp_count = UINT32_MAX;
    uint32_t actual_vert_count = UINT32_MAX;

    CHECK(ga_find_components(
        graph, &actual_ids, &actual_vert_count, &actual_comp_count) == GA_STATUS_OK);

    CHECK(actual_vert_count == vertices_count);
    CHECK(actual_comp_count == expected_count);
    CHECK(actual_ids != NULL || vertices_count == 0);

    for (uint32_t i = 0; i < vertices_count; ++i) {
        for (uint32_t j = i + 1; j < vertices_count; ++j) {
            bool expected_same =
                expected_groups[i] == expected_groups[j];

            bool actual_same =
                actual_ids[i] == actual_ids[j];

            CHECK(expected_same == actual_same);
        }
    }

    free(actual_ids);
}


/* ---------- Test 1: empty graph ---------- */

static void test_components_empty(void)
{
    graph_inst_t *graph = make_graph_checked(0);

    uint32_t dummy = 0;
    uint32_t *components = &dummy;
    uint32_t vert_count = UINT32_MAX;
    uint32_t comp_count = UINT32_MAX;

    CHECK(ga_find_components(
        graph, &components, &vert_count, &comp_count) == GA_STATUS_OK);

    CHECK(components == NULL);
    CHECK(comp_count == 0);
    CHECK(vert_count == 0);

    graph_free(graph);
}

/* ---------- Test 2: all vertices are isolated ---------- */

static void test_components_isolated(void)
{
    graph_inst_t *graph = make_graph_checked(5);

    const uint32_t groups[] = {0, 1, 2, 3, 4};

    check_component_partition(graph, groups, 5, 5);

    graph_free(graph);
}
/* ---------- Test 3: one connected component with cycles ---------- */

static void test_components_single_connected(void)
{
    graph_inst_t *graph = make_graph_checked(6);

    for (uint32_t i = 0; i < 6; ++i) {
        for (uint32_t j = i + 1; j < 6; ++j)
            connect(graph, i, j);
    }

    const uint32_t groups[] = {0, 0, 0, 0, 0, 0};

    check_component_partition(graph, groups, 6, 1);

    graph_free(graph);
}


/* ---------- Test 4: multiple components ---------- */

static void test_components_multiple(void)
{
    graph_inst_t *graph = make_graph_checked(9);

    connect(graph, 0, 1);
    connect(graph, 1, 2);
    connect(graph, 2, 0);

    connect(graph, 3, 4);

    connect(graph, 6, 7);
    connect(graph, 7, 8);

    const uint32_t groups[] = {
        0, 0, 0,
        1, 1,
        2,
        3, 3, 3
    };

    check_component_partition(graph, groups, 9, 4);

    graph_free(graph);
}


static void test_components_single_vertex(void)
{
    graph_inst_t *graph = make_graph_checked(1);

    uint32_t *components = NULL;
    uint32_t vertices_count = 0;
    uint32_t components_count = 0;

    CHECK(ga_find_components(
        graph,
        &components,
        &vertices_count,
        &components_count
    ) == GA_STATUS_OK);

    CHECK(components != NULL);
    CHECK(vertices_count == 1);
    CHECK(components_count == 1);
    CHECK(components[0] != UINT32_MAX);

    free(components);
    graph_free(graph);
}


/* ---------- Test 1: empty graph ---------- */

static void test_cycle_empty(void)
{
    graph_inst_t *graph = make_graph_checked(0);
    bool has_cycle = true;

    CHECK(ga_has_cycle(graph, &has_cycle) == GA_STATUS_OK);
    CHECK(has_cycle == false);

    graph_free(graph);
}

/* ---------- Test 2: single vertex ---------- */

static void test_cycle_single_vertex(void)
{
    graph_inst_t *graph = make_graph_checked(1);
    bool has_cycle = true;

    CHECK(ga_has_cycle(graph, &has_cycle) == GA_STATUS_OK);
    CHECK(has_cycle == false);

    graph_free(graph);
}

/* ---------- Test 3: chain without cycles ---------- */

/*
 * 0 --- 1 --- 2 --- 3 --- 4 --- 5
 */

static void test_cycle_chain(void)
{
    graph_inst_t *graph = make_graph_checked(6);

    for (uint32_t i = 0; i < 5; ++i)
        connect(graph, i, i + 1);

    bool has_cycle = true;

    CHECK(ga_has_cycle(graph, &has_cycle) == GA_STATUS_OK);
    CHECK(has_cycle == false);

    graph_free(graph);
}

/* ---------- Test 4: triangle ---------- */

/*
 * 0 --- 1
 *  \   /
 *    2
 */

static void test_cycle_triangle(void)
{
    graph_inst_t *graph = make_graph_checked(3);

    connect(graph, 0, 1);
    connect(graph, 1, 2);
    connect(graph, 2, 0);

    bool has_cycle = false;

    CHECK(ga_has_cycle(graph, &has_cycle) == GA_STATUS_OK);
    CHECK(has_cycle == true);

    graph_free(graph);
}

/* ---------- Test 5: tree without cycles ---------- */

/*
 *       0
 *      / \
 *     1   2
 *    / \   \
 *   3   4   5
 */

static void test_cycle_tree(void)
{
    graph_inst_t *graph = make_graph_checked(6);

    connect(graph, 0, 1);
    connect(graph, 0, 2);
    connect(graph, 1, 3);
    connect(graph, 1, 4);
    connect(graph, 2, 5);

    bool has_cycle = true;

    CHECK(ga_has_cycle(graph, &has_cycle) == GA_STATUS_OK);
    CHECK(has_cycle == false);

    graph_free(graph);
}

/* ---------- Test 6: disconnected graph with cycle ---------- */

/*
 * Component A: 0 --- 1 --- 2
 *
 * Component B: 3 --- 4
 *               \   /
 *                 5
 *
 * Component C: 6 (isolated)
 */

static void test_cycle_disconnected(void)
{
    graph_inst_t *graph = make_graph_checked(7);

    connect(graph, 0, 1);
    connect(graph, 1, 2);

    connect(graph, 3, 4);
    connect(graph, 4, 5);
    connect(graph, 5, 3);

    bool has_cycle = false;

    CHECK(ga_has_cycle(graph, &has_cycle) == GA_STATUS_OK);
    CHECK(has_cycle == true);

    graph_free(graph);
}

/* ---------- Test 7: all isolated vertices ---------- */

static void test_cycle_isolated(void)
{
    graph_inst_t *graph = make_graph_checked(10);

    bool has_cycle = true;

    CHECK(ga_has_cycle(graph, &has_cycle) == GA_STATUS_OK);
    CHECK(has_cycle == false);

    graph_free(graph);
}

/* ---------- Test 8: cycle at the end of a long chain ---------- */

/*
 * 0 -- 1 -- 2 -- 3 -- 4 -- 5
 *                       |    |
 *                       7 -- 6
 */

static void test_cycle_deep(void)
{
    graph_inst_t *graph = make_graph_checked(8);

    for (uint32_t i = 0; i < 7; ++i)
        connect(graph, i, i + 1);

    connect(graph, 7, 4);

    bool has_cycle = false;

    CHECK(ga_has_cycle(graph, &has_cycle) == GA_STATUS_OK);
    CHECK(has_cycle == true);

    graph_free(graph);
}

/* ---------- Test 9: invalid arguments ---------- */

static void test_cycle_invalid_args(void)
{
    graph_inst_t *graph = make_graph_checked(3);
    bool has_cycle = false;

    CHECK(ga_has_cycle(NULL, &has_cycle) == GA_ERROR_BAD_ARG);
    CHECK(ga_has_cycle(graph, NULL) == GA_ERROR_BAD_ARG);

    graph_free(graph);
}


/* ---------- Weighted undirected edges ---------- */

static void test_weighted_undirected(void)
{
    graph_inst_t *g = graph_create(GRAPH_UNDIRECTED);
    CHECK(g != NULL);

    add_vertex(g); // 0
    add_vertex(g); // 1
    add_vertex(g); // 2

    CHECK(graph_add_weighted_edge(g, 0, 1, 15)
          == GRAPH_STATUS_OK);
    CHECK(graph_add_weighted_edge(g, 1, 2, 42)
          == GRAPH_STATUS_OK);

    check_edge(g, 0, 1, 15);
    check_edge(g, 1, 0, 15);
    check_edge(g, 1, 2, 42);
    check_edge(g, 2, 1, 42);

    check_degree(g, 0, 1);
    check_degree(g, 1, 2);
    check_degree(g, 2, 1);

    check_no_edge(g, 0, 2);
    check_no_edge(g, 2, 0);

    graph_free(g);
}

/* ---------- Directed edges ---------- */

static void test_directed_edges(void)
{
    graph_inst_t *g = graph_create(GRAPH_DIRECTED);
    CHECK(g != NULL);

    add_vertex(g); // 0
    add_vertex(g); // 1
    add_vertex(g); // 2

    CHECK(graph_add_edge(g, 0, 1) == GRAPH_STATUS_OK);
    CHECK(graph_add_edge(g, 1, 2) == GRAPH_STATUS_OK);

    check_edge(g, 0, 1, 1);
    check_edge(g, 1, 2, 1);

    check_no_edge(g, 1, 0);
    check_no_edge(g, 2, 1);

    check_degree(g, 0, 1);
    check_degree(g, 1, 1);
    check_degree(g, 2, 0);

    graph_free(g);
}

/* ---------- Duplicate directed edge to sink ---------- */

static void test_directed_duplicate(void)
{
    graph_inst_t *g = graph_create(GRAPH_DIRECTED);
    CHECK(g != NULL);

    add_vertex(g); // 0
    add_vertex(g); // 1

    // Вершина 1 не имеет исходящих рёбер
    CHECK(graph_add_weighted_edge(g, 0, 1, 7)
          == GRAPH_STATUS_OK);

    CHECK(graph_add_weighted_edge(g, 0, 1, 7)
          == GRAPH_STATUS_OK);

    check_degree(g, 0, 1);
    check_degree(g, 1, 0);

    check_edge(g, 0, 1, 7);
    check_no_edge(g, 1, 0);

    graph_free(g);
}

/* ---------- Independent opposite directed edges ---------- */

static void test_directed_opposite_edges(void)
{
    graph_inst_t *g = graph_create(GRAPH_DIRECTED);
    CHECK(g != NULL);

    add_vertex(g); // 0
    add_vertex(g); // 1

    CHECK(graph_add_weighted_edge(g, 0, 1, 10)
          == GRAPH_STATUS_OK);

    CHECK(graph_add_weighted_edge(g, 1, 0, 25)
          == GRAPH_STATUS_OK);

    check_edge(g, 0, 1, 10);
    check_edge(g, 1, 0, 25);

    check_degree(g, 0, 1);
    check_degree(g, 1, 1);

    graph_free(g);
}

/* ---------- Mixed weighted and default edges ---------- */

static void test_mixed_weights(void)
{
    graph_inst_t *g = graph_create(GRAPH_UNDIRECTED);
    CHECK(g != NULL);

    for (uint32_t i = 0; i < 4; ++i)
        add_vertex(g);

    CHECK(graph_add_edge(g, 0, 1) == GRAPH_STATUS_OK);
    CHECK(graph_add_weighted_edge(g, 1, 2, 0)
          == GRAPH_STATUS_OK);
    CHECK(graph_add_weighted_edge(g, 2, 3, UINT32_MAX)
          == GRAPH_STATUS_OK);

    check_edge(g, 0, 1, 1);
    check_edge(g, 1, 0, 1);

    check_edge(g, 1, 2, 0);
    check_edge(g, 2, 1, 0);

    check_edge(g, 2, 3, UINT32_MAX);
    check_edge(g, 3, 2, UINT32_MAX);

    graph_free(g);
}

/* ---------- Adjacency array expansion ---------- */

static void test_weighted_edges_expansion(void)
{
    enum { VERTICES = 100 };

    graph_inst_t *g = graph_create(GRAPH_DIRECTED);
    CHECK(g != NULL);

    for (uint32_t i = 0; i < VERTICES; ++i)
        CHECK(add_vertex(g) == i);

    // 99 исходящих рёбер из вершины 0.
    // Превышаем начальную ёмкость массива (32).
    for (uint32_t i = 1; i < VERTICES; ++i) {
        CHECK(graph_add_weighted_edge(g, 0, i, i * 10)
              == GRAPH_STATUS_OK);
    }

    check_degree(g, 0, VERTICES - 1);

    for (uint32_t i = 1; i < VERTICES; ++i) {
        check_edge(g, 0, i, i * 10);
        check_no_edge(g, i, 0);
        check_degree(g, i, 0);
    }

    graph_free(g);
}

/* ---------- Graph type and invalid arguments ---------- */

static void test_weighted_graph_api(void)
{
    graph_inst_t *g = graph_create(GRAPH_DIRECTED);
    CHECK(g != NULL);

    graph_type_t type = GRAPH_UNDIRECTED;

    CHECK(graph_get_type(g, &type) == GRAPH_STATUS_OK);
    CHECK(type == GRAPH_DIRECTED);

    CHECK(graph_get_type(NULL, &type) == GRAPH_ERROR_BAD_ARG);
    CHECK(graph_get_type(g, NULL) == GRAPH_ERROR_BAD_ARG);

    add_vertex(g); // 0
    add_vertex(g); // 1

    CHECK(graph_add_weighted_edge(NULL, 0, 1, 5)
          == GRAPH_ERROR_BAD_ARG);

    CHECK(graph_add_weighted_edge(g, 0, 0, 5)
          == GRAPH_ERROR_LOOP);

    CHECK(graph_add_weighted_edge(g, 0, 99, 5)
          == GRAPH_ERROR_VERTEX_IS_NOT_EXIST);

    // Ошибочные вызовы не должны добавлять рёбра
    check_degree(g, 0, 0);
    check_degree(g, 1, 0);

    graph_free(g);
}

int main(void)
{
    printf("=== Graph Core Tests ===\n\n");

    RUN_TEST(test_create_free);
    RUN_TEST(test_vertex_ids);
    RUN_TEST(test_undirected_edges);
    RUN_TEST(test_duplicate_edges);
    RUN_TEST(test_self_loop);
    RUN_TEST(test_isolated_vertices);
    RUN_TEST(test_invalid_arguments);
    RUN_TEST(test_many_neighbours);
    RUN_TEST(test_large_graph);
    
    printf("\nAll graph tests passed!\n");

    RUN_TEST(test_weighted_undirected);
    RUN_TEST(test_directed_edges);
    RUN_TEST(test_directed_duplicate);
    RUN_TEST(test_directed_opposite_edges);
    RUN_TEST(test_mixed_weights);
    RUN_TEST(test_weighted_edges_expansion);
    RUN_TEST(test_weighted_graph_api);

    puts("All weighted/directed graph tests passed!");

    test_bfs_simple();
    test_bfs_shortest();

    RUN_TEST(test_bfs_same_vertex);
    RUN_TEST(test_bfs_unreachable);
    RUN_TEST(test_bfs_invalid_vertices);
    RUN_TEST(test_bfs_multiple_paths);

    puts("All BFS tests passed!");

    RUN_TEST(test_components_empty);
    RUN_TEST(test_components_isolated);
    RUN_TEST(test_components_single_connected);
    RUN_TEST(test_components_multiple);
    RUN_TEST(test_components_single_vertex);

    puts("All components tests passed!");

    RUN_TEST(test_cycle_empty);
    RUN_TEST(test_cycle_single_vertex);
    RUN_TEST(test_cycle_chain);
    RUN_TEST(test_cycle_triangle);
    RUN_TEST(test_cycle_tree);
    RUN_TEST(test_cycle_disconnected);
    RUN_TEST(test_cycle_isolated);
    RUN_TEST(test_cycle_deep);
    RUN_TEST(test_cycle_invalid_args);

    puts("All cycle detection tests passed!");

    return EXIT_SUCCESS;
}
