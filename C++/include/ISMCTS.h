#pragma once

#include "GameState.h"
#include "SampleGenerator.h"
#include <cstdint>
#include <array>
#include <vector>

struct Node {
    uint32_t visits = 0;
    uint32_t availability = 0;
    uint64_t total_score = 0;
    std::array<uint32_t, 32> child_indices{};

    float ucb1() const {
        if (visits == 0) return std::numeric_limits<float>::infinity();

        float exploitation = static_cast<float>(total_score) / static_cast<float>(visits);
        float exploration = 184 * std::sqrt(std::log(static_cast<float>(availability)) / static_cast<float>(visits));

        return exploitation + exploration;
    }
};

class ISMCTS {
public:
    ISMCTS(const std::array<uint8_t, 8>& current_player_cards, uint8_t player_id);
    void play_card(uint8_t card, uint8_t player_id);
    void run(double max_duration);
    uint8_t best_move();
    void set_player_cards(std::array<std::array<uint8_t, 8>, 4>& sample);
    void set_current_player(uint8_t player_id);
    GameState game_state_;
private:
    std::array<uint8_t, 8> current_player_cards_;
    std::vector<Node> nodes_;
    SampleGenerator sample_generator_;
    std::mt19937 rng_;
    std::array<std::array<std::array<uint32_t, 52>, 4>, 16> mast_visits;
    std::array<std::array<std::array<uint64_t, 52>, 4>, 16> mast_total_score;

    void randomize_cards();
    uint8_t pick_rollout_move(uint8_t acting_player, uint8_t bucket, const std::array<uint8_t,8>& moves, uint8_t num_moves);
};