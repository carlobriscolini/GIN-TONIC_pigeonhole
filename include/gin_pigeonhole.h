#ifndef GIN_PIGEONHOLE_H
#define GIN_PIGEONHOLE_H

#include <gin_gin.h>
#include <stdint.h>
#include <stdio.h>

/**
 * Compact graph representation used by the verification stage.
 *
 * Forward and backward adjacency lists are stored in CSR form so bounded graph
 * traversals around a seed can scan neighbors without repeated table lookups.
 * Node ids are expected to match positions in gin_graph_t::vertex_list.
 */
typedef struct {
    uint32_t *fwd_offsets;
    uint32_t *fwd_edges;
    uint32_t *bwd_offsets;
    uint32_t *bwd_edges;
    uint32_t *node_lengths;
    int_t num_nodes;
} gin_csr_graph_t;

/**
 * Seed interval selected from a query by the pigeonhole phase.
 *
 * qo is the seed offset in the full query. subs owns the query substring used
 * as an exact seed, and exact_matches owns the GIN fork intervals matching it.
 */
typedef struct query_region {
    int_t qo;
    gin_string_t *subs;
    gin_vector_t *exact_matches;
} query_region_t;

/**
 * Per-query state passed from seeding to verification.
 */
typedef struct query_task_ {
    gin_string_t *str;
    gin_vector_t *regions_found;
} query_task_t;

/**
 * Wall-clock timing counters collected by gin pigeonhole mode.
 */
typedef struct {
    double setup_time;
    double query_read_time;
    double seeding_time;
    double verification_time;
    double output_time;
    double total_execution_time;
} gin_pigeonhole_stats_t;

/**
 * Build and free the CSR helper structure used for approximate verification.
 */
gin_csr_graph_t *build_csr_from_graph(gin_graph_t *graph);
void free_csr_graph(gin_csr_graph_t *csr);

/**
 * Verify the exact seeds stored in task by aligning the full query against
 * local graph windows around each decoded seed occurrence.
 *
 * max_matches limits the number of decoded GIN hits per fork. max_errors adds
 * slack to the graph expansion distances and sets the start-offset search
 * window radius. If a valid alignment is found, the best Theseus result for
 * the query is written to log_file.
 */
void gin_gin_verify_task(
    gin_gin_t *gin, gin_graph_t *graph, gin_csr_graph_t *csr, gin_gin_decoder_t *dec,
    query_task_t *task, int_t max_matches, int_t max_errors, FILE *log_file
);

#endif
