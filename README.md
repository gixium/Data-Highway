# Data Highway

![Language](https://img.shields.io/badge/language-C%20%28GNU11%29-blue)
![Grade](https://img.shields.io/badge/grade-30%2F30-brightgreen)
![University](https://img.shields.io/badge/university-Politecnico%20di%20Milano-red)
![Year](https://img.shields.io/badge/year-2022-lightgrey)

Academic solo project for the *Algorithms and Data Structures* course at Politecnico di Milano.  
The program manages a simulated highway network and computes optimal routes between stations,
subject to constraints on vehicle range and fuel stops.

---

## Problem overview

A highway is modelled as an ordered sequence of **stations**, each identified by its distance
from the start in kilometres. Every station has a **parking lot** of up to 512 vehicles;
each vehicle is characterised by a single integer: its **range** (km it can travel on a full tank).

A journey from station A to station B uses only the vehicles available at each intermediate stop —
you swap to a new vehicle at every station you pass through. The goal is to find the route with
the **fewest stops**; when two routes have the same number of stops, the one that prefers stops
**closest to the departure end** wins.

The program reads a stream of commands from stdin and writes responses to stdout.

### Commands

| Command | Arguments | Effect |
|---|---|---|
| `aggiungi-stazione` | `<km> <n> <r₁> … <rₙ>` | Add a station at `km` with `n` vehicles of ranges `r₁…rₙ` |
| `demolisci-stazione` | `<km>` | Remove the station at `km` |
| `aggiungi-auto` | `<km> <range>` | Add a vehicle to the station at `km` |
| `rottama-auto` | `<km> <range>` | Remove a vehicle from the station at `km` |
| `pianifica-percorso` | `<start_km> <end_km>` | Print the optimal route, or `nessun percorso` |

---

## Architecture

The original single-file submission has been reorganised into three focused units:

| File | Role |
|---|---|
| [`src/highway.h`](src/highway.h) | Public interface: `Station` struct, constants, all function prototypes annotated with time/space complexity |
| [`src/highway.c`](src/highway.c) | Algorithm implementations: binary search, station CRUD, forward/backward path finding, path optimisation |
| [`src/main.c`](src/main.c) | Command-loop: reads from stdin, dispatches to `highway.c`, writes to stdout |

---

## Algorithms & complexity

*n* = number of stations on the highway at query time.

| Function | Algorithm | Time | Space |
|---|---|---|---|
| `find_station` | Binary search on sorted array | O(log n) | O(1) |
| `add_station` | Right-shift + `realloc` | O(n) | O(1) |
| `delete_station` | Left-shift + `realloc` | O(n) | O(1) |
| `first_reachable_forward` | Linear scan | O(n) | O(1) |
| `find_path_forward` | Greedy backward recursion | O(n²) worst | O(n) stack |
| `first_reachable_backward` | Linear scan | O(n) | O(1) |
| `find_path_backward` | Greedy backward recursion | O(n²) worst | O(n) stack |
| `path_feasible_within` | Bounded recursion | O(n · budget) | O(n) stack |
| `best_intermediate` | Scan + feasibility check | O(n²) per call | O(n) stack |
| `optimize_path` | Iterates waypoints | O(n³) worst | O(n) stack |

**Path-finding strategy.**  
For forward routes (A → B, A < B), `find_path_forward` works *backwards* from B: it finds the
leftmost station that can reach the current target, then recurses toward A. The greedy choice at
each step is correct because range is symmetric and we only care about hop count.  
For backward routes (A → B, A > B), `find_path_backward` finds the minimum-hop path first, then
`optimize_path` refines it by replacing waypoints with better (more leftward) alternatives when they
exist — implementing the tie-breaking rule without a full re-search.

---

## Data structures

- **`highway[]`** — a 1-indexed heap-allocated sorted array of `Station` structs. Index 0 is a
  permanent sentinel, which lets the binary search treat all valid indices as ≥ 1 without
  special-casing empty arrays. Insert and delete are O(n) due to shifting; this was acceptable
  given the problem's constraints.
- **`parking[]`** — a fixed-size `calloc`'d array of 512 `unsigned int` values per station.
  Zero encodes an empty slot. The maximum range across all slots is cached in `max_range` and
  updated incrementally to keep path-reachability checks at O(1) per station.
- **`path[]`** — a global array of `MAX_PATH_LEN` waypoints. The recursive path finders write
  into it in reverse order during stack unwinding; `main.c` reads it back-to-front for output.

---

## Build

```sh
cd src
make          # optimised build  (-O2, -Wall -Werror, gnu11)
make debug    # debug build      (-O0, AddressSanitizer, UBSanitizer)
make clean    # remove objects and binary
```

Requires GCC and GNU Make. Tested on Linux (x86-64) and macOS (Apple Silicon via GCC from Homebrew).

---

## Running tests

111 open test cases are included (input/output pairs).

```sh
cd test
chmod +x run_tests.sh

./run_tests.sh          # run all 111 cases
./run_tests.sh 1 2 3    # run a specific subset
```

Output:
```
Building...
Build OK.

PASS  test 1
PASS  test 2
...
Results: 111/111 passed, 0 failed
```

---

## Debugging

Build with debug symbols and sanitisers, then attach GDB:

```sh
make -C src debug

gdb src/highway
(gdb) break find_path_backward
(gdb) run < test/input/open_1.input.txt
(gdb) print *finish
(gdb) backtrace
(gdb) continue
```

Check for memory errors with Valgrind, or profile cache behaviour with Cachegrind:

```sh
valgrind --leak-check=full src/highway < test/input/open_1.input.txt
valgrind --tool=cachegrind  src/highway < test/input/open_1.input.txt
```

Reference programs used during development to learn these tools are in [`docs/ref/`](docs/ref/).

---

## Project structure

```
Data-Highway/
├── README.md
├── docs/
│   ├── specifications.pdf        # original assignment specification
│   └── ref/
│       ├── bug_collection.c      # sample program for ASAN / Valgrind practice
│       └── simple.c              # sample program for Cachegrind / perf practice
├── src/
│   ├── highway.h                 # public interface and data structures
│   ├── highway.c                 # algorithm implementations
│   ├── main.c                    # command-loop dispatcher
│   └── Makefile
└── test/
    ├── run_tests.sh              # automated regression runner
    ├── input/                    # 111 open test inputs
    └── output/                   # 111 expected outputs
```
