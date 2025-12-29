#include "utils.h"
#include "GameState.h"
#include "search.h"
#include "constants.h"
#include "SampleGenerator.h"
#include "ISMCTS.h"
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

extern std::array<TTEntry, TABLE_SIZE>transposition_table;

int main() {
    // std::random_device rd;
    // std::mt19937 gen(3);

    // std::array<uint8_t, 32> deck = {
    //     0, 6, 7, 8, 9, 10, 11, 12,
    //     13, 19, 20, 21, 22, 23, 24, 25,
    //     26, 32, 33, 34, 35, 36, 37, 38,
    //     39, 45, 46, 47, 48, 49, 50, 51
    // };

    // std::shuffle(deck.begin(), deck.end(), gen);

    // std::array<std::array<uint8_t, 8>, 4> player_cards = {{
    //     { 26, 12, 10,  8, 48, 51, 46, 20 },
    //     { 37, 13, 50, 45, 33, 36, 23, 22 },
    //     {  7, 11, 47, 34, 19, 32, 25, 21 },
    //     {  0,  9, 39, 49,  6, 24, 38, 35 }
    // }};

    // for (size_t i = 0; i < 4; i++) {
    //     for (size_t j = 0; j < 8; j++) {
    //         player_cards[i][j] = deck[i * 8 + j];
    //     }
    // }

    // GameState game_state(player_cards);
    // game_state.view_player_cards();

    // ISMCTS ismcts(player_cards[0], 0);

    // ismcts.run(30);
    
    // uint8_t best_move = ismcts.best_move();

    // std::cout << "Player 0: " << RANK_NAMES[get_rank(best_move)] << SUIT_SYMBOLS[get_suit(best_move)] << std::endl;

    std::random_device rd;
    std::mt19937 gen(rd());

    std::array<uint8_t, 32> deck = {
        0, 6, 7, 8, 9, 10, 11, 12,
        13, 19, 20, 21, 22, 23, 24, 25,
        26, 32, 33, 34, 35, 36, 37, 38,
        39, 45, 46, 47, 48, 49, 50, 51
    };

    int home_score = 0;
    int away_score = 0;

    uint8_t starting_player = 0;
    
    while (true) {
        for (auto& e : transposition_table) {
            e.hash = 0;
            e.trick_depth = 0;
        }

        std::shuffle(deck.begin(), deck.end(), gen);

        std::array<std::array<uint8_t, 8>, 4> player_cards = {{
            { 35,  0,  8,  6, 45, 25, 24, 21 },
            { 26, 32, 11, 51, 13, 22, 23, 19 },
            { 34, 33, 39, 48, 50, 49, 47, 20 },
            { 38, 37, 36,  9, 12, 10,  7, 46 }
        }};

        for (size_t i = 0; i < 4; i++) {
            for (size_t j = 0; j < 8; j++) {
                player_cards[i][j] = deck[i * 8 + j];
            }
        }

        for (uint8_t i = 0; i < 4; i++) std::sort(player_cards[i].begin(), player_cards[i].end(),
                [](const auto& a, const auto& b) {
                    return SUIT_SYMBOLS[get_suit(a)] != SUIT_SYMBOLS[get_suit(b)] ? SUIT_SYMBOLS[get_suit(a)] > SUIT_SYMBOLS[get_suit(b)] : RANK_ORDER[get_rank(a)] > RANK_ORDER[get_rank(b)];
                });

        ISMCTS player_0_ismcts(player_cards[0], 0, player_cards);
        ISMCTS player_1_ismcts(player_cards[1], 1, player_cards);
        ISMCTS player_2_ismcts(player_cards[2], 2, player_cards);
        ISMCTS player_3_ismcts(player_cards[3], 3, player_cards);

        std::array<ISMCTS, 4> ismcts_arr = {player_0_ismcts, player_1_ismcts, player_2_ismcts, player_3_ismcts};

        GameState game_state(player_cards);
        
        game_state.view_player_cards();

        game_state.set_current_player(starting_player);
        for (size_t i = 0; i < 4; i++) ismcts_arr[i].set_current_player(starting_player);
        starting_player = (starting_player + 1) % 4;

        while (game_state.num_of_played_cards() != 32) {
            if (game_state.current_player() % 2 == 0) {
                ismcts_arr[game_state.current_player()].run(0.1);
                uint8_t move = ismcts_arr[game_state.current_player()].best_move();
                for (size_t i = 0; i < 4; i++) {
                    ismcts_arr[i].play_card(move, game_state.current_player());
                }
                std::cout << "Player " << +game_state.current_player() << ": " << RANK_NAMES[get_rank(move)] << SUIT_SYMBOLS[get_suit(move)] << std::endl;
                for (uint8_t& card : player_cards[game_state.current_player()]) if (card == move) card = NO_CARD;

                game_state.make_move(move);
            } else {
                ismcts_arr[game_state.current_player()].run(0.1);
                uint8_t move = ismcts_arr[game_state.current_player()].best_move();
                for (size_t i = 0; i < 4; i++) {
                    ismcts_arr[i].play_card(move, game_state.current_player());
                }
                std::cout << "Player " << +game_state.current_player() << ": " << RANK_NAMES[get_rank(move)] << SUIT_SYMBOLS[get_suit(move)] << std::endl;
                for (uint8_t& card : player_cards[game_state.current_player()]) if (card == move) card = NO_CARD;

                game_state.make_move(move);
            }
            if (game_state.num_of_played_cards() % 4 == 0) std::cout << "\n";
        }

        std::cout << "Score: " << +game_state.evaluate() << std::endl;
        home_score += game_state.evaluate();
        away_score += (130 - game_state.evaluate());
        std::cout << "Home Score: " << home_score << std::endl;
        std::cout << "Away Score: " << away_score << std::endl;
    }

    return 0;
}