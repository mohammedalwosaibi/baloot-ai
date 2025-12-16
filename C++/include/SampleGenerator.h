#pragma once

#include "GameState.h"
#include <array>
#include <cstdint>

class SampleGenerator {
public:
    SampleGenerator(const std::array<uint8_t, 8>& current_player_cards, int player_id);

private:
    std::array<uint8_t, 53> allowed_players_;
    std::array<uint8_t, 32> played_cards_;
    std::array<uint8_t, 8> current_player_cards_;
    std::array<std::array<uint8_t, 4>, 4> player_has_suit_;
    uint8_t player_id_;
    uint8_t current_player_;
};