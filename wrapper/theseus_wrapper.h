#ifndef GIN_THESEUS_WRAPPER_H
#define GIN_THESEUS_WRAPPER_H

#ifdef __cplusplus
extern "C" {
#endif

// struct in C per ottenere i risultati da theseus
typedef struct {
    int score;             // penalità (oss: 0 se 0 errori)
    char* cigar_string;    // CIGAR
    int start_offset;      // offset calcolato
    int end_offset;
    int valid;             // 1 se l'allineamento ha avuto successo, 0 altrimenti
} theseus_result_t;

// la funzione chiamata in gin_gin.c
theseus_result_t run_theseus_alignment(
    const char* gfa_string, 
    const char* read_seq, 
    const char* start_node_name, 
    int start_offset
);

void free_theseus_result(theseus_result_t* res);

#ifdef __cplusplus
}
#endif

#endif // GIN_THESEUS_WRAPPER_H