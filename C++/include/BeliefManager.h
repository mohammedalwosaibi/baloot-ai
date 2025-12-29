#pragma once

#include "SampleGenerator.h"
#include <array>
#include <vector>
#include <random>

inline constexpr uint16_t N = 500;

struct Particle {
    std::array<std::array<uint8_t, 8>, 4> world;
    float weight;
};

struct RootStats {
    uint32_t total_iters = 0;
    std::array<uint32_t, 52> C_a{};
    std::array<std::array<uint16_t, N>, 52> C_ad{};
    void clear() {
        total_iters = 0;
        C_a.fill(0);
        for (auto& row : C_ad) row.fill(0);
    }
};

class BeliefManager {
public:
    BeliefManager(const std::array<uint8_t, 8>& current_player_cards, uint8_t player_id);
    void update_from_root_stats(uint8_t a_obs, const RootStats& stats);
    uint16_t generate_sample(std::array<std::array<uint8_t, 8>, 4>& sample);
private:
    SampleGenerator sample_generator_;
    std::vector<Particle> particles_;
    std::mt19937 rng_;

    void normalize();
};