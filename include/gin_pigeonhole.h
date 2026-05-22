#ifndef GIN_PIGEONHOLE_H
#define GIN_PIGEONHOLE_H

#include <gin_gin.h>
#include <stdint.h>
#include <stdio.h>

typedef struct {
    uint32_t *fwd_offsets;
    uint32_t *fwd_edges;
    uint32_t *bwd_offsets;
    uint32_t *bwd_edges;
    uint32_t *node_lengths;
    int_t num_nodes;
} gin_csr_graph_t;

typedef struct query_region {
    int_t qo;
    gin_string_t *subs;
    gin_vector_t *exact_matches;
} query_region_t;

typedef struct query_task_ {
    gin_string_t *str;
    gin_vector_t *regions_found;
} query_task_t;

typedef struct {
    double setup_time;
    double query_read_time;
    double seeding_time;
    double verification_time;
    double output_time;
    double total_execution_time;
} gin_pigeonhole_stats_t;

gin_csr_graph_t *build_csr_from_graph(gin_graph_t *graph);
void free_csr_graph(gin_csr_graph_t *csr);
void gin_gin_verify_task(
    gin_gin_t *gin, gin_graph_t *graph, gin_csr_graph_t *csr, gin_gin_decoder_t *dec,
    query_task_t *task, int_t max_matches, int_t max_errors, FILE *log_file
);

#endif
