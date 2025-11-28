#include "utils.h"
#include "constants.h"
#include <random>

uint8_t get_suit(const uint8_t card) {
    return (card - 1) / 13; // integer division
}

uint8_t get_rank(const uint8_t card) {
    return ((card - 1) % 13) + 1;
}

bool has_suit(const std::array<uint8_t, 8>& cards, uint8_t suit) {
    for (uint8_t card : cards) {
        if (card != 0 && get_suit(card) == suit) {
            return true;
        }
    }
    return false;
}

std::pair<uint8_t, uint8_t> get_trick_stats(std::span<const uint8_t> trick) {
    uint8_t trick_suit = get_suit(trick[0]);
    uint8_t max_rank = get_rank(trick[0]);
    uint8_t winner = 0;
    uint8_t score = RANK_VALUES[max_rank];

    for (int i = 1; i < 4; i++) {
        uint8_t cur_rank = get_rank(trick[i]);
        score += RANK_VALUES[cur_rank];
        if (get_suit(trick[i]) == trick_suit && RANK_ORDER[cur_rank] > RANK_ORDER[max_rank]) {
            max_rank = cur_rank;
            winner = i;
        }
    }
    
    return {winner, score};
}

uint8_t calculate_score(const std::array<uint8_t, 32>& round) {
    uint8_t winner = 0;
    uint8_t round_score = 0;
    for (int i = 0; i < 32; i += 4) {
        auto [trick_winner, trick_score] = get_trick_stats(std::span(round).subspan(i, 4));

        winner = (winner + trick_winner) % 4;
        if (winner == 0 || winner == 2) {
            round_score += trick_score;
        }
    }
    if (winner == 0 || winner == 2) {
        round_score += 10;
    }
    return round_score;
}

uint64_t random_hash() {
    return (uint64_t(rand()) << 33) ^ (uint64_t(rand()) << 2) ^ (uint64_t(rand()) >> 29);
}