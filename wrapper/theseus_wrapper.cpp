#include "theseus_wrapper.h"
#include <string>
#include <cstdlib>
#include <iostream>
#include <unordered_map>

#include "theseus/penalties.h"
#include "theseus/heuristics.h" 
#include "theseus/alignment.h"
#include "theseus/graph.h"
#include "theseus/theseus_aligner_impl.h"

extern "C" {

theseus_result_t run_theseus_alignment(
    th_node_t* nodes, int num_nodes,
    th_edge_t* edges, int num_edges,
    const char* read_seq, 
    long long start_node_id, 
    int start_offset
) {
    theseus_result_t res;
    res.valid = 0; res.score = 0; res.start_offset = 0; res.end_offset = 0; res.cigar_string = nullptr;

    try {
        theseus::Graph g;
        std::unordered_map<long long, theseus::Graph::NodeId> gin_to_theseus;
        gin_to_theseus.reserve(num_nodes); 

        for (int i = 0; i < num_nodes; ++i) {
            std::string clean_seq;
            clean_seq.reserve(nodes[i].seq_length);
            for(size_t j = 0; j < nodes[i].seq_length; ++j) {
                char c = nodes[i].seq[j];
                if (c != '\0' && c != '\n' && c != '\r') {
                    clean_seq.push_back(c);
                }
            }
            auto t_id = g.add_node(clean_seq);
            gin_to_theseus[nodes[i].id] = t_id;
        }

        for (int i = 0; i < num_edges; ++i) {
            auto src_t = gin_to_theseus.at(edges[i].src);
            auto dst_t = gin_to_theseus.at(edges[i].dst);
            g.add_edge(src_t, dst_t);
        }

        theseus::Penalties penalties(0, 5, 5, 1);
        theseus::Heuristics heuristics(false, false);
        
        theseus::TheseusAlignerImpl aligner(penalties, heuristics, std::move(g), false);
        
        std::string read(read_seq);
        theseus::Graph::NodeId t_start_node = gin_to_theseus.at(start_node_id);
        
        theseus::Alignment aln = aligner.align(read, t_start_node, start_offset);
        
        res.score = aln.compute_affine_gap_score(penalties);
        res.start_offset = aln.start_offset;
        res.end_offset = aln.end_offset;
        res.valid = 1;

    } catch (const std::exception& e) {
        fprintf(stderr, "[THESEUS ERROR C++] Eccezione: %s\n", e.what());
    } catch (...) {
        fprintf(stderr, "[THESEUS ERROR C++] Errore fatale sconosciuto in Theseus!\n");
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