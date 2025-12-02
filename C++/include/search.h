#pragma once

#include "GameState.h"
#include <cstdint>

uint8_t minimax(GameState& game_state, uint8_t depth, uint8_t alpha, uint8_t beta, bool maximizing);
std::array<uint8_t, 32> extract_pv(const GameState& root, uint8_t depth);