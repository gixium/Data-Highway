/**
 * highway.h — Data Highway public interface
 *
 * Defines the Station data structure, shared global state, and prototypes
 * for all station-management and path-finding routines.
 *
 * Naming convention
 *   n = number of stations currently on the highway
 *   p = number of waypoints in the route being planned
 */

#ifndef HIGHWAY_H
#define HIGHWAY_H

#include <stdbool.h>

/* ── Constants ───────────────────────────────────────────────────────────── */

/** Maximum number of vehicles per station parking lot. */
#define MAX_VEHICLES 512

/** Maximum number of waypoints that can be stored in path[]. */
#define MAX_PATH_LEN 65536

/* ── Data structures ─────────────────────────────────────────────────────── */

/**
 * Station — one stop on the highway.
 *
 * @km        Distance from the highway start (kilometre marker).
 * @max_range Range of the longest-range car currently in the parking lot.
 *            A car with range r parked at km d can reach any station
 *            between (d - r) and (d + r).
 * @parking   Heap-allocated array of MAX_VEHICLES vehicle ranges.
 *            Unused slots hold 0.
 */
typedef struct {
    unsigned int  km;
    unsigned int  max_range;
    unsigned int *parking;
} Station;

/* ── Global state ────────────────────────────────────────────────────────── */
/*
 * The highway is stored as a 1-indexed sorted array.  Index 0 is a
 * permanently empty sentinel so that all valid indices are >= 1.
 *
 * Global state is used to pass context into recursive helpers without
 * threading extra parameters through every call — a deliberate trade-off
 * to keep the recursive signatures simple.
 */

/** Sorted array of stations; reallocated on every insert/delete. */
extern Station      *highway;

/** Number of stations currently stored (valid range: highway[1..last_idx]). */
extern unsigned int  last_idx;

/**
 * Set by find_station() on a miss: the index at which the queried km
 * would be inserted to maintain sorted order.
 */
extern unsigned int  insert_idx;

/** Waypoints of the current route, stored in reverse order during recursion. */
extern unsigned int  path[MAX_PATH_LEN];

/** Anchor station for the ongoing path search (start or end, direction-dependent). */
extern Station      *path_start;

/** Step counter incremented by the recursive path finders; equals path length on return. */
extern unsigned int  path_len;

/* ── Station management ──────────────────────────────────────────────────── */

/**
 * find_station() — binary search on the sorted highway array.
 *
 * On a hit  : returns the index of the station with the given km.
 * On a miss : returns 0 and sets insert_idx to the slot where km would go.
 *
 * Time  O(log n)   Space  O(1)
 */
unsigned int find_station(unsigned int km);

/**
 * add_station() — insert a new station at the given sorted position.
 *
 * Reallocates highway[], shifts existing stations one slot to the right,
 * and initialises the new station with an empty calloc'd parking lot.
 *
 * @param pos  Insertion index (typically the insert_idx set by find_station).
 * @param km   Kilometre marker of the new station.
 *
 * Time  O(n)   Space  O(1) amortised (excluding parking allocation)
 */
void add_station(unsigned int pos, unsigned int km);

/**
 * delete_station() — remove the station at the given kilometre marker.
 *
 * Frees the parking lot, shifts the array left, and reallocates.
 * Does nothing if the station does not exist.
 *
 * @param km  Kilometre marker of the station to remove.
 *
 * Time  O(n)   Space  O(1)
 */
void delete_station(unsigned int km);

/* ── Forward path finding (left → right) ────────────────────────────────── */

/**
 * first_reachable_forward() — leftmost station between path_start and x
 * whose best car can reach km x.
 *
 * @param x  Target kilometre.
 * @return   Index of the qualifying station; 0 if none exists.
 *
 * Time  O(n)   Space  O(1)
 */
unsigned int first_reachable_forward(unsigned int x);

/**
 * find_path_forward() — greedy recursive path finder, travelling left→right.
 *
 * Works backwards from finish toward path_start: at each step it finds the
 * leftmost station that can reach the current target, then recurses.
 * Waypoints are stored in path[] in reverse order as the recursion unwinds.
 *
 * @param finish  Destination station.
 * @param depth   Recursion depth (call with 0).
 * @return        true if a valid path from path_start to finish exists.
 *
 * Time  O(n²) worst case   Space  O(n) stack
 */
bool find_path_forward(Station *finish, unsigned int depth);

/* ── Backward path finding (right → left) ───────────────────────────────── */

/**
 * first_reachable_backward() — rightmost station between path_start and x
 * whose best car can reach km x (travelling in the decreasing-km direction).
 *
 * @param x  Target kilometre.
 * @return   Index of the qualifying station; 0 if none exists.
 *
 * Time  O(n)   Space  O(1)
 */
unsigned int first_reachable_backward(unsigned int x);

/**
 * find_path_backward() — greedy recursive path finder, travelling right→left.
 *
 * Mirrors find_path_forward for the reverse direction.
 * Also increments path_len so the caller knows how many hops were taken.
 *
 * @param finish  Destination station.
 * @param depth   Recursion depth (call with 0).
 * @return        true if a valid path from path_start to finish exists.
 *
 * Time  O(n²) worst case   Space  O(n) stack
 */
bool find_path_backward(Station *finish, unsigned int depth);

/**
 * path_feasible_within() — bounded feasibility check for the right→left direction.
 *
 * Like find_path_backward but stops early (returns false) once the hop
 * budget is exhausted.  Used during path optimisation to prune candidates.
 *
 * @param finish  Candidate station to reach.
 * @param budget  Maximum additional hops allowed.
 * @return        true if finish is reachable within budget hops.
 *
 * Time  O(n · budget)   Space  O(budget) stack
 */
bool path_feasible_within(Station *finish, unsigned int budget);

/* ── Path optimisation ───────────────────────────────────────────────────── */

/**
 * best_intermediate() — for two consecutive path[] waypoints, find whether
 * a better (more leftward) intermediate station exists.
 *
 * "Better" means closer to the start, while still allowing the remaining
 * leg of the route to be completed.
 *
 * @param path_idx    Index into path[] of the current waypoint.
 * @param hop_budget  Remaining-hop budget passed to path_feasible_within.
 * @return            km of the better intermediate; 0 if none found.
 *
 * Time  O(n²) per call   Space  O(n) stack
 */
unsigned int best_intermediate(unsigned int path_idx, unsigned int hop_budget);

/**
 * optimize_path() — scan all intermediate waypoints in path[] and replace
 * each one with a better station if available.
 *
 * Called once after find_path_backward() to prefer leftward stops
 * (the tie-breaking rule: when hop counts are equal, prefer the route
 * that uses the stations closest to the starting end).
 *
 * @param hops  Number of intermediate waypoints (= path_len after the initial search).
 *
 * Time  O(n³) worst case   Space  O(n) stack
 */
void optimize_path(unsigned int hops);

#endif /* HIGHWAY_H */
