
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

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
    const uint32_t *list = NULL;
    uint32_t count = UINT32_MAX;
    uint32_t occurrences = 0;

    CHECK(graph_get_neighbours(graph, vertex,
                              &list, &count) == GRAPH_STATUS_OK);

    CHECK(count == 0 || list != NULL);

    for (uint32_t i = 0; i < count; ++i) {
        if (list[i] == neighbour)
            ++occurrences;
    }

    return occurrences;
}

static void check_degree(graph_inst_t *graph,
                         uint32_t vertex,
                         uint32_t expected)
{
    const uint32_t *list = NULL;
    uint32_t count = UINT32_MAX;

    CHECK(graph_get_neighbours(graph, vertex,
                              &list, &count) == GRAPH_STATUS_OK);
    CHECK(count == expected);
    CHECK(count == 0 || list != NULL);
}

/* ---------- Test 1: creation and empty graph ---------- */

static void test_create_free(void)
{
    graph_inst_t *graph = graph_create();

    CHECK(graph != NULL);

    graph_free(graph);
    graph_free(NULL);
}

/* ---------- Test 2: vertex IDs ---------- */

static void test_vertex_ids(void)
{
    graph_inst_t *graph = graph_create();
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
    graph_inst_t *graph = graph_create();
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
    graph_inst_t *graph = graph_create();
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
    graph_inst_t *graph = graph_create();
    CHECK(graph != NULL);

    uint32_t a = add_vertex(graph);

    CHECK(graph_add_edge(graph, a, a) == GRAPH_ERROR_LOOP);

    check_degree(graph, a, 0);

    graph_free(graph);
}

/* ---------- Test 6: isolated vertices ---------- */

static void test_isolated_vertices(void)
{
    graph_inst_t *graph = graph_create();
    CHECK(graph != NULL);

    uint32_t a = add_vertex(graph);
    uint32_t b = add_vertex(graph);
    uint32_t c = add_vertex(graph);

    CHECK(graph_add_edge(graph, a, b) == GRAPH_STATUS_OK);

    check_degree(graph, c, 0);

    const uint32_t *list = (const uint32_t *)1;
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
    graph_inst_t *graph = graph_create();
    CHECK(graph != NULL);

    uint32_t id = add_vertex(graph);

    CHECK(graph_add_vertex(NULL, &id) == GRAPH_ERROR_BAD_ARG);
    CHECK(graph_add_vertex(graph, NULL) == GRAPH_ERROR_BAD_ARG);

    CHECK(graph_add_edge(NULL, 0, 1) == GRAPH_ERROR_BAD_ARG);

    CHECK(graph_add_edge(graph, 0, 1)
          == GRAPH_ERROR_VERTEX_IS_NOT_EXIST);

    CHECK(graph_add_edge(graph, UINT32_MAX, 0)
          == GRAPH_ERROR_VERTEX_IS_NOT_EXIST);

    const uint32_t *list = NULL;
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
    graph_inst_t *graph = graph_create();
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
    graph_inst_t *graph = graph_create();
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
    graph_inst_t *g = graph_create();
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

    test_bfs_simple();
    test_bfs_shortest();

    puts("All BFS tests passed!");

    return EXIT_SUCCESS;
}
