/**
 * highway.c — Data Highway algorithm implementations
 *
 * Station management (binary search, insert, delete) and path-finding
 * routines for the highway route planner.
 */

#include <stdlib.h>
#include <stdbool.h>

#include "highway.h"

/* ── Global state definitions ────────────────────────────────────────────── */

Station      *highway    = NULL;
unsigned int  last_idx   = 0;
unsigned int  insert_idx = 0;

unsigned int  path[MAX_PATH_LEN] = {0};
Station      *path_start = NULL;
unsigned int  path_len   = 0;

/* ══════════════════════════════════════════════════════════════════════════
 * Station management
 * ══════════════════════════════════════════════════════════════════════════ */

unsigned int find_station(unsigned int km) {
    unsigned int left = 1, right = last_idx, mid;

    /* empty highway */
    if (right == 0) {
        insert_idx = 1;
        return 0;
    }

    /* single station — exact match */
    if (left == right && highway[left].km == km)
        return left;

    /* single station — no match: advise insertion before or after */
    if (left == right && highway[left].km != km) {
        insert_idx = (highway[left].km < km) ? 2 : 1;
        return 0;
    }

    while (left <= right) {
        /* check current boundaries before computing midpoint */
        if (highway[left].km  == km) return left;
        if (highway[right].km == km) return right;

        mid = (left + right) / 2;

        if (highway[mid].km == km)
            return mid;

        if (highway[mid].km < km) {
            /* km is in the right half */
            if (mid == last_idx) {
                /* km belongs past the last station */
                insert_idx = last_idx + 1;
                return 0;
            }
            left = mid + 1;
            if (highway[left].km > km) {
                /* gap found: km belongs between mid and mid+1 */
                insert_idx = left;
                return 0;
            }
        } else {
            /* km is in the left half */
            if (mid == 1) {
                /* km belongs before the first station */
                insert_idx = 1;
                return 0;
            }
            right = mid - 1;
            if (highway[right].km < km) {
                /* gap found: km belongs between mid-1 and mid */
                insert_idx = mid;
                return 0;
            }
        }
    }

    return 0;
}

/* ─────────────────────────────────────────────────────────────────────────── */

void add_station(unsigned int pos, unsigned int km) {
    last_idx++;
    highway = realloc(highway, (last_idx + 1) * sizeof(Station));

    /* shift existing stations one slot to the right to open pos */
    for (unsigned int i = last_idx - 1, j = last_idx; i >= pos; i--, j--)
        highway[j] = highway[i];

    /* initialise the new slot */
    highway[pos].km        = km;
    highway[pos].max_range = 0;
    highway[pos].parking   = calloc(MAX_VEHICLES, sizeof(unsigned int));
}

/* ─────────────────────────────────────────────────────────────────────────── */

void delete_station(unsigned int km) {
    unsigned int k = find_station(km);
    if (!k)
        return;

    free(highway[k].parking);

    /* shift everything one slot to the left */
    for (unsigned int i = k, j = k + 1; i < last_idx; i++, j++)
        highway[i] = highway[j];

    last_idx--;
    highway = realloc(highway, (last_idx + 1) * sizeof(Station));
}

/* ══════════════════════════════════════════════════════════════════════════
 * Forward path finding  (left → right, increasing km)
 * ══════════════════════════════════════════════════════════════════════════ */

unsigned int first_reachable_forward(unsigned int x) {
    unsigned int dest = find_station(x);
    unsigned int cur  = find_station(path_start->km);

    /* scan left-to-right until a station whose car reaches x is found */
    for (; highway[cur].km + highway[cur].max_range < x && cur < dest; cur++) {}

    if (highway[cur].km + highway[cur].max_range >= x && cur < dest)
        return cur;

    return 0;
}

/* ─────────────────────────────────────────────────────────────────────────── */

bool find_path_forward(Station *finish, unsigned int depth) {
    /* base case: we have walked back to the start */
    if (finish->km == path_start->km)
        return true;

    if (finish->km > path_start->km) {
        unsigned int x = first_reachable_forward(finish->km);
        if (!x)
            return false;

        bool found = find_path_forward(&highway[x], depth + 1);
        path_len++;

        if (found) {
            /* record this waypoint as the recursion unwinds */
            path[depth] = finish->km;
            return true;
        }
    }

    return false;
}

/* ══════════════════════════════════════════════════════════════════════════
 * Backward path finding  (right → left, decreasing km)
 * ══════════════════════════════════════════════════════════════════════════ */

unsigned int first_reachable_backward(unsigned int x) {
    unsigned int dest     = find_station(x);
    unsigned int cur      = find_station(path_start->km);
    unsigned int min_reach;

    /* minimum km reachable from the current station */
    min_reach = (highway[cur].km > highway[cur].max_range)
                    ? highway[cur].km - highway[cur].max_range
                    : 0;

    /* scan right-to-left until a station that can reach x is found */
    while (min_reach > x && cur != dest) {
        cur--;
        min_reach = (highway[cur].km > highway[cur].max_range)
                        ? highway[cur].km - highway[cur].max_range
                        : 0;
    }

    if (min_reach <= x && cur != dest)
        return cur;

    return 0;
}

/* ─────────────────────────────────────────────────────────────────────────── */

bool find_path_backward(Station *finish, unsigned int depth) {
    /* base case: we have walked forward to the (rightmost) start */
    if (finish->km == path_start->km)
        return true;

    if (finish->km < path_start->km) {
        unsigned int x = first_reachable_backward(finish->km);
        if (!x)
            return false;

        bool found = find_path_backward(&highway[x], depth + 1);
        path_len++;

        if (found) {
            path[depth] = finish->km;
            return true;
        }
    }

    return false;
}

/* ─────────────────────────────────────────────────────────────────────────── */

bool path_feasible_within(Station *finish, unsigned int budget) {
    if (finish->km == path_start->km)
        return true;

    if (finish->km < path_start->km) {
        unsigned int x = first_reachable_backward(finish->km);

        if (!x)          return false;
        if (budget == 0) return false;

        return path_feasible_within(&highway[x], budget - 1);
    }

    return false;
}

/* ══════════════════════════════════════════════════════════════════════════
 * Path optimisation
 * ══════════════════════════════════════════════════════════════════════════ */

unsigned int best_intermediate(unsigned int path_idx, unsigned int hop_budget) {
    unsigned int prev_idx = find_station(path[path_idx - 1]);
    unsigned int cur_idx  = find_station(path[path_idx]);
    unsigned int min_reach;

    /* stations are adjacent in the highway array — nothing to improve */
    if (cur_idx == prev_idx + 1)
        return 0;

    /*
     * Scan every station strictly between prev and cur.
     * The first one that (a) can reach the previous waypoint and
     * (b) allows a feasible path within the remaining budget is preferred
     * because it is further left — the tie-breaking rule.
     */
    for (unsigned int i = prev_idx + 1; i != cur_idx; i++) {
        min_reach = (highway[i].km > highway[i].max_range)
                        ? highway[i].km - highway[i].max_range
                        : 0;

        if (min_reach <= highway[prev_idx].km)
            if (path_feasible_within(&highway[i], hop_budget))
                return highway[i].km;
    }

    return 0;
}

/* ─────────────────────────────────────────────────────────────────────────── */

void optimize_path(unsigned int hops) {
    unsigned int candidate;
    unsigned int remaining = hops;

    for (unsigned int i = 1; i <= hops; i++) {
        candidate = best_intermediate(i, remaining--);
        if (candidate)
            path[i] = candidate;
    }
}
