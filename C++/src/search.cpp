#include "search.h"
#include "utils.h"
#include "constants.h"
#include <array>
#include <cstdint>
#include <iostream>
#include <climits>
#include <fstream>

std::array<TTEntry, TABLE_SIZE> transposition_table;
std::array<std::array<uint8_t, 32>, 33> pv_table;
std::array<uint8_t, 33> pv_length;

extern uint32_t sample_id;

static void store_tt_entry(uint8_t trick_depth, TTEntry& entry, int eval, uint8_t best_move, uint64_t hash, int original_alpha, int original_beta) {
    if (trick_depth >= entry.trick_depth) {
        entry.score = eval;
        entry.trick_depth = trick_depth;
        entry.best_move = best_move;
        entry.hash = hash;
        entry.sample_id = sample_id;
        if (eval >= original_beta) entry.type = LOWER;
        else if (eval > original_alpha) {
            entry.type = EXACT;
        } else entry.type = UPPER;
    }
}

static void update_pv(uint8_t ply, uint8_t best_move) {
    pv_table[ply][0] = best_move;
    uint8_t child_len = pv_length[ply + 1];
    if (child_len > 0) {
        std::memcpy(&pv_table[ply][1], &pv_table[ply + 1][0], child_len * sizeof(uint8_t));
    }
    pv_length[ply] = child_len + 1;
}

int minimax(GameState& game_state, uint8_t trick_depth, int alpha, int beta, bool maximizing, uint8_t ply) {
    if (trick_depth == 0 || game_state.num_of_played_cards() == 28 || game_state.num_of_played_cards() == 32) {

        int eval = game_state.evaluate();

        if (game_state.num_of_played_cards() == 28) {
            pv_length[ply] = 4;
            
            std::array<uint8_t, 4> last_trick = game_state.last_trick();
            std::memcpy(pv_table[ply].data(), last_trick.data(), 4);
        } else {
            pv_length[ply] = 0;
        }

        return eval;
    }

    int original_alpha = alpha;
    int original_beta = beta;
    uint64_t hash = game_state.hash();

    std::array<uint8_t, 8> legal_moves;
    uint8_t num_moves = game_state.get_legal_moves(legal_moves);
    uint8_t best_move = legal_moves[0];

    pv_length[ply] = 0;

    TTEntry& entry = transposition_table[hash & (TABLE_SIZE - 1)];
    if (entry.hash == hash && entry.sample_id == sample_id) {
        if (game_state.num_of_played_cards() % 4 == 0 && entry.trick_depth >= trick_depth) {
            if (entry.type == TTType::LOWER) {
                if (entry.score >= beta) return entry.score;
                if (entry.score > alpha) alpha = entry.score;
            } else if (entry.type == TTType::UPPER) {
                if (entry.score <= alpha) return entry.score;
                if (entry.score < beta) beta = entry.score;
            } else return entry.score;
        }

        best_move = entry.best_move;

        for (uint8_t i = 1; i < num_moves; i++) {
            if (legal_moves[i] == best_move) {
                std::swap(legal_moves[0], legal_moves[i]);
                break;
            }
        }
    }

    if (maximizing) {
        int max_eval = -1;
        for (size_t i = 0; i < num_moves; i++) {
            uint8_t move = legal_moves[i];
            game_state.make_move(move);
            uint8_t next_player = game_state.current_player();
            bool maximizing_player = next_player == 0 || next_player == 2;
            uint8_t next_trick_depth = trick_depth - (game_state.num_of_played_cards() % 4 == 0 ? 1 : 0);
            int eval = minimax(game_state, next_trick_depth, alpha, beta, maximizing_player, ply + 1);
            game_state.undo_move();
            if (eval > max_eval) {
                max_eval = eval;
                best_move = move;
                update_pv(ply, best_move);
                if (max_eval > alpha) {
                    alpha = max_eval;
                    if (beta <= max_eval) break;
                }
            }
        }
        store_tt_entry(trick_depth, entry, max_eval, best_move, hash, original_alpha, original_beta);
        return max_eval;
    } else {
        int min_eval = 131;
        for (size_t i = 0; i < num_moves; i++) {
            uint8_t move = legal_moves[i];
            game_state.make_move(move);
            uint8_t next_player = game_state.current_player();
            bool maximizing_player = next_player == 0 || next_player == 2;
            uint8_t next_trick_depth = trick_depth - (game_state.num_of_played_cards() % 4 == 0 ? 1 : 0);
            int eval = minimax(game_state, next_trick_depth, alpha, beta, maximizing_player, ply + 1);
            game_state.undo_move();
            if (eval < min_eval) {
                min_eval = eval;
                best_move = move;
                update_pv(ply, best_move);
                if (min_eval < beta) {
                    beta = min_eval;
                    if (min_eval <= alpha) break;
                }
            }
        }
        store_tt_entry(trick_depth, entry, min_eval, best_move, hash, original_alpha, original_beta);
        return min_eval;
    }
}