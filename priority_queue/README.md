# Priority Queue (C11)

A single-threaded priority queue with efficient lookup, priority updates, and removal by user-defined ID. Written in C11 as an educational data-structures project.

The implementation combines a **binary max-heap** with an **open-addressed hash table**, while keeping both components reusable. It also includes a standalone **addressable heap** that supports direct access to elements through opaque handles.

> Educational implementation, not a production-hardened library.

## Features

- Highest `int` priority is served first.
- **Stable FIFO ordering** for elements with equal priority, based on original insertion order.
- Unique `uint32_t` IDs with fast lookup and updates.
- `ADD`, `SET`, `REMOVE`, `PEEK`, `POP`, `COUNT` operations.
- Opaque internal types, dynamically growing storage, explicit resource ownership.
- No third-party runtime dependencies.

The queue stores arbitrary data pointers. It does **not** execute tasks or manage threads.

## Architecture

```text
                     Priority Queue
                  (ID-based public API)
                     /           \
                    /             \
           Addressable Heap     Hash Table
           priority / FIFO      ID -> handle*
                    \             /
                     \___________/
                    stable handles
```

- **`addr_heap.[ch]`** implements a binary max-heap whose elements can be accessed, reprioritized, or removed through opaque handles. A handle tracks the owning heap, an up-to-date array index, and whether the element is active. Sifting updates the index stored in each affected handle.
- **`hash_table.[ch]`** implements open addressing with linear probing, tombstones, and rehashing. It maps `uint32_t` keys to `void *` values.
- **`prior_queue.[ch]`** connects these structures: `ID -> handle*` in the table, and a private `{ id, user_data }` record stored as opaque data in the heap. The heap never needs to know what an ID is.
- **`heap.[ch]`** retains the initial, non-addressable binary heap implementation.

### Handle lifetime

The standalone addressable-heap API transfers ownership of each newly created handle to its caller:

- `addr_heap_add_item()` returns a handle for the inserted element.
- `addr_heap_peek_item()` and `addr_heap_get_item()` do not invalidate it.
- `addr_heap_pop()` and `addr_heap_remove_item()` invalidate the removed element's handle.
- `addr_heap_deallocate()` invalidates remaining active handles, but does **not** free them.
- `addr_heap_free_handler()` frees an **inactive** handle; freeing an active handle is rejected.

Using a handle after freeing it violates the API contract. Users of the higher-level priority queue do **not** manage handles; the queue owns and releases them internally.

## Public API

Declared in [`prior_queue.h`](prior_queue.h):

```c
#include <stdint.h>
#include <stddef.h>

typedef struct pri_queue pri_queue_t;

typedef struct {
    uint32_t id;
    int priority;
    void *data;
} pq_item_t;

pri_queue_t *pri_queue_create(void);
void pri_queue_destroy(pri_queue_t *pq);

int pri_queue_add(pri_queue_t *pq, uint32_t id, int priority, void *data);
int pri_queue_set(pri_queue_t *pq, uint32_t id, int priority);
int pri_queue_remove(pri_queue_t *pq, uint32_t id);
int pri_queue_peek(pri_queue_t *pq, pq_item_t *out);
int pri_queue_pop(pri_queue_t *pq, pq_item_t *out);
size_t pri_queue_count(pri_queue_t *pq);
```

`PEEK` observes the highest-priority element; `POP` removes it. `SET` changes an existing element's priority while preserving its original insertion sequence. Duplicate IDs are rejected.

### Example

```c
#include <stdio.h>
#include "prior_queue.h"

int main(void)
{
    int a = 100, b = 200;
    pri_queue_t *pq = pri_queue_create();
    if (!pq) return 1;

    if (pri_queue_add(pq, 10, 5, &a) != 0 ||
        pri_queue_add(pq, 20, 10, &b) != 0 ||
        pri_queue_set(pq, 10, 15) != 0) {
        pri_queue_destroy(pq);
        return 1;
    }

    pq_item_t item;
    while (pri_queue_pop(pq, &item) == 0) {
        printf("id=%u priority=%d value=%d\n",
               (unsigned)item.id, item.priority, *(int *)item.data);
    }

    pri_queue_destroy(pq);
    return 0;
}
```

### Memory ownership

The queue owns its private records and heap handles, including for items still queued when `pri_queue_destroy()` is called. It **does not own** the objects referenced by user-supplied `data` pointers. The caller must keep those objects alive while queued and release them as appropriate. The current `pri_queue_add()` implementation rejects `data == NULL`.

### Return codes

The current implementation uses `0` for success and nonzero codes for failure. Common codes include:

| Code | Meaning |
| --- | --- |
| `0` | Success |
| `-1` | Invalid argument (where checked) |
| `-2` | Empty queue on `POP` |
| `-3` | Memory allocation failure |
| `-50` | ID not found, or empty queue on `PEEK` |
| `-200` | Duplicate ID on `ADD` |

The public API does not yet define a shared status enum; callers should consistently check success with `ret == 0`.

## Time complexity

For `n` queued elements:

| Operation | Expected time |
| --- | --- |
| `ADD` | Amortized **O(log n)** |
| `SET` | **O(log n)** |
| `REMOVE` | **O(log n)** |
| `PEEK` | **O(1)** |
| `POP` | **O(log n)** |
| `COUNT` | **O(1)** |

Hash-table lookups and removals are **O(1) on average**, not guaranteed in the worst case. A resizing operation may take **O(n)**; heap sifting costs **O(log n)**.

## Build

Requires a C11 compiler and **CMake 3.10+**. The included build currently produces a test/demo executable rather than an installed library.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/priority_queue
```

For Ninja, add `-G Ninja` to the configuration command and install Ninja if necessary.

On Unix with GCC/Clang, the supplied `CMakeLists.txt` currently enables AddressSanitizer and UndefinedBehaviorSanitizer:

```sh
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 \
./build/priority_queue
```

## Tests

`main.c` contains tests for the heap, hash table, and their integration. Integration scenarios cover empty and nonempty queues, duplicates, priority changes, FIFO, removal, growth, repeated ID usage, and nonempty destruction. A randomized sequence of **10,000 operations** is compared against a simple linear-search reference model.

Sanitizers help detect invalid memory accesses and leaks on exercised paths; they are not a guarantee of correctness. Allocation-failure injection is not currently part of the test suite.

## Limitations

- Single-threaded; concurrent callers require external synchronization.
- Queue contents are pointers, not copies of user objects.
- The hash table may degrade to linear-time searches under unfavorable collision patterns.
- Insertion-sequence counter wraparound is not handled for effectively unbounded operation counts.
- The repository does not yet provide an installable library target or a stable public error-code enum.

## Files

| File | Role |
| --- | --- |
| `prior_queue.[ch]` | ID-based priority queue |
| `addr_heap.[ch]` | Addressable binary max-heap |
| `hash_table.[ch]` | Open-addressed hash table |
| `heap.[ch]` | Original binary heap |
| `main.c` | Tests and demonstration |
| `CMakeLists.txt` | Build configuration |

---

A learning project focused on data structures, invariants, memory ownership, and practical C API design.
