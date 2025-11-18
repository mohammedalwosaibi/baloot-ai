#include "GameState.h"
#include <iostream>

GameState::GameState(const std::array<std::array<uint8_t, 8>, 4>& player_cards) : player_cards_(player_cards) {}

void GameState::view_player_cards() {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            std::cout << +player_cards_[i][j] << " ";
        }
        std::cout << "\n";
    }
}