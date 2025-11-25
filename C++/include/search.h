#pragma once

#include "GameState.h"
#include <cstdint>

uint8_t minimax(GameState& game_state, uint8_t depth, uint8_t alpha, uint8_t beta, bool maximizing);