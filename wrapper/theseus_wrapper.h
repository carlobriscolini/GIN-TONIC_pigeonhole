#ifndef GIN_THESEUS_WRAPPER_H
#define GIN_THESEUS_WRAPPER_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Node exported from GIN to the Theseus C++ graph. */
typedef struct {
    long long id;          
    const char* seq;       
    size_t seq_length;
} th_node_t;

/** Directed edge exported from GIN to the Theseus C++ graph. */
typedef struct {
    long long src;         
    long long dst;         
} th_edge_t;

/** Candidate alignment start in GIN node coordinates. */
typedef struct {
    long long node_id;
    int offset;
} th_start_t;

/** Best alignment returned by the Theseus wrapper. */
typedef struct {
    int score;             
    char* cigar_string;    
    int start_offset;      
    int end_offset;
    long long start_node_id;
    int valid;             
} theseus_result_t;

/**
 * Align read_seq from a single candidate start.
 *
 * This is a convenience wrapper around run_theseus_alignment_batch(). The input
 * node, edge, and read arrays are borrowed for the duration of the call and
 * remain owned by the caller.
 */
theseus_result_t run_theseus_alignment(
    th_node_t* nodes, int num_nodes,
    th_edge_t* edges, int num_edges,
    const char* read_seq, 
    long long start_node_id, 
    int start_offset
);

/**
 * Build one temporary Theseus graph and align read_seq from several starts.
 *
 * The lowest-penalty valid alignment is returned. Invalid starts are skipped,
 * and an invalid result is returned if no candidate can be aligned. The graph
 * data passed by the caller is copied into the temporary Theseus graph.
 */
theseus_result_t run_theseus_alignment_batch(
    th_node_t* nodes, int num_nodes,
    th_edge_t* edges, int num_edges,
    const char* read_seq,
    th_start_t* starts,
    int num_starts
);

/** Free heap-owned fields inside a theseus_result_t. */
void free_theseus_result(theseus_result_t* res);

#ifdef __cplusplus
}
#endif

#endif // GIN_THESEUS_WRAPPER_H