#include "utils.h"
#include "GameState.h"
#include "search.h"
#include <iostream>
#include <cstdint>
#include <array>
#include <chrono>
#include <random>

std::array<int, 8> nodes_visited = {};

int main() {
    const std::array<std::array<uint8_t, 8>, 4> player_cards = {{
        {40, 11, 23, 52, 47, 14, 7, 33},
        {21, 22, 49, 10, 1, 36, 48, 27},
        {12, 24, 8, 38, 51, 20, 25, 37},
        {13, 35, 34, 50, 39, 26, 46, 9}
    }};

    GameState game_state(player_cards);
    auto start = std::chrono::high_resolution_clock::now();
    uint8_t score = minimax(game_state, 32, 0, 130, true);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    std::cout << "Time taken: " << duration.count() << " seconds" << std::endl;
    std::cout << "Score: " << +score << "\n";
    int total_nodes = 0;
    for (int i = 0; i < 8; i++) {
        total_nodes += nodes_visited[i];
        std::cout << nodes_visited[i] << (i == 7 ? "\n" : " ");
    }
    std::cout << "Total Nodes: " << total_nodes << "\n";
    return 0;
}