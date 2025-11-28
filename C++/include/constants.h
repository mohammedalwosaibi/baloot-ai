#pragma once

#include <array>
#include <cstdint>

extern const std::array<uint8_t, 14> RANK_VALUES;
extern const std::array<uint8_t, 14> RANK_ORDER;

inline constexpr uint8_t NUM_CARDS = 52;
inline constexpr uint8_t NUM_PLAYERS = 4;
inline constexpr uint8_t MAX_SCORE = 130;

extern uint64_t CARD_KEY[NUM_CARDS + 1];
extern uint64_t PLAYER_KEY[NUM_PLAYERS];
extern uint64_t SCORE_KEY[MAX_SCORE + 1];

void init_hashes();