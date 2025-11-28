#pragma once
#include <array>
#include <vector>
#include <cstdint>

class GameState {
public:
    GameState(const std::array<std::array<uint8_t, 8>, 4>& player_cards);
    void view_player_cards();
    void make_move(uint8_t card);
    void undo_move();
    uint8_t get_legal_moves(std::array<uint8_t, 8>& moves);
    const std::array<uint8_t, 32>& played_cards() const;
    uint8_t score() const;

private:
    std::array<std::array<uint8_t, 8>, 4> player_cards_;
    std::array<uint8_t, 32> played_cards_;
    std::array<uint8_t, 32> player_indices_;
    std::array<uint8_t, 32> card_indices_;
    std::array<uint8_t, 8> trick_scores_;
    uint8_t current_player_;
    uint8_t num_of_played_cards_;
    uint8_t score_;
    uint64_t hash_;
};