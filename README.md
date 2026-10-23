# algo_projects

A collection of small data-structure and algorithm projects written in **C**. The goal is to learn by implementing the underlying mechanisms, testing their invariants, and composing reusable components into more practical programs.

This is an educational repository, not a production-ready library distribution.

## Projects

| Directory | Component | Overview |
| --- | --- | --- |
| [`heap/`](heap/) | Binary heap | Array-backed priority heap with logarithmic insertion and removal. |
| [`addr_heap/`](addr_heap/) | Addressable heap | Binary heap with stable opaque handles for updating and removing arbitrary elements. |
| [`hash_table/`](hash_table/) | Hash table | `uint32_t` keys mapped to `void *` values, using open addressing, linear probing, tombstones, and resizing. |
| [`priority_queue/`](priority_queue/) | Priority queue | Combines an addressable max-heap and a hash table for ID-based lookup, priority updates, and removal. |

Each component lives in its own directory with a public header, implementation, and CMake configuration. The repository is organized to support standalone library targets and future projects.

## Repository layout

```text
algo_projects/
├── heap/
│   ├── heap.c
│   ├── heap.h
│   └── CMakeLists.txt
├── addr_heap/
│   ├── addr_heap.c
│   ├── addr_heap.h
│   └── CMakeLists.txt
├── hash_table/
│   ├── hash_table.c
│   ├── hash_table.h
│   └── CMakeLists.txt
├── priority_queue/
│   ├── prior_queue.c
│   ├── prior_queue.h
│   ├── main.c
│   └── CMakeLists.txt
├── CMakeLists.txt
└── README.md
```

## How the components fit together

```text
                  Priority Queue
                  (ID-based API)
                   /          \
                  /            \
       Addressable Heap      Hash Table
       (priority order)     (ID -> handle)
              |
         Heap handles
              |
        Private item record
         { ID, user data }

       Binary Heap (separate foundational implementation)
```

The priority queue keeps the data structures independent: the heap orders opaque items by priority, while the hash table locates their handles by ID. Moving an item within the heap updates its handle, without requiring changes to the hash table.

### Priority queue behavior

- Higher integer priorities are returned first.
- Equal-priority elements retain **FIFO** insertion order.
- Items have unique `uint32_t` identifiers.
- An existing item's priority can be updated without searching the entire heap.
- Arbitrary items can be removed by ID.
- User payloads are stored as `void *` pointers; **the caller owns the pointed-to objects**.

Public entry points are declared in [`priority_queue/prior_queue.h`](priority_queue/prior_queue.h):

```c
pri_queue_t *pri_queue_create(void);
void pri_queue_destroy(pri_queue_t *pq);
int pri_queue_add(pri_queue_t *pq, uint32_t id, int priority, void *data);
int pri_queue_set(pri_queue_t *pq, uint32_t id, int priority);
int pri_queue_remove(pri_queue_t *pq, uint32_t id);
int pri_queue_peek(pri_queue_t *pq, pq_item_t *out);
int pri_queue_pop(pri_queue_t *pq, pq_item_t *out);
size_t pri_queue_count(pri_queue_t *pq);
```

Operations that return `int` use `0` for success and nonzero values for errors. See the public headers and source for individual operation contracts.

### Example

```c
#include <stdio.h>
#include "prior_queue.h"

int main(void)
{
    int a = 42;
    int b = 99;
    pri_queue_t *pq = pri_queue_create();
    if (!pq)
        return 1;

    if (pri_queue_add(pq, 1, 10, &a) != 0 ||
        pri_queue_add(pq, 2, 20, &b) != 0) {
        pri_queue_destroy(pq);
        return 1;
    }

    pq_item_t item;
    while (pri_queue_pop(pq, &item) == 0)
        printf("id=%u priority=%d value=%d\n",
               (unsigned)item.id, item.priority, *(int *)item.data);

    pri_queue_destroy(pq);
    return 0;
}
```

The example assumes the `priority_queue/` include directory and relevant library target are supplied by your build.

## Complexity

For `n` stored entries, assuming average-case `O(1)` hash table operations:

| Priority queue operation | Expected time |
| --- | --- |
| Add | Amortized `O(log n)` |
| Change priority | `O(log n)` |
| Remove by ID | `O(log n)` |
| Peek | `O(1)` |
| Pop | `O(log n)` |
| Count | `O(1)` |

A heap adjustment is `O(log n)`. Hash-table probing can degrade to `O(n)` in the worst case; resizing can also make an individual operation `O(n)`.

## Build

Requirements:

- A C11-capable compiler (GCC or Clang recommended)
- CMake **3.12** or newer
- Optional: Ninja

From the repository root:

```sh
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

If Ninja is not installed, omit `-G Ninja` and use the generator available on your system.

The root `CMakeLists.txt` discovers immediate subdirectories containing their own `CMakeLists.txt` files. Individual components define their targets there. Inspect a component's CMake file or run the following command to list available targets:

```sh
cmake --build build --target help
```

## Testing

The project includes manually written C tests and integration checks rather than a third-party testing framework. Priority queue coverage includes:

- Empty queue and duplicate IDs
- Priority updates and arbitrary removals
- FIFO behavior with equal priorities
- Heap and hash table growth
- Repeated insertions/removals and draining the queue
- A deterministic randomized comparison against a simple reference model
- Destruction of a nonempty queue

During development, use **AddressSanitizer (ASan)** and **UndefinedBehaviorSanitizer (UBSan)** to catch memory and undefined-behavior errors. Leak checking can be enabled with:

```sh
ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 \
UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 \
./build/priority_queue/priority_queue
```

The executable path above is an example for a target named `priority_queue`; adjust it if the component's CMake target or output directory differs. Sanitizers must be enabled when building the executable.

## Design principles

- **Separation of concerns:** data structures remain independent of application-specific IDs and task execution.
- **Explicit ownership:** heap handles and internal queue wrappers have defined owners; user payloads are not freed by the queue.
- **Invariant-driven development:** use assertions for internal programming errors and error returns for expected failures.
- **Small, testable components:** focus on data structures and their behavior before adding application frameworks.

## Scope and limitations

These implementations are primarily for learning and experimentation. They are currently **single-threaded**, and the APIs and build layout may evolve. They are not intended as drop-in replacements for thoroughly tested production containers.

Future exercises can build upon the existing components rather than reimplementing every structure from scratch.
