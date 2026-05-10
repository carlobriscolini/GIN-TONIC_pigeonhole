#ifndef GIN_THESEUS_WRAPPER_H
#define GIN_THESEUS_WRAPPER_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    long long id;          
    const char* seq;       
    size_t seq_length;
} th_node_t;

typedef struct {
    long long src;         
    long long dst;         
} th_edge_t;

typedef struct {
    int score;             
    char* cigar_string;    
    int start_offset;      
    int end_offset;
    int valid;             
} theseus_result_t;

theseus_result_t run_theseus_alignment(
    th_node_t* nodes, int num_nodes,
    th_edge_t* edges, int num_edges,
    const char* read_seq, 
    long long start_node_id, 
    int start_offset
);

void free_theseus_result(theseus_result_t* res);

#ifdef __cplusplus
}
#endif

#endif // GIN_THESEUS_WRAPPER_H