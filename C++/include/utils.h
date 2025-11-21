#pragma once

#include <span>
#include <cstdint>
#include <array>

uint8_t get_suit(const uint8_t card);
uint8_t get_rank(const uint8_t card);
bool has_suit(const std::array<uint8_t, 8>& cards, uint8_t suit);
uint8_t calculate_score(const std::array<uint8_t, 32>& round);
std::pair<uint8_t, uint8_t> get_trick_stats(std::span<const uint8_t> trick);