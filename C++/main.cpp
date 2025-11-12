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

uint8_t get_suit(const uint8_t card) {
    return (card - 1) / 13; // integer division
}

uint8_t get_rank(const uint8_t card) {
    return card % 13;
}

uint8_t calculate_score(const std::array<uint8_t, 32>& round) {
    uint8_t winner = 0;
    for (int i = 0; i < 32; i += 4) {
        uint8_t round_card = round[i];
        uint8_t round_suit = get_suit(round_card);
        uint8_t max_rank = get_rank(round_card);
        for (int j = 1; j < 4; j++) {
            uint8_t cur_card = round[i + j];
            uint8_t cur_rank = get_rank(cur_card);
            if (round_suit == get_suit(cur_card) && get_rank(cur_card) > max_rank) {
                max_rank = cur_rank;
            }
        }
    }
    return 100;
}

int main() {
    const std::array<uint8_t, 32> round = {40, 27, 9, 34, 47, 24, 38, 8, 46, 22, 37, 52, 20, 26, 7, 12, 36, 11, 13, 23, 21, 33, 35, 1, 25, 48, 50, 10, 49, 14, 51, 39};
    calculate_score(round);
    return 0;
}