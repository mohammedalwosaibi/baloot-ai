#pragma once

#include "GameState.h"
#include <cstdint>
#include <vector>

enum TTType {UPPER, LOWER, EXACT};

struct TTEntry {
    int score;
    TTType type;
    uint8_t best_move;
    uint8_t trick_depth;
    uint64_t hash;
};

int minimax(GameState& game_state, uint8_t trick_depth, int alpha, int beta, bool maximizing, uint8_t ply);