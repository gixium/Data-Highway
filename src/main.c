/**
 * main.c — Data Highway command-loop dispatcher
 *
 * Reads commands from stdin and dispatches to the routines in highway.c.
 * All output is written to stdout, matching the protocol defined in the
 * project specification.
 *
 * Supported commands:
 *   aggiungi-stazione <km> <n> <r1> ... <rn>
 *   demolisci-stazione <km>
 *   aggiungi-auto <station_km> <range>
 *   rottama-auto <station_km> <range>
 *   pianifica-percorso <start_km> <end_km>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "highway.h"

int main(void) {
    char         cmd[20];
    unsigned int a, b, k, w;
    bool         route_found;

    /*
     * Initialise the highway array.  Index 0 is kept as an empty sentinel
     * so all valid station indices are >= 1, simplifying boundary checks
     * throughout the binary search.
     */
    highway = calloc(1, sizeof(Station));

    while (scanf("%19s", cmd) != EOF) {

        /* ── aggiungi-stazione ──────────────────────────────────────────── */
        if (!strcmp(cmd, "aggiungi-stazione")) {

            if (scanf("%u", &a) != 1) continue;

            k = find_station(a);
            if (k) {
                /* station already exists */
                printf("non aggiunta\n");
                continue;
            }

            if (scanf("%u", &b) != 1) continue;

            if (b > MAX_VEHICLES) {
                printf("non aggiunta\n");
                continue;
            }

            unsigned int pos = insert_idx;
            add_station(pos, a);

            for (unsigned int i = 0; i < b; i++) {
                unsigned int v;
                if (scanf("%u", &v) != 1) continue;
                highway[pos].parking[i] = v;
                if (v > highway[pos].max_range)
                    highway[pos].max_range = v;
            }
            printf("aggiunta\n");
        }

        /* ── demolisci-stazione ─────────────────────────────────────────── */
        else if (!strcmp(cmd, "demolisci-stazione")) {

            if (scanf("%u", &a) != 1) continue;

            k = find_station(a);
            if (!k)
                printf("non demolita\n");
            else {
                delete_station(a);
                printf("demolita\n");
            }
        }

        /* ── aggiungi-auto ──────────────────────────────────────────────── */
        else if (!strcmp(cmd, "aggiungi-auto")) {

            if (scanf("%u", &a) != 1) continue;

            k = find_station(a);
            if (!k) {
                printf("non aggiunta\n");
                continue;
            }

            /* find the first empty slot (value == 0) */
            unsigned int slot;
            for (slot = 0; slot < MAX_VEHICLES && highway[k].parking[slot] != 0; slot++) {}

            if (slot >= MAX_VEHICLES) {
                printf("non aggiunta\n");
                continue;
            }

            unsigned int v;
            if (scanf("%u", &v) != 1) continue;

            highway[k].parking[slot] = v;
            if (v > highway[k].max_range)
                highway[k].max_range = v;
            printf("aggiunta\n");
        }

        /* ── rottama-auto ───────────────────────────────────────────────── */
        else if (!strcmp(cmd, "rottama-auto")) {

            if (scanf("%u", &a) != 1) continue;

            k = find_station(a);
            if (!k) {
                printf("non rottamata\n");
                continue;
            }

            unsigned int v;
            if (scanf("%u", &v) != 1) continue;

            /* find the vehicle in the parking lot */
            unsigned int slot;
            for (slot = 0; slot < MAX_VEHICLES && highway[k].parking[slot] != v; slot++) {}

            if (slot >= MAX_VEHICLES) {
                printf("non rottamata\n");
                continue;
            }

            highway[k].parking[slot] = 0;
            printf("rottamata\n");

            /* if the scrapped car held the max_range, recompute it */
            if (v == highway[k].max_range) {
                unsigned int new_max = 0;
                for (unsigned int j = 0; j < MAX_VEHICLES; j++)
                    if (highway[k].parking[j] > new_max)
                        new_max = highway[k].parking[j];
                highway[k].max_range = new_max;
            }
        }

        /* ── pianifica-percorso ─────────────────────────────────────────── */
        else if (!strcmp(cmd, "pianifica-percorso")) {

            if (scanf("%u %u", &a, &b) != 2) continue;

            if (a == b) {
                printf("%u\n", a);
                continue;
            }

            if (a < b) {
                /* forward route: increasing km */
                k = find_station(a);
                w = find_station(b);

                if (w == k + 1) {
                    /* adjacent stations: one direct hop check */
                    if (highway[k].km + highway[k].max_range >= highway[w].km)
                        printf("%u %u\n", a, b);
                    else
                        printf("nessun percorso\n");
                } else {
                    path_start  = &highway[k];
                    route_found = find_path_forward(&highway[w], 0);

                    if (!route_found) {
                        printf("nessun percorso\n");
                    } else {
                        printf("%u ", a);
                        path_len--;
                        for (w = path_len; w != 0; w--)
                            printf("%u ", path[w]);
                        printf("%u\n", path[0]);
                    }

                    route_found = false;
                    path_start  = NULL;
                    path_len    = 0;
                }
            }

            else {
                /* backward route: decreasing km */
                k = find_station(a);
                w = find_station(b);

                if (w == k - 1) {
                    /* adjacent stations: one direct hop check */
                    if (highway[k].max_range >= highway[k].km ||
                        highway[k].km - highway[k].max_range <= highway[w].km)
                        printf("%u %u\n", a, b);
                    else
                        printf("nessun percorso\n");
                } else {
                    path_start  = &highway[k];
                    route_found = find_path_backward(&highway[w], 0);

                    if (!route_found) {
                        printf("nessun percorso\n");
                    } else {
                        path_len--;
                        optimize_path(path_len);

                        printf("%u ", a);
                        for (w = path_len; w != 0; w--)
                            printf("%u ", path[w]);
                        printf("%u\n", path[0]);
                    }

                    route_found = false;
                    path_start  = NULL;
                    path_len    = 0;
                }
            }
        }
    }

    return 0;
}
