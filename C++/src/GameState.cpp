#include "GameState.h"
#include "utils.h"
#include <iostream>
#include <span>

GameState::GameState(const std::array<std::array<uint8_t, 8>, 4>& player_cards) :
player_cards_(player_cards),
played_cards_{},
player_indices_{},
card_indices_{},
trick_scores_{},
current_player_(0),
num_of_played_cards_(0),
score_(0)
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
                auto [trick_winner, trick_score] = get_trick_stats(std::span(played_cards_).subspan(num_of_played_cards_ - 4, 4));
                current_player_ = (current_player_ + 1 + trick_winner) % 4;
                if (current_player_ == 0 || current_player_ == 2) {
                    if (num_of_played_cards_ == 32) trick_score += 10;
                    score_ += trick_score;
                    trick_scores_[(num_of_played_cards_ / 4) - 1] = trick_score;
                } else {
                    trick_scores_[(num_of_played_cards_ / 4) - 1] = 0;
                }
            } else {
                current_player_ = (current_player_ + 1) % 4;
            }
            break;
        }
    }
}

void GameState::undo_move() {
    if (num_of_played_cards_ % 4 == 0) {
        score_ -= trick_scores_[(num_of_played_cards_ / 4) - 1];
    }

    num_of_played_cards_--;
    uint8_t last_player = player_indices_[num_of_played_cards_];
    std::array<uint8_t, 8>& last_player_cards = player_cards_[last_player];

    last_player_cards[card_indices_[num_of_played_cards_]] = played_cards_[num_of_played_cards_];

    current_player_ = last_player;
}

uint8_t GameState::get_legal_moves(std::array<uint8_t, 8>& moves) {
    std::array<uint8_t, 8>& current_player_cards = player_cards_[current_player_];
    uint8_t num_moves = 0;

    if (num_of_played_cards_ % 4 == 0) {
        for (int i = 0; i < current_player_cards.size(); i++) {
            if (current_player_cards[i] != 0) moves[num_moves++] = current_player_cards[i];
        }
    } else {
        uint8_t trick_leader_idx = num_of_played_cards_ - (num_of_played_cards_ % 4);
        uint8_t trick_suit = get_suit(played_cards_[trick_leader_idx]);
        bool player_has_suit = has_suit(current_player_cards, trick_suit);
        for (int i = 0; i < current_player_cards.size(); i++) {
            uint8_t card = current_player_cards[i];
            if (card == 0) continue;
            if (player_has_suit && get_suit(card) == trick_suit) {
                moves[num_moves++] = card;
            } else if (!player_has_suit) {
                moves[num_moves++] = card;
            }
        }
    }

    return num_moves;
}

const std::array<uint8_t, 32>& GameState::played_cards() const { return played_cards_; }

uint8_t GameState::score() const { return score_; }