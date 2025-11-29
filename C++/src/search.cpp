#include "search.h"
#include "utils.h"
#include <array>
#include <cstdint>
#include <unordered_map>
#include <iostream>

std::unordered_map<uint64_t, uint8_t> transposition_table;

extern std::array<int, 8> nodes_visited;

int8_t current_move = -1;

uint8_t minimax(GameState& game_state, uint8_t depth, uint8_t alpha, uint8_t beta, bool maximizing) {
    uint8_t original_alpha = alpha;
    uint8_t original_beta = beta;
    uint64_t hash = game_state.hash();
    if (depth == 0) {
        return game_state.score();
    }

    if (depth == 31) {
        current_move++;
    } else if (depth != 32) {
        nodes_visited[current_move] += 1;
    }

    
    if (depth % 4 == 0) {
        std::unordered_map<uint64_t, uint8_t>::iterator it = transposition_table.find(hash);
        if (it != transposition_table.end()) {
            return it->second;
        }
    }

    if (maximizing) {
        uint8_t max_eval = 0;
        std::array<uint8_t, 8> legal_moves;
        uint8_t num_moves = game_state.get_legal_moves(legal_moves);
        for (int i = 0; i < num_moves; i++) {
            uint8_t move = legal_moves[i];
            game_state.make_move(move);
            uint8_t eval = minimax(game_state, depth - 1, alpha, beta, false);
            game_state.undo_move();
            if (eval > max_eval) {
                max_eval = eval;
                if (max_eval > alpha) {
                    alpha = max_eval;
                    if (beta <= max_eval) break;
                }
            }
        }
        if (depth % 4 == 0 && original_alpha < max_eval && max_eval < original_beta) transposition_table[hash] = max_eval;
        return max_eval;
    } else {
        uint8_t min_eval = 130;
        std::array<uint8_t, 8> legal_moves;
        uint8_t num_moves = game_state.get_legal_moves(legal_moves);
        for (int i = 0; i < num_moves; i++) {
            uint8_t move = legal_moves[i];
            game_state.make_move(move);
            uint8_t eval = minimax(game_state, depth - 1, alpha, beta, true);
            game_state.undo_move();
            if (eval < min_eval) {
                min_eval = eval;
                if (min_eval < beta) {
                    beta = min_eval;
                    if (min_eval <= alpha) break;
                }
            }
        }
        if (depth % 4 == 0 && original_alpha < min_eval && min_eval < original_beta) transposition_table[hash] = min_eval;
        return min_eval;
    }
}

// No TT: 66,898,419
// TT Exact Scores: 51,031,819
// TT Exact Scores (Inclusive): 26,522,770
// TT All Hits: 1,769,405