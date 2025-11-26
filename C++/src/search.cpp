#include "search.h"
#include "utils.h"
#include <array>

extern std::array<uint64_t, 8> nodes_visited;

int8_t current_move = -1;

uint8_t minimax(GameState& game_state, uint8_t depth, uint8_t alpha, uint8_t beta, bool maximizing) {
    if (depth == 0) {
        return calculate_score(game_state.played_cards());
    }

    if (depth == 31) {
        current_move++;
    } else if (depth != 32) {
        nodes_visited[current_move] += 1;
    }

    if (maximizing) {
        uint8_t max_eval = 0;
        for (uint8_t move : game_state.get_legal_moves()) {
            game_state.make_move(move);
            uint8_t eval = minimax(game_state, depth - 1, alpha, beta, false);
            game_state.undo_move();
            if (eval > max_eval) {
                max_eval = eval;
                if (max_eval > alpha) {
                    alpha = max_eval;
                    if (beta <= alpha) break;
                }
            }
        }
        return max_eval;
    } else {
        uint8_t min_eval = 130;
        for (uint8_t move : game_state.get_legal_moves()) {
            game_state.make_move(move);
            uint8_t eval = minimax(game_state, depth - 1, alpha, beta, true);
            game_state.undo_move();
            if (eval < min_eval) {
                min_eval = eval;
                if (min_eval < beta) {
                    beta = min_eval;
                    if (beta <= alpha) break;
                }
            }
        }
        return min_eval;
    }
}

