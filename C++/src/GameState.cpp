#include "GameState.h"
#include "utils.h"
#include <iostream>
#include <span>

GameState::GameState(const std::array<std::array<uint8_t, 8>, 4>& player_cards) :
player_cards_(player_cards),
played_cards_{},
player_indices_{},
card_indices_{},
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
            player_indices_[num_of_played_cards_] = current_player_;
            card_indices_[num_of_played_cards_] = i;
            played_cards_[num_of_played_cards_++] = card;
            if (num_of_played_cards_ % 4 == 0) {
                auto [trick_winner, _] = get_trick_stats(std::span(played_cards_).subspan(num_of_played_cards_ - 4, 4));
                current_player_ = (current_player_ + 1 + trick_winner) % 4;
            } else {
                current_player_ = (current_player_ + 1) % 4;
            }
        }
    }
}

void GameState::undo_move() {
    num_of_played_cards_--;
    uint8_t last_player = player_indices_[num_of_played_cards_];
    std::array<uint8_t, 8>& last_player_cards = player_cards_[last_player];

    last_player_cards[card_indices_[num_of_played_cards_]] = played_cards_[num_of_played_cards_];
    
    card_indices_[num_of_played_cards_] = 0;
    player_indices_[num_of_played_cards_] = 0;
    played_cards_[num_of_played_cards_] = 0;

    current_player_ = last_player;
}