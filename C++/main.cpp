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

static constexpr int TRIALS = 100;

int main() {
    std::random_device rd;
    std::mt19937 gen(rd());

    std::array<double, 5> success{};

    for (int t = 0; t < 200; t++) {
        std::cout << "Trial #" << t << "\n";

        std::unordered_map<uint8_t, double> votes;

        std::array<uint8_t, 32> deck = {21, 23, 13, 20, 24, 46, 52, 35, 36, 37, 1, 12, 14, 40, 26, 22, 27, 10, 49, 7, 48, 47, 38, 25, 9, 39, 51, 8, 33, 34, 50, 11};

        std::shuffle(deck.begin(), deck.end(), gen);

        std::array<std::array<uint8_t, 8>, 4> player_cards{};

        std::array<uint8_t, 24> opponent_cards{};

        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 8; j++) {
                player_cards[i][j] = deck[i * 8 + j];
                if (i != 0) opponent_cards[(i - 1) * 8 + j] = deck[i * 8 + j];
            }
        }

        auto start = std::chrono::high_resolution_clock::now();
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        
        int iterations = 0;

        while (duration.count() < 30) {
            for (auto& e : transposition_table) e.hash = 0;
            
            std::shuffle(opponent_cards.begin(), opponent_cards.end(), gen);

            for (int i = 1; i < 4; i++) {
                for (int j = 0; j < 8; j++) {
                    player_cards[i][j] = opponent_cards[(i - 1) * 8 + j];
                }
            }

            GameState game_state(player_cards);

            for (int i = 0; i < 8; i++) {
                uint8_t card = player_cards[0][i];
                game_state.make_move(card);
                votes[card] += minimax(game_state, 4, -131, 131, false, 1);
                game_state.undo_move();
            }

            end = std::chrono::high_resolution_clock::now();
            duration = end - start;
            iterations++;
        }

        std::vector<std::pair<uint8_t, double>> sorted_votes_benchmark;
        sorted_votes_benchmark.reserve(votes.size());

        for (auto& kv : votes) {
            kv.second /= iterations;
            sorted_votes_benchmark.push_back(kv);
        }

        std::sort(sorted_votes_benchmark.begin(), sorted_votes_benchmark.end(),
                [](const auto& a, const auto& b) {
                    return a.second > b.second;
                });

        for (int depth = 1; depth <= 5; depth++) {
            start = std::chrono::high_resolution_clock::now();
            end = std::chrono::high_resolution_clock::now();
            duration = end - start;
            iterations = 0;

            while (duration.count() < 5) {
                for (auto& e : transposition_table) e.hash = 0;
                
                std::shuffle(opponent_cards.begin(), opponent_cards.end(), gen);

                for (int i = 1; i < 4; i++) {
                    for (int j = 0; j < 8; j++) {
                        player_cards[i][j] = opponent_cards[(i - 1) * 8 + j];
                    }
                }

                GameState game_state(player_cards);

                for (int i = 0; i < 8; i++) {
                    uint8_t card = player_cards[0][i];
                    game_state.make_move(card);
                    votes[card] += minimax(game_state, depth, -131, 131, false, 1);
                    game_state.undo_move();
                }

                end = std::chrono::high_resolution_clock::now();
                duration = end - start;
                iterations++;
            }

            std::vector<std::pair<uint8_t, double>> sorted_votes;
            sorted_votes.reserve(votes.size());

            for (auto& kv : votes) {
                kv.second /= iterations;
                sorted_votes.push_back(kv);
            }

            std::sort(sorted_votes.begin(), sorted_votes.end(),
                    [](const auto& a, const auto& b) {
                        return a.second > b.second;
                    });

            if (sorted_votes_benchmark[0].first == sorted_votes[0].first || sorted_votes_benchmark[1].first == sorted_votes[0].first) { success[depth - 1]++; std::cout << "Depth #" << depth << ": Success\n"; }
            else std::cout << "Depth #" << depth << ": Failure\n";
        }

        for (int i = 0; i < success.size(); i++) {
            std::cout << "Depth #" << i + 1 << " Success Rate: " << success[i] * 100 / (t + 1) << "\n";
        }
        std::cout << std::endl;
    }

    // std::array<std::array<uint8_t, 8>, 4> player_cards = {{
    //     {35, 37, 26, 34, 1, 39, 51, 7},
    //     {38, 20, 48, 9, 40, 36, 24, 25},
    //     {50, 27, 11, 46, 49, 23, 47, 33},
    //     {21, 8, 22, 12, 14, 10, 13, 52}
    // }};

    // GameState game_state(player_cards);

    // game_state.view_player_cards();

    // game_state.make_move(1);

    // auto start = std::chrono::high_resolution_clock::now();

    // int score;
    // for (int i = 1; i <= ceil((double) (32 - game_state.num_of_played_cards()) / 4) - 1; i++) {
    //     score = minimax(game_state, i, -131, 131, false, 0);
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

    // return 0;
}