#include "theseus_wrapper.h"
#include <sstream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "theseus/theseus_aligner.h"
#include "theseus/penalties.h"
#include "theseus/alignment.h"

extern "C" {

theseus_result_t run_theseus_alignment(
    const char* gfa_string, 
    const char* read_seq, 
    const char* start_node_name, 
    int start_offset
) {
    // struttura che dovrà tornare al codice C per portare i risultati da C++ a C
    theseus_result_t res;

    // inizializzazione della struttura in uno stato di default
    res.valid = 0; 
    res.score = 0; 
    res.start_offset = 0; 
    res.end_offset = 0; 
    res.cigar_string = nullptr;

    try {

        // approccio 1: scrivo su fiel

        // scrivo il GFA su un file temporaneo
        std::ofstream out_gfa("debug_graph.gfa");
        out_gfa << gfa_string;
        out_gfa.close();

        // faccio leggere a theseus il file fisico
        std::ifstream in_gfa("debug_graph.gfa");

        // approccio 2: uso una stringstream per incapsulare la gfa_string direttamente in memoria
        // std::istringstream in_gfa(gfa_string);

        // APPROCCIO 2 È PIÙ LENTO, COME È POSSIBILE?

        // configurazione delle penalità: 0 = match, 5 = mismatch, 5 = gap open, 1 = gap extend
        theseus::Penalties penalties(0, 5, 5, 1);
        
        // aligner carica il grafo dal file e prepara le matrici di calcolo
        theseus::TheseusAligner aligner(penalties, in_gfa);
        
        std::string start_node(start_node_name);
        std::string read(read_seq);
        
        theseus::Alignment aln = aligner.align(read, start_node, start_offset);
        
        // estrazione risultati
        res.score = aln.compute_affine_gap_score(penalties);
        res.start_offset = aln.start_offset;
        res.end_offset = aln.end_offset;
        res.valid = 1;

        // catch di possibili errori trovati da theseus durante l'allineamento
    } catch (const std::exception& e) {
        printf("[THESEUS ERROR C++] Eccezione catturata: %s\n", e.what());
    } catch (...) {
        printf("[THESEUS ERROR C++] Errore fatale sconosciuto in Theseus!\n");
    }

    return res;
}

void free_theseus_result(theseus_result_t* res) {
    if (res && res->cigar_string) {
        free(res->cigar_string);
        res->cigar_string = nullptr;
    }
}

} // extern "C"