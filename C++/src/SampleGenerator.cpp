#include "SampleGenerator.h"
#include "utils.h"
#include "constants.h"
#include <functional>
#include <cmath>
#include <algorithm>
#include <iostream>

SampleGenerator::SampleGenerator(const std::array<uint8_t, 8>& current_player_cards, uint8_t player_id) :
current_player_cards_(current_player_cards),
allowed_players_{},
played_cards_{},
played_count_{},
remaining_cards_{},
player_id_(player_id),
num_of_played_cards_(0),
rng_(std::random_device{}())
{
    for (size_t i = 0; i < 52; i++) allowed_players_[i] = 0b1111 & ~(1 << player_id_); // bit mask of 1111 for card i means it is allowed to be in any player's hand
    for (uint8_t card : current_player_cards_) if (card != NO_CARD) allowed_players_[card] = static_cast<uint8_t>(1u << player_id_);
    for (uint8_t i = 0; i < 52; i++) {
        uint8_t rank = get_rank(i);
        if ((rank > 5 || rank == 0) && std::find(current_player_cards_.begin(), current_player_cards_.end(), i) == current_player_cards_.end()) remaining_cards_.push_back(i);
    }
}

void SampleGenerator::generate_sample(std::array<std::array<uint8_t, 8>, 4>& sample) {
    for (auto& row : sample) row.fill(NO_CARD);

    std::array<uint8_t, 4> has = {0, 0, 0, 0};
    std::array<uint8_t, 4> need = {8, 8, 8, 8};

    for (size_t i = 0; i < 4; i++) {
        need[i] -= played_count_[i];
    }

    for (uint8_t card : current_player_cards_) if (card != NO_CARD) sample[player_id_][has[player_id_]++] = card;
    need[player_id_] = 0;

    std::shuffle(remaining_cards_.begin(), remaining_cards_.end(), rng_);

    auto feasible_count = [&](uint8_t card) {
        int count = 0;
        for (uint8_t p = 0; p < 4; p++) {
            if (need[p] > 0 && (allowed_players_[card] & (1u << p))) count++;
        }
        return count;
    };

    std::function<bool(size_t)> assign = [&](size_t idx) -> bool {
        if (idx == remaining_cards_.size()) return true;

        size_t best_j = idx;
        int best_count = INT_MAX;

        for (size_t j = idx; j < remaining_cards_.size(); j++) {
            int count = feasible_count(remaining_cards_[j]);
            if (count == 0) return false;
            if (count < best_count) {
                best_count = count;
                best_j = j;
                if (best_count == 1) break;
            }
        }

        std::swap(remaining_cards_[idx], remaining_cards_[best_j]);

        uint8_t card = remaining_cards_[idx];

        std::array<uint8_t,4> players = {0,1,2,3};
        std::shuffle(players.begin(), players.end(), rng_);

        for (uint8_t p : players) {
            if (need[p] == 0) continue;
            if (!(allowed_players_[card] & (1u << p))) continue;

            sample[p][has[p]++] = card;
            need[p]--;

            if (assign(idx + 1)) return true;

            need[p]++;
            has[p]--;
        }

        return false;
    };


    assign(0);
}

void SampleGenerator::play_card(uint8_t card, uint8_t player_id) {
    played_cards_[num_of_played_cards_] = card;
    
    if (player_id_ != player_id) {
        std::erase(remaining_cards_, card);
     } else {
        for (uint8_t& current_player_card : current_player_cards_) if (current_player_card == card) current_player_card = NO_CARD;
    }

    
    if (num_of_played_cards_ % 4 != 0) {
        if (get_suit(card) != trick_suit_) {
            for (uint8_t c : remaining_cards_) {
                if (get_suit(c) == trick_suit_) {
                    allowed_players_[c] &= ~(1u << player_id);
                }
            }
        }
    } else {
        trick_suit_ = get_suit(card);
    }

    played_count_[player_id]++;
    num_of_played_cards_++;
}
