#pragma once

#include <cstdint>
#include <array>

uint8_t get_suit(const uint8_t card);
uint8_t get_rank(const uint8_t card);
bool has_suit(const std::array<uint8_t, 8>& cards, uint8_t suit);
uint8_t calculate_score(const std::array<uint8_t, 32>& round);
uint8_t get_trick_winner(const std::array<uint8_t, 4>& trick);