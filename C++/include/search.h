#pragma once

#include "GameState.h"
#include <cstdint>

enum TTType {UPPER, LOWER, EXACT};

struct TTEntry {
    uint8_t score;
    TTType type;
    uint8_t best_move;
    uint8_t trick_depth;
};

uint8_t minimax(GameState& game_state, uint8_t trick_depth, uint8_t alpha, uint8_t beta, bool maximizing);
std::vector<uint8_t> extract_pv(const GameState& root, uint8_t depth);