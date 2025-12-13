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
#include <fstream>

extern std::array<std::array<uint8_t, 32>, 33> pv_table;
extern std::array<uint8_t, 33> pv_length;

extern std::array<TTEntry, 1 << 20> transposition_table;

extern int nodes_visited;

int main() {
    std::random_device rd;
    std::mt19937 gen(rd());

    std::array<uint8_t, 24> deck = {36, 37, 1, 12, 14, 40, 26, 22, 27, 10, 49, 7, 48, 47, 38, 25, 9, 39, 51, 8, 33, 34, 50, 11};

    std::unordered_map<uint8_t, double> votes;

    std::array<std::array<uint8_t, 8>, 4> player_cards = {{
        {21, 23, 13, 20, 24, 46, 52, 35},
        {36, 37, 1, 12, 14, 40, 26, 22},
        {27, 10, 49, 7, 48, 47, 38, 25},
        {9, 39, 51, 8, 33, 34, 50, 11}
    }};

    constexpr int ITERS = 1000;

    auto start = std::chrono::high_resolution_clock::now();

    for (int _ = 0; _ < ITERS; _++) {
        for (auto& e : transposition_table) e.hash = 0;
        
        std::shuffle(deck.begin(), deck.end(), gen);

        for (int i = 1; i < 4; i++) {
            for (int j = 0; j < 8; j++) {
                player_cards[i][j] = deck[(i - 1) * 8 + j];
            }
        }

        GameState game_state(player_cards);

        // game_state.view_player_cards();

        uint8_t score;
        for (int i = 1; i <= ceil((double) (32 - game_state.num_of_played_cards()) / 4) - 4; i++) {
            score = minimax(game_state, i, 0, 130, true, 0);
        }

        for (int i = 0; i < 8; i++) {
            uint8_t card = player_cards[0][i];
            game_state.make_move(card);
            votes[card] += minimax(game_state, 4, 0, 130, false, 1);
            game_state.undo_move();
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    std::cout << "Time Taken: " << duration.count() << " seconds\n";

    std::vector<std::pair<uint8_t, double>> sorted_votes;
    sorted_votes.reserve(votes.size());

    for (auto& kv : votes) {
        kv.second /= ITERS;
        sorted_votes.push_back(kv);
    }

    std::sort(sorted_votes.begin(), sorted_votes.end(),
            [](const auto& a, const auto& b) {
                return a.second > b.second;
            });

    for (const auto& [card, count] : sorted_votes) {
        std::cout << RANK_NAMES[get_rank(card)]
                << SUIT_SYMBOLS[get_suit(card)]
                << ": " << count << "\n";
    }

    // std::array<std::array<uint8_t, 8>, 4> player_cards = {{
    //     {21, 23, 13, 20, 24, 46, 52, 35},
    //     {36, 37, 1, 12, 14, 40, 26, 22},
    //     {27, 10, 49, 7, 48, 47, 38, 25},
    //     {9, 39, 51, 8, 33, 34, 50, 11}
    // }};

    // GameState game_state(player_cards);

    // game_state.view_player_cards();

    // auto start = std::chrono::high_resolution_clock::now();

    // uint8_t score;
    // for (int i = 1; i <= ceil((double) (32 - game_state.num_of_played_cards()) / 4) - 1; i++) {
    //     score = minimax(game_state, i, 0, 130, false, 0);
    // }
    
    // auto end = std::chrono::high_resolution_clock::now();
    // std::chrono::duration<double> duration = end - start;

    // for (int i = 0; i < pv_length[0]; i++) {
    //     if (i % 4 == 0) std::cout << "Hand #" << (i / 4) + 1 << ":\n";
    //     uint8_t card = pv_table[0][i];
    //     std::cout << RANK_NAMES[get_rank(card)] << SUIT_SYMBOLS[get_suit(card)] << (i % 4 == 3 ? "\n\n" : " ");
    // }

    // std::cout << "Time taken: " << duration.count() << " seconds" << std::endl;
    // std::cout << "Score: " << +score << "\n";
    // std::cout << "Total Nodes: " << nodes_visited << "\n";

    return 0;
}