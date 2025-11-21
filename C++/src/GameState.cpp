#include "GameState.h"
#include "utils.h"
#include <iostream>
#include <span>

GameState::GameState(const std::array<std::array<uint8_t, 8>, 4>& player_cards) :
player_cards_(player_cards),
played_cards_{},
current_player_(0),
num_of_played_cards_(0)
{}

void GameState::view_player_cards() {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            std::cout << +player_cards_[i][j] << " ";
        }
        std::cout << "\n";
    }
}

void GameState::make_move(uint8_t card) {
    std::array<uint8_t, 8>& current_player_cards = player_cards_[current_player_];

    for (int i = 0; i < 8; i++) {
        uint8_t curr = current_player_cards[i];
        if (curr == card) {
            current_player_cards[i] = 0;
            // update game state
            played_cards_[num_of_played_cards_++] = card;
            if (num_of_played_cards_ % 4 == 0) {
                auto [trick_winner, _] = get_trick_stats(std::span(played_cards_).subspan(num_of_played_cards_ - 4, 4));
                current_player_ = (current_player_ + 1 + trick_winner) % 4;
                std::cout << "Trick Winner: " << +trick_winner << "\n";
            } else {
                current_player_ = (current_player_ + 1) % 4;
            }
        }
    }
}