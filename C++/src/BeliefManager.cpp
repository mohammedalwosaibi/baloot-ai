#include "BeliefManager.h"

BeliefManager::BeliefManager(const std::array<uint8_t, 8>& current_player_cards, uint8_t player_id) :
sample_generator_(current_player_cards, player_id),
rng_(std::random_device{}())
{
    particles_.resize(N);
    for (size_t i = 0; i < N; i++) {
        sample_generator_.generate_sample(particles_[i].world);
        particles_[i].weight = 1.0f / N;
    }
}

uint16_t BeliefManager::generate_sample(std::array<std::array<uint8_t, 8>, 4>& sample) {
    float sum = 0.0f;
    for (auto& p : particles_) sum += p.weight;

    if (sum <= 0.0f) {
        uint16_t i = rng_() % N;
        sample = particles_[i].world;
        return i;
    }

    std::uniform_real_distribution<float> uni(0.0f, sum);
    float r = uni(rng_);

    float acc = 0.0f;
    for (uint16_t i = 0; i < N; i++) {
        acc += particles_[i].weight;
        if (r <= acc) {
            sample = particles_[i].world;
            return i;
        }
    }

    sample = particles_[N - 1].world;
    return N - 1;
}

void BeliefManager::normalize() {
    float sum = 0.0f;
    for (auto& p : particles_) sum += p.weight;
    if (sum > 0.0f) for (auto& p : particles_) p.weight /= sum;
}

void BeliefManager::update_from_root_stats(uint8_t a_obs, const RootStats& stats) {
    uint32_t Ca = stats.C_a[a_obs];
    if (Ca == 0 || stats.total_iters == 0) return;

    float w = (float) Ca / (float) stats.total_iters;

    for (uint16_t d = 0; d < N; d++) {
        float post = (float) stats.C_ad[a_obs][d] / (float) Ca;
        particles_[d].weight = (1.0f - w) * particles_[d].weight + w * post;
    }

    normalize();
}