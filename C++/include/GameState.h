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
    int evaluate();
    uint64_t hash() const;
    uint8_t current_player() const;
    uint8_t num_of_played_cards() const;
    int score_difference() const;
    std::array<uint8_t, 14> home_ranks() const;
    std::array<uint8_t, 4> last_trick() const;
    void set_player_cards(std::array<std::array<uint8_t, 8>, 4>& sample);

private:
    std::array<std::array<uint8_t, 8>, 4> player_cards_;
    std::array<uint8_t, 32> played_cards_;
    std::array<uint8_t, 32> player_indices_;
    std::array<uint8_t, 32> card_indices_;
    std::array<uint8_t, 8> trick_scores_;
    std::array<uint8_t, 14> home_ranks_;
    std::array<uint8_t, 4> last_trick_;
    
    uint8_t current_player_;
    uint8_t num_of_played_cards_;
    uint8_t home_score_;
    uint8_t away_score_;
    uint64_t hash_;
};