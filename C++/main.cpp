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

int nodes_visited = 0;

int main() {
    const std::array<std::array<uint8_t, 8>, 4> player_cards = {{
        {40, 11, 23, 52, 47, 14, 7, 33},
        {21, 22, 49, 10, 1, 36, 48, 27},
        {12, 24, 8, 38, 51, 20, 25, 37},
        {13, 35, 34, 50, 39, 26, 46, 9}
    }};

    int depth = 8;

    GameState game_state(player_cards);

    game_state.view_player_cards();

    auto start = std::chrono::high_resolution_clock::now();
    uint8_t score;
    for (int i = 1; i <= depth; i++) {
        score = minimax(game_state, i, 0, 130, true);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    std::vector<uint8_t> pv = extract_pv(game_state, 32);

    for (int i = 0; i < 32 - game_state.num_of_played_cards(); i++) {
        if (i % 4 == 0) std::cout << "Hand #" << (i / 4) + 1 << ":\n";
        uint8_t card = pv[i];
        std::cout << RANK_NAMES[get_rank(card)] << SUIT_SYMBOLS[get_suit(card)] << (i % 4 == 3 ? "\n\n" : " ");
    }

    std::cout << "Time taken: " << duration.count() << " seconds" << std::endl;
    std::cout << "Score: " << +score << "\n";
    std::cout << "Total Nodes: " << nodes_visited << "\n";

    return 0;
}