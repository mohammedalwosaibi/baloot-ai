#include "utils.h"
#include "GameState.h"
#include "search.h"
#include "constants.h"
#include "SampleGenerator.h"
#include <iostream>
#include <cstdint>
#include <array>
#include <chrono>
#include <random>
#include <cmath>
#include <fstream>
#include <unordered_map>

extern std::array<std::array<uint8_t, 32>, 33> pv_table;
extern std::array<uint8_t, 33> pv_length;

extern std::array<TTEntry, 1 << 20> transposition_table;

extern int nodes_visited;

static constexpr int TRIALS = 100;

int main() {
    std::random_device rd;
    std::mt19937 gen(rd());

    std::array<uint8_t, 32> deck = {
        1, 7, 8, 9, 10, 11, 12, 13,
        14, 20, 21, 22, 23, 24, 25, 26,
        27, 33, 34, 35, 36, 37, 38, 39,
        40, 46, 47, 48, 49, 50, 51, 52
    };

    std::shuffle(deck.begin(), deck.end(), gen);

    std::array<std::array<uint8_t, 8>, 4> player_cards{};

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            player_cards[i][j] = deck[i * 8 + j];
        }
    }

    std::sort(player_cards[0].begin(), player_cards[0].end(),
            [](const auto& a, const auto& b) {
                return SUIT_SYMBOLS[get_suit(a)] != SUIT_SYMBOLS[get_suit(b)] ? SUIT_SYMBOLS[get_suit(a)] > SUIT_SYMBOLS[get_suit(b)] : RANK_ORDER[get_rank(a)] > RANK_ORDER[get_rank(b)];
            });

    SampleGenerator player_0_sg(player_cards[0], 0);
    SampleGenerator player_1_sg(player_cards[1], 1);
    SampleGenerator player_2_sg(player_cards[2], 2);
    SampleGenerator player_3_sg(player_cards[3], 3);

    std::array<SampleGenerator, 4> sg_arr = {player_0_sg, player_1_sg, player_2_sg, player_3_sg};

    GameState game_state(player_cards);

    while (game_state.num_of_played_cards() != 32) {
        if (game_state.current_player() == 0) {
            std::cout << "Player Cards: ";
            for (int i = 0; i < 8; i++) if (player_cards[0][i] != 0) std::cout << RANK_NAMES[get_rank(player_cards[0][i])] << SUIT_SYMBOLS[get_suit(player_cards[0][i])] << " (" << +player_cards[0][i] << ") ";
            std::cout << "\n";
            std::cout << "Player 0: ";
            int card_in;
            std::cin >> card_in;
            uint8_t card = static_cast<uint8_t>(card_in);

            std::array<std::array<uint8_t, 8>, 4> sample;
            sg_arr[0].generate_sample(sample);
            game_state.set_player_cards(sample);

            for (uint8_t& c : player_cards[0]) if (c == card) c = 0;
            game_state.make_move(card);
            for (int i = 0; i < 4; i++) sg_arr[i].play_card(card, 0);
        } else {
            // generate best move from other players
            auto start = std::chrono::high_resolution_clock::now();
            auto mid = std::chrono::high_resolution_clock::now();
            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> duration = end - start;

            std::unordered_map<uint8_t, double> votes;
            uint8_t best_card = -1;
            std::unordered_map<uint8_t, int> best_moves;
            while (duration.count() < 5) {
                for (auto& e : transposition_table) e.hash = 0;
                
                std::array<std::array<uint8_t, 8>, 4> sample;
                sg_arr[game_state.current_player()].generate_sample(sample);
                game_state.set_player_cards(sample);

                std::array<uint8_t, 8> moves;
                uint8_t num_moves = game_state.get_legal_moves(moves);

                int best_score = game_state.current_player() == 0 || game_state.current_player() == 2 ? -INT_MAX : INT_MAX;
                uint8_t cur_card;
                for (int i = 0; i < num_moves; i++) {
                    uint8_t card = moves[i];
                    game_state.make_move(card);
                    votes[card] += minimax(game_state, 4, -131, 131, game_state.current_player() == 0 || game_state.current_player() == 2, 0);
                    game_state.undo_move();
                    if (game_state.current_player() % 2 == 0 && votes[card] > best_score) {
                        cur_card = card;
                        best_score = votes[card];
                    } else if (game_state.current_player() % 2 == 1 && votes[card] < best_score) {
                        cur_card = card;
                        best_score = votes[card];
                    }
                }

                best_moves[cur_card]++;

                if (cur_card != best_card) {
                    best_card = cur_card;
                    mid = std::chrono::high_resolution_clock::now();
                }

                end = std::chrono::high_resolution_clock::now();
                duration = end - start;
            }
            std::chrono::duration<double> last_update = end - mid;

            for (auto& [card, freq] : best_moves) {
                std::cout << RANK_NAMES[get_rank(card)] << SUIT_SYMBOLS[get_suit(card)] << " " << freq << "\n";
            }

            std::cout << "Time since last updated card: " << last_update.count() << " seconds\n";

            std::vector<std::pair<uint8_t, double>> sorted_votes;
            sorted_votes.reserve(votes.size());

            for (auto& kv : votes) {
                sorted_votes.push_back(kv);
                std::cout << RANK_NAMES[get_rank(kv.first)] << SUIT_SYMBOLS[get_suit(kv.first)] << " " << kv.second << "\n";
            }

            if (game_state.current_player() == 0 || game_state.current_player() == 2) std::sort(sorted_votes.begin(), sorted_votes.end(),
                    [](const auto& a, const auto& b) {
                        return a.second > b.second;
                    });
            else std::sort(sorted_votes.begin(), sorted_votes.end(),
                    [](const auto& a, const auto& b) {
                        return a.second < b.second;
                    });

            for (int i = 0; i < 4; i++) sg_arr[i].play_card(sorted_votes[0].first, game_state.current_player());
            std::cout << "Player " << +game_state.current_player() << ": " << RANK_NAMES[get_rank(sorted_votes[0].first)] << SUIT_SYMBOLS[get_suit(sorted_votes[0].first)] << std::endl;
            game_state.make_move(sorted_votes[0].first);
        }
        if (game_state.num_of_played_cards() % 4 == 0) std::cout << "\n";
    }

    std::cout << "Score: " << +game_state.evaluate() << std::endl;


    // std::random_device rd;
    // std::mt19937 gen(rd());

    // std::array<double, 5> success{};

    // for (int t = 0; t < 200; t++) {
    //     std::cout << "Trial #" << t << "\n";

    //     std::unordered_map<uint8_t, double> votes;

    //     std::array<uint8_t, 32> deck = {21, 23, 13, 20, 24, 46, 52, 35, 36, 37, 1, 12, 14, 40, 26, 22, 27, 10, 49, 7, 48, 47, 38, 25, 9, 39, 51, 8, 33, 34, 50, 11};

    //     std::shuffle(deck.begin(), deck.end(), gen);

    //     std::array<std::array<uint8_t, 8>, 4> player_cards{};

    //     std::array<uint8_t, 24> opponent_cards{};

    //     for (int i = 0; i < 4; i++) {
    //         for (int j = 0; j < 8; j++) {
    //             player_cards[i][j] = deck[i * 8 + j];
    //             if (i != 0) opponent_cards[(i - 1) * 8 + j] = deck[i * 8 + j];
    //         }
    //     }

    //     auto start = std::chrono::high_resolution_clock::now();
    //     auto end = std::chrono::high_resolution_clock::now();
    //     std::chrono::duration<double> duration = end - start;
        
    //     int iterations = 0;

    //     while (duration.count() < 30) {
    //         for (auto& e : transposition_table) e.hash = 0;
            
    //         std::shuffle(opponent_cards.begin(), opponent_cards.end(), gen);

    //         for (int i = 1; i < 4; i++) {
    //             for (int j = 0; j < 8; j++) {
    //                 player_cards[i][j] = opponent_cards[(i - 1) * 8 + j];
    //             }
    //         }

    //         GameState game_state(player_cards);

    //         for (int i = 0; i < 8; i++) {
    //             uint8_t card = player_cards[0][i];
    //             game_state.make_move(card);
    //             votes[card] += minimax(game_state, 4, -131, 131, false, 1);
    //             game_state.undo_move();
    //         }

    //         end = std::chrono::high_resolution_clock::now();
    //         duration = end - start;
    //         iterations++;
    //     }

    //     std::vector<std::pair<uint8_t, double>> sorted_votes_benchmark;
    //     sorted_votes_benchmark.reserve(votes.size());

    //     for (auto& kv : votes) {
    //         kv.second /= iterations;
    //         sorted_votes_benchmark.push_back(kv);
    //     }

    //     std::sort(sorted_votes_benchmark.begin(), sorted_votes_benchmark.end(),
    //             [](const auto& a, const auto& b) {
    //                 return a.second > b.second;
    //             });

    //     for (int depth = 1; depth <= 5; depth++) {
    //         start = std::chrono::high_resolution_clock::now();
    //         end = std::chrono::high_resolution_clock::now();
    //         duration = end - start;
    //         iterations = 0;

    //         while (duration.count() < 5) {
    //             for (auto& e : transposition_table) e.hash = 0;
                
    //             std::shuffle(opponent_cards.begin(), opponent_cards.end(), gen);

    //             for (int i = 1; i < 4; i++) {
    //                 for (int j = 0; j < 8; j++) {
    //                     player_cards[i][j] = opponent_cards[(i - 1) * 8 + j];
    //                 }
    //             }

    //             GameState game_state(player_cards);

    //             for (int i = 0; i < 8; i++) {
    //                 uint8_t card = player_cards[0][i];
    //                 game_state.make_move(card);
    //                 votes[card] += minimax(game_state, depth, -131, 131, false, 1);
    //                 game_state.undo_move();
    //             }

    //             end = std::chrono::high_resolution_clock::now();
    //             duration = end - start;
    //             iterations++;
    //         }

    //         std::vector<std::pair<uint8_t, double>> sorted_votes;
    //         sorted_votes.reserve(votes.size());

    //         for (auto& kv : votes) {
    //             kv.second /= iterations;
    //             sorted_votes.push_back(kv);
    //         }

    //         std::sort(sorted_votes.begin(), sorted_votes.end(),
    //                 [](const auto& a, const auto& b) {
    //                     return a.second > b.second;
    //                 });

    //         if (sorted_votes_benchmark[0].first == sorted_votes[0].first || sorted_votes_benchmark[1].first == sorted_votes[0].first) { success[depth - 1]++; std::cout << "Depth #" << depth << ": Success\n"; }
    //         else std::cout << "Depth #" << depth << ": Failure\n";
    //     }

    //     for (int i = 0; i < success.size(); i++) {
    //         std::cout << "Depth #" << i + 1 << " Success Rate: " << success[i] * 100 / (t + 1) << "\n";
    //     }
    //     std::cout << std::endl;
    // }

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