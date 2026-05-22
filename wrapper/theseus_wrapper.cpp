#include "theseus_wrapper.h"
#include <string>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <unordered_map>

#include "theseus/penalties.h"
#include "theseus/heuristics.h"
#include "theseus/alignment.h"
#include "theseus/graph.h"
#include "theseus/theseus_aligner_impl.h"

namespace {

theseus_result_t empty_result() {
    theseus_result_t res;
    res.valid = 0;
    res.score = 0;
    res.start_offset = 0;
    res.end_offset = 0;
    res.start_node_id = -1;
    res.cigar_string = nullptr;
    return res;
}

void add_gin_nodes_to_theseus(
    th_node_t *nodes,
    int num_nodes,
    theseus::Graph &graph,
    std::unordered_map<long long, theseus::Graph::NodeId> &gin_to_theseus
) {
    gin_to_theseus.reserve(num_nodes);

    for (int i = 0; i < num_nodes; ++i) {
        std::string clean_seq;
        clean_seq.reserve(nodes[i].seq_length);
        for (size_t j = 0; j < nodes[i].seq_length; ++j) {
            char c = nodes[i].seq[j];
            if (c != '\0' && c != '\n' && c != '\r') {
                clean_seq.push_back(c);
            }
        }

        auto t_id = graph.add_node(std::move(clean_seq));
        gin_to_theseus[nodes[i].id] = t_id;
    }
}

void add_gin_edges_to_theseus(
    th_edge_t *edges,
    int num_edges,
    theseus::Graph &graph,
    const std::unordered_map<long long, theseus::Graph::NodeId> &gin_to_theseus
) {
    for (int i = 0; i < num_edges; ++i) {
        auto src_it = gin_to_theseus.find(edges[i].src);
        auto dst_it = gin_to_theseus.find(edges[i].dst);
        if (src_it == gin_to_theseus.end() || dst_it == gin_to_theseus.end()) {
            continue;
        }
        graph.add_edge(src_it->second, dst_it->second);
    }
}

} // namespace

extern "C" {

theseus_result_t run_theseus_alignment(
    th_node_t* nodes, int num_nodes,
    th_edge_t* edges, int num_edges,
    const char* read_seq,
    long long start_node_id,
    int start_offset
) {
    th_start_t start;
    start.node_id = start_node_id;
    start.offset = start_offset;
    return run_theseus_alignment_batch(
        nodes, num_nodes, edges, num_edges, read_seq, &start, 1
    );
}

theseus_result_t run_theseus_alignment_batch(
    th_node_t* nodes, int num_nodes,
    th_edge_t* edges, int num_edges,
    const char* read_seq,
    th_start_t* starts,
    int num_starts
) {
    theseus_result_t best = empty_result();

    if (!nodes || num_nodes <= 0 || !read_seq || !starts || num_starts <= 0) {
        return best;
    }

    try {
        theseus::Graph graph;
        std::unordered_map<long long, theseus::Graph::NodeId> gin_to_theseus;

        add_gin_nodes_to_theseus(nodes, num_nodes, graph, gin_to_theseus);
        add_gin_edges_to_theseus(edges, num_edges, graph, gin_to_theseus);

        theseus::Penalties penalties(0, 5, 5, 1);
        theseus::Heuristics heuristics(false, false);
        theseus::TheseusAlignerImpl aligner(penalties, heuristics, std::move(graph), false);
        std::string read(read_seq);

        int best_score = std::numeric_limits<int>::max();

        for (int i = 0; i < num_starts; ++i) {
            auto start_it = gin_to_theseus.find(starts[i].node_id);
            if (start_it == gin_to_theseus.end()) {
                continue;
            }

            try {
                theseus::Graph::NodeId t_start_node = start_it->second;
                theseus::Alignment aln = aligner.align(read, t_start_node, starts[i].offset);
                int score = aln.compute_affine_gap_score(penalties);

                if (!best.valid || score < best_score) {
                    best_score = score;
                    best.score = score;
                    best.start_offset = aln.start_offset;
                    best.end_offset = aln.end_offset;
                    best.start_node_id = starts[i].node_id;
                    best.valid = 1;
                }
            } catch (const std::exception& e) {
                fprintf(stderr, "[THESEUS ERROR C++] Start non valido (%lld:%d): %s\n",
                        starts[i].node_id, starts[i].offset, e.what());
            }

            if (best.valid && best.score == 0) {
                break;
            }
        }
    } catch (const std::exception& e) {
        fprintf(stderr, "[THESEUS ERROR C++] Eccezione: %s\n", e.what());
    } catch (...) {
        fprintf(stderr, "[THESEUS ERROR C++] Errore fatale sconosciuto in Theseus!\n");
    }

    return best;
}

void free_theseus_result(theseus_result_t* res) {
    if (res && res->cigar_string) {
        free(res->cigar_string);
        res->cigar_string = nullptr;
    }
}

} // extern "C"
