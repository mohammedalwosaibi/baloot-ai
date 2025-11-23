#pragma once
#include <array>
#include <cstdint>

class GameState {
public:
    GameState(const std::array<std::array<uint8_t, 8>, 4>& player_cards);
    void view_player_cards();
    void make_move(uint8_t card);
    void undo_move();

private:
    std::array<std::array<uint8_t, 8>, 4> player_cards_;
    std::array<uint8_t, 32> played_cards_;
    std::array<uint8_t, 32> player_indices_;
    std::array<uint8_t, 32> card_indices_;
    uint8_t current_player_;
    uint8_t num_of_played_cards_;
};