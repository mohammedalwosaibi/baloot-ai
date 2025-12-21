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

int main() {
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
    
    while (home_score < 200 && away_score < 200
    ) {

        std::shuffle(deck.begin(), deck.end(), gen);

        std::array<std::array<uint8_t, 8>, 4> player_cards{};

        for (size_t i = 0; i < 4; i++) {
            for (size_t j = 0; j < 8; j++) {
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
                for (size_t i = 0; i < 8; i++) if (player_cards[0][i] != 0) std::cout << RANK_NAMES[get_rank(player_cards[0][i])] << SUIT_SYMBOLS[get_suit(player_cards[0][i])] << " (" << +player_cards[0][i] << ") ";
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
                for (size_t i = 0; i < 4; i++) sg_arr[i].play_card(card, 0);
            } else {
                // generate best move from other players
                auto start = std::chrono::high_resolution_clock::now();
                auto end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> duration = end - start;

                std::unordered_map<uint8_t, double> votes;
                while (duration.count() < 2) {
                    for (auto& e : transposition_table) e.hash = 0;
                    
                    std::array<std::array<uint8_t, 8>, 4> sample;
                    sg_arr[game_state.current_player()].generate_sample(sample);
                    game_state.set_player_cards(sample);

                    std::array<uint8_t, 8> moves;
                    uint8_t num_moves = game_state.get_legal_moves(moves);

                    for (size_t i = 0; i < num_moves; i++) {
                        uint8_t card = moves[i];
                        game_state.make_move(card);
                        votes[card] += minimax(game_state, 4, 0, 130, game_state.current_player() == 0 || game_state.current_player() == 2, 0);
                        game_state.undo_move();
                    }

                    end = std::chrono::high_resolution_clock::now();
                    duration = end - start;
                }

                std::vector<std::pair<uint8_t, double>> sorted_votes;
                sorted_votes.reserve(votes.size());

                for (auto& kv : votes) {
                    sorted_votes.push_back(kv);
                }

                if (game_state.current_player() == 0 || game_state.current_player() == 2) std::sort(sorted_votes.begin(), sorted_votes.end(),
                        [](const auto& a, const auto& b) {
                            return a.second > b.second;
                        });
                else std::sort(sorted_votes.begin(), sorted_votes.end(),
                        [](const auto& a, const auto& b) {
                            return a.second < b.second;
                        });

                for (size_t i = 0; i < 4; i++) sg_arr[i].play_card(sorted_votes[0].first, game_state.current_player());
                std::cout << "Player " << +game_state.current_player() << ": " << RANK_NAMES[get_rank(sorted_votes[0].first)] << SUIT_SYMBOLS[get_suit(sorted_votes[0].first)] << std::endl;
                game_state.make_move(sorted_votes[0].first);
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