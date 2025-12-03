#include "utils.h"
#include "GameState.h"
#include "search.h"
#include "constants.h"
#include <iostream>
#include <cstdint>
#include <array>
#include <chrono>
#include <random>

int nodes_visited = 0;

int main() {
    const std::array<std::array<uint8_t, 8>, 4> player_cards = {{
        {40, 11, 23, 52, 47, 14, 7, 33},
        {21, 22, 49, 10, 1, 36, 48, 27},
        {12, 24, 8, 38, 51, 20, 25, 37},
        {13, 35, 34, 50, 39, 26, 46, 9}
    }};

    for (int i = 0; i < 4; i++) {
        std::cout << "Player #" << i + 1 << "'s Cards:" << "\n";
        for (int j = 0; j < 8; j++) {
            uint8_t card = player_cards[i][j];
            std::cout << RANK_NAMES[get_rank(card)] << " of " << SUIT_NAMES[get_suit(card)] << "\n";
        }
        std::cout << "\n";
    }

    int depth = 32;

    GameState game_state(player_cards);
    auto start = std::chrono::high_resolution_clock::now();
    uint8_t score;
    for (int i = 4; i <= depth; i += 4) {
        score = minimax(game_state, i, 0, 130, true);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << "Time taken: " << duration.count() << " seconds" << std::endl;
    std::cout << "Score: " << +score << "\n";
    std::cout << "Total Nodes: " << nodes_visited << "\n";
    std::array<uint8_t, 32> pv = extract_pv(game_state, depth);

    for (int i = 0; i < depth; i++) {
        if (i % 4 == 0) std::cout << "\nHand #" << (i / 4) + 1 << ":\n";
        uint8_t card = pv[i];
        std::cout << RANK_NAMES[get_rank(card)] << " of " << SUIT_NAMES[get_suit(card)] << "\n";
    }

    return 0;
}