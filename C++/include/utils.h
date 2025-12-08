#pragma once

#include <cstdint>
#include <array>

constexpr uint8_t get_suit(const uint8_t card) {
    return (card - 1) / 13;
}

constexpr uint8_t get_rank(const uint8_t card) {
    return ((card - 1) % 13) + 1;
}

bool has_suit(const std::array<uint8_t, 8>& cards, uint8_t suit);
std::pair<uint8_t, uint8_t> get_trick_stats(const uint8_t* trick);
uint64_t random_hash();