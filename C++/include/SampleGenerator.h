#pragma once

#include <array>
#include <vector>
#include <cstdint>
#include <random>

class SampleGenerator {
public:
    SampleGenerator(const std::array<uint8_t, 8>& current_player_cards, uint8_t player_id);
    void generate_sample(std::array<std::array<uint8_t, 8>, 4>& sample);
    void play_card(uint8_t card, uint8_t player_id);

private:
    std::array<uint8_t, 8> current_player_cards_;
    std::array<uint8_t, 52> allowed_players_;
    std::array<uint8_t, 32> played_cards_;
    std::vector<uint8_t> remaining_cards_;
    uint8_t player_id_;
    uint8_t num_of_played_cards_;
    uint8_t trick_suit_;
    std::mt19937 rng_;
};