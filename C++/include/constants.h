#pragma once

#include <array>
#include <cstdint>
#include <string_view>

inline constexpr uint8_t NO_CARD = 255;

inline constexpr std::array<uint8_t, 13> RANK_VALUES = {11, 0, 0, 0, 0, 0, 0, 0, 0, 10, 2, 3, 4};
inline constexpr std::array<uint8_t, 13> RANK_ORDER = {7, 0, 0, 0, 0, 0, 0, 1, 2, 6, 3, 4, 5};
inline constexpr std::array<std::string_view, 13> RANK_NAMES = {"A", "", "", "", "", "", "7", "8", "9", "10", "J", "Q", "K"};
inline constexpr std::array<std::string_view, 4> SUIT_SYMBOLS = {"♥", "♠", "♦", "♣"};

inline constexpr uint8_t NUM_CARDS = 52;
inline constexpr uint8_t NUM_PLAYERS = 4;
inline constexpr uint8_t MAX_SCORE = 130;

extern uint64_t CARD_KEY[NUM_CARDS];
extern uint64_t PLAYER_KEY[NUM_PLAYERS];
extern uint64_t SCORE_KEY[MAX_SCORE + 1];

void init_hashes();