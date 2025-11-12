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

uint8_t calculate_score(const std::array<uint8_t, 32>& round) {
    uint8_t winner = 0;
    for (int i = 0; i < 32; i += 4) {
        uint8_t round_suit = (round[i] - 1) / 13; // integer division

    }
    return 100;
}

int main() {
    const std::array<uint8_t, 32> round = {40, 27, 9, 34, 47, 24, 38, 8, 46, 22, 37, 52, 20, 26, 7, 12, 36, 11, 13, 23, 21, 33, 35, 1, 25, 48, 50, 10, 49, 14, 51, 39};
    calculate_score(round);
    return 0;
}