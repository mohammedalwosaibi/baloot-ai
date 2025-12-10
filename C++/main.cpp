#include "utils.h"
#include "GameState.h"
#include "search.h"
#include "constants.h"
#include <iostream>
#include <cstdint>
#include <array>
#include <chrono>
#include <random>
#include <cmath>

extern std::array<std::array<uint8_t, 32>, 33> pv_table;
extern std::array<uint8_t, 33> pv_length;

int nodes_visited = 0;

/*
{14, 39, 1, 47, 26, 33, 52, 20},
{7, 25, 22, 50, 11, 46, 24, 49},
{36, 10, 35, 27, 12, 37, 23, 8},
{34, 38, 40, 48, 21, 51, 13, 9}
*/

int main() {
    const std::array<std::array<uint8_t, 8>, 4> player_cards = {{
        {40, 11, 23, 52, 47, 14, 7, 33},
        {21, 22, 49, 10, 1, 36, 48, 27},
        {12, 24, 8, 38, 51, 20, 25, 37},
        {13, 35, 34, 50, 39, 26, 46, 9}
    }};

    GameState game_state(player_cards);

    game_state.view_player_cards();

    auto start = std::chrono::high_resolution_clock::now();
    
    uint8_t score;
    for (int i = 1; i <= ceil((double) (32 - game_state.num_of_played_cards()) / 4); i++) {
        score = minimax(game_state, i, 0, 130, true, 0);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    for (int i = 0; i < pv_length[0]; i++) {
        if (i % 4 == 0) std::cout << "Hand #" << (i / 4) + 1 << ":\n";
        uint8_t card = pv_table[0][i];
        std::cout << RANK_NAMES[get_rank(card)] << SUIT_SYMBOLS[get_suit(card)] << (i % 4 == 3 ? "\n\n" : " ");
    }

    std::cout << "Time taken: " << duration.count() << " seconds" << std::endl;
    std::cout << "Score: " << +score << "\n";
    std::cout << "Total Nodes: " << nodes_visited << "\n";

    return 0;
}