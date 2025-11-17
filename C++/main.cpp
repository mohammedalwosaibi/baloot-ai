#include <iostream>
#include <cstdint>
#include <array>
#include <tuple>
#include "utils.h"

void make_move(std::array<std::array<uint8_t, 8>, 4>& player_cards, uint8_t player, uint8_t card) {
    for (int i = 0; i < 8; i++) {
        if (player_cards[player][i] == card) {
            player_cards[player][i] = 0;
            return;
        }
    }
}

int main() {
    const std::array<uint8_t, 32> round = {1, 24, 51, 35, 14, 22, 38, 7, 27, 48, 33, 12, 40, 25, 52, 11, 10, 9, 20, 34, 23, 46, 37, 50, 36, 13, 47, 21, 49, 39, 8, 26};

    std::array<std::array<uint8_t, 8>, 4> player_cards = {{
        {1, 14, 27, 40, 10, 23, 36, 49},
        {24, 22, 48, 25, 9, 46, 13, 39},
        {51, 38, 33, 52, 20, 37, 47, 8},
        {35, 7, 12, 11, 34, 50, 21, 26}
    }};

    make_move(player_cards, 0, 14);
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            std::cout << +player_cards[i][j] << " ";
        }
        std::cout << std::endl;
    }
    return 0;
}