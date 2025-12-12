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

int nodes_visited = 0;

/*
{14, 39, 1, 47, 26, 33, 52, 20},
{7, 25, 22, 50, 11, 46, 24, 49},
{36, 10, 35, 27, 12, 37, 23, 8},
{34, 38, 40, 48, 21, 51, 13, 9}
*/

extern int num_samples;
extern std::array<uint8_t, 7> cur_samples;

int main() {
    std::array<std::array<uint8_t, 8>, 4> player_cards = {{
        {46, 49, 8, 38, 47, 51, 23, 13},
        {21, 50, 24, 36, 37, 20, 52, 7},
        {1, 34, 35, 26, 40, 12, 11, 39},
        {10, 33, 9, 48, 14, 22, 27, 25}
    }};

    std::array<uint8_t, 32> deck = {40, 11, 23, 52, 47, 14, 7, 33, 21, 22, 49, 10, 1, 36, 48, 27, 12, 24, 8, 38, 51, 20, 25, 37, 13, 35, 34, 50, 39, 26, 46, 9};

    int ITERS = 200;

    double time_taken = 0;

    std::ofstream file("samples.csv");

    file << "score_diff,tricks_left,is_home_turn,"
         << "home_num_A,home_num_10,home_num_K,home_num_Q,home_num_J,home_num_9,home_num_8,home_num_7,"
         << "away_num_A,away_num_10,away_num_K,away_num_Q,away_num_J,away_num_9,away_num_8,away_num_7,"
         << "true_score\n";

    std::array<double, 7> root_regret{}; // quantity of score missed at lower depths
    std::array<double, 7> root_success{}; // matching rate of lower depth scores to optimal score

    for (int _ = 0; _ < ITERS; _++) {
        num_samples = 0;
        cur_samples.fill(0);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::shuffle(deck.begin(), deck.end(), gen);

        for (int p = 0; p < 4; p++) {
            for (int c = 0; c < 8; c++) {
                player_cards[p][c] = deck[p * 8 + c];
            }
        }

        GameState game_state(player_cards);

        game_state.view_player_cards();

        // std::array<uint8_t, 8> root_moves{};
        // std::array<uint8_t, 7> root_scores{};

        // std::array<int, 53> searched{};
        // searched.fill(-1);

        auto start = std::chrono::high_resolution_clock::now();
        uint8_t score = minimax(game_state, 8, 0, 130, true, 0, file);
        // for (int i = 1; i <= 8; i++) {
        //     score = minimax(game_state, i, 0, 130, true, 0);
        //     root_moves[i - 1] = pv_table[0][0];
        // }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;

        time_taken += duration.count();

        // for (int i = 0; i < 7; i++) {
        //     if (searched[root_moves[i]] == -1) {
        //         game_state.make_move(root_moves[i]);
        //         root_scores[i] = minimax(game_state, 8, 0, 130, false, 0);
        //         game_state.undo_move();
        //         searched[root_moves[i]] = root_scores[i];
        //     }
        //     root_regret[i] += (score - searched[root_moves[i]]);
        //     root_success[i] += (score == searched[root_moves[i]] ? 1 : 0);
        // }
    }
    std::cout << "\nAverage Time Taken: " << time_taken / ITERS << " seconds\n";
    file.close();

    // for (int i = 0; i < 7; i++) {
    //     root_regret[i] /= ITERS;
    //     root_success[i] *= 100 / ITERS;
    //     std::cout << "Move #" << i + 1 << " -- Regret: " << root_regret[i] << " -- Success Rate: %" << root_success[i] << "\n";
    // }

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
    // for (int i = 1; i <= ceil((double) (32 - game_state.num_of_played_cards()) / 4); i++) {
    //     score = minimax(game_state, i, 0, 130, true, 0);
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