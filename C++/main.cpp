#include <iostream>
#include <cstdint>
#include <array>
#include <tuple>
#include <unordered_map>

const std::unordered_map<uint8_t, uint8_t> RANK_VALUES = {
    {1, 11},
    {7, 0},
    {8, 0},
    {9, 0},
    {10, 10},
    {11, 2},
    {12, 3},
    {13, 4},
};

const std::unordered_map<uint8_t, uint8_t> RANK_ORDER = {
    {7, 0},
    {8, 1},
    {9, 2},
    {11, 3},
    {12, 4},
    {13, 5},
    {10, 6},
    {1, 7},
};

uint8_t get_suit(const uint8_t card) {
    return (card - 1) / 13; // integer division
}

uint8_t get_rank(const uint8_t card) {
    return ((card - 1) % 13) + 1;
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

int main() {
    const std::array<uint8_t, 32> round = {1, 24, 51, 35, 14, 22, 38, 7, 27, 48, 33, 12, 40, 25, 52, 11, 10, 9, 20, 34, 23, 46, 37, 50, 36, 13, 47, 21, 49, 39, 8, 26};
    std::cout << +calculate_score(round) << std::endl;
    return 0;
}