#include "SampleGenerator.h"
#include "utils.h"
#include <functional>
#include <cmath>
#include <algorithm>

SampleGenerator::SampleGenerator(const std::array<uint8_t, 8>& current_player_cards, int player_id) :
current_player_cards_(current_player_cards),
player_id_(player_id),
remaining_cards_{},
allowed_players_{},
played_cards_{},
num_of_played_cards_(0),
trick_suit_(-1),
rng_(std::random_device{}())
{
    for (int i = 1; i < 53; i++) allowed_players_[i] = 0b1111 & ~(1 << player_id_); // bit mask of 1111 for card i means it is allowed to be in any player's hand
    for (uint8_t card : current_player_cards_) if (card != 0) allowed_players_[card] = 1 << player_id_;
    for (int i = 1; i < 53; i++) {
        uint8_t rank = get_rank(i);
        if ((rank > 6 || rank == 1) && std::find(current_player_cards_.begin(), current_player_cards_.end(), i) == current_player_cards_.end()) remaining_cards_.push_back(i);
    }
}

bool SampleGenerator::generate_sample(std::array<std::array<uint8_t, 8>, 4>& sample) {
    for (auto& row : sample) row.fill(0);
    uint8_t tricks_completed = num_of_played_cards_ / 4;
    uint8_t cards_in_current_trick = num_of_played_cards_ % 4;

    std::array<uint8_t, 4> has = {0, 0, 0, 0};
    uint8_t initial_need = static_cast<uint8_t>(8 - tricks_completed);
    std::array<uint8_t, 4> need = {initial_need, initial_need, initial_need, initial_need};

    for (int i = 0; i < cards_in_current_trick; i++) {
        uint8_t player = (player_id_ - i + 3) % 4;
        need[player]--;
    }

    for (uint8_t card : current_player_cards_) if (card != 0) sample[player_id_][has[player_id_]++] = card;
    need[player_id_] = 0;

    std::shuffle(remaining_cards_.begin(), remaining_cards_.end(), rng_);

    auto feasible_count = [&](uint8_t card) {
        int count = 0;
        for (int p = 0; p < 4; p++)
            if (need[p] > 0 && (allowed_players_[card] & (1u << p))) count++;
        return count;
    };

    std::function<bool(int)> assign = [&](int idx) -> bool {
        if (idx == remaining_cards_.size()) return true;

        int best_j = -1;
        int best_count = 99;

        for (int j = idx; j < remaining_cards_.size(); j++) {
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

        for (int p = 0; p < 4; ++p) {
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

    return assign(0);
}

void SampleGenerator::play_card(uint8_t card, uint8_t player_id) {
    played_cards_[num_of_played_cards_] = card;
    
    if (player_id_ != player_id) {
        std::erase(remaining_cards_, card);
     } else {
        for (uint8_t& current_player_card : current_player_cards_) if (current_player_card == card) current_player_card = 0;
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

    num_of_played_cards_++;
}
