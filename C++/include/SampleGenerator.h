#pragma once

#include "GameState.h"
#include <array>
#include <vector>
#include <cstdint>
#include <random>

class SampleGenerator {
public:
    SampleGenerator(const std::array<uint8_t, 8>& current_player_cards, int player_id);
    bool generate_sample(std::array<std::array<uint8_t, 8>, 4>& sample);
    void play_card(uint8_t card, uint8_t player_id);

private:
    std::array<uint8_t, 53> allowed_players_;
    std::array<uint8_t, 32> played_cards_;
    std::vector<uint8_t> remaining_cards_;
    std::array<uint8_t, 8> current_player_cards_;
    uint8_t player_id_;
    uint8_t num_of_played_cards_;
    uint8_t trick_suit_;
    uint8_t current_player;
    std::mt19937 rng_;
};