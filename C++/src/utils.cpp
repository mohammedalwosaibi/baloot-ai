#include "utils.h"
#include "constants.h"

uint8_t get_suit(const uint8_t card) {
    return (card - 1) / 13; // integer division
}

uint8_t get_rank(const uint8_t card) {
    return ((card - 1) % 13) + 1;
}

bool has_suit(const std::array<uint8_t, 8>& cards, uint8_t suit) {
    for (uint8_t card : cards) {
        if (get_suit(card) == suit) {
            return true;
        }
    }
    return false;
}

uint8_t get_trick_winner(const std::array<uint8_t, 4>& trick) {
    uint8_t trick_suit = get_suit(trick[0]);
    uint8_t max_rank = get_rank(trick[0]);
    uint8_t winner = 0;

    for (int i = 1; i < 4; i++) {
        uint8_t cur_rank = get_rank(trick[i]);
        if (get_suit(trick[i]) == trick_suit && RANK_ORDER.at(cur_rank) > RANK_ORDER.at(max_rank)) {
            max_rank = cur_rank;
            winner = i;
        }
    }
    
    return winner;
}

uint8_t calculate_score(const std::array<uint8_t, 32>& round) {
    uint8_t winner = 0;
    uint8_t round_score = 0;
    for (int i = 0; i < 32; i += 4) {
        uint8_t trick_card = round[i];
        uint8_t trick_suit = get_suit(trick_card);
        uint8_t max_rank = get_rank(trick_card);
        uint8_t trick_winner = 0;
        uint8_t trick_score = RANK_VALUES.at(max_rank);
        for (int j = 1; j < 4; j++) {
            uint8_t cur_card = round[i + j];
            uint8_t cur_rank = get_rank(cur_card);
            trick_score += RANK_VALUES.at(cur_rank);
            if (trick_suit == get_suit(cur_card) && RANK_ORDER.at(cur_rank) > RANK_ORDER.at(max_rank)) {
                max_rank = cur_rank;
                trick_winner = j;
            }
        }
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