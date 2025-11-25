#include <iostream>
#include <cstdint>
#include <array>
#include <tuple>
#include "utils.h"
#include "GameState.h"
#include "search.h"

int main() {
    const std::array<uint8_t, 32> round = {1, 24, 51, 35, 14, 22, 38, 7, 27, 48, 33, 12, 40, 25, 52, 11, 10, 9, 20, 34, 23, 46, 37, 50, 36, 13, 47, 21, 49, 39, 8, 26};

    const std::array<std::array<uint8_t, 8>, 4> player_cards = {{
        {1, 14, 27, 40, 9, 23, 36, 49},
        {24, 22, 48, 25, 20, 46, 13, 39},
        {51, 38, 33, 52, 10, 37, 47, 8},
        {35, 7, 12, 11, 34, 50, 21, 26}
    }};

    GameState game_state(player_cards);
    uint8_t score = minimax(game_state, 32, 0, 130, true);
    std::cout << +score << "\n";
    return 0;
}