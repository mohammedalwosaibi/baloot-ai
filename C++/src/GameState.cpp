#include "GameState.h"
#include "utils.h"
#include "constants.h"
#include <iostream>

GameState::GameState(const std::array<std::array<uint8_t, 8>, 4>& player_cards) :
player_cards_(player_cards),
played_cards_{},
player_indices_{},
card_indices_{},
trick_scores_{},
current_player_(0),
num_of_played_cards_(0),
score_(0),
hash_(0)
{
    init_hashes();
    hash_ ^= PLAYER_KEY[current_player_];
    hash_ ^= SCORE_KEY[current_player_];
}

void GameState::view_player_cards() {
    for (int i = 0; i < 4; i++) {
        std::cout << "Player #" << i + 1 << "'s Cards: ";
        for (int j = 0; j < 8; j++) {
            uint8_t card = player_cards_[i][j];
            uint8_t rank = get_rank(card);
            std::cout << (rank == 10 ? "" : " ") << RANK_NAMES[rank] << SUIT_SYMBOLS[get_suit(card)] << (j == 7 ? "\n" : " ");
        }
    }
    
    std::cout << "\n";
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
            hash_ ^= CARD_KEY[card];
            hash_ ^= PLAYER_KEY[current_player_];
            if (num_of_played_cards_ % 4 == 0) {
                auto [trick_winner, trick_score] = get_trick_stats(&played_cards_[num_of_played_cards_ - 4]);
                current_player_ = (current_player_ + 1 + trick_winner) % 4;
                hash_ ^= SCORE_KEY[score_];
                uint8_t trick_idx = (num_of_played_cards_ / 4) - 1;
                if (current_player_ == 0 || current_player_ == 2) {
                    if (num_of_played_cards_ == 32) trick_score += 10;
                    score_ += trick_score;
                    trick_scores_[trick_idx] = trick_score;
                } else {
                    trick_scores_[trick_idx] = 0;
                }
                hash_ ^= SCORE_KEY[score_];
            } else {
                current_player_ = (current_player_ + 1) % 4;
            }
            hash_ ^= PLAYER_KEY[current_player_];
            break;
        }
    }
}

void GameState::undo_move() {
    if (num_of_played_cards_ % 4 == 0) {
        hash_ ^= SCORE_KEY[score_];
        score_ -= trick_scores_[(num_of_played_cards_ / 4) - 1];
        hash_ ^= SCORE_KEY[score_];
    }

    num_of_played_cards_--;
    uint8_t last_player = player_indices_[num_of_played_cards_];
    std::array<uint8_t, 8>& last_player_cards = player_cards_[last_player];

    last_player_cards[card_indices_[num_of_played_cards_]] = played_cards_[num_of_played_cards_];
    hash_ ^= CARD_KEY[played_cards_[num_of_played_cards_]];

    hash_ ^= PLAYER_KEY[current_player_];
    current_player_ = last_player;
    hash_ ^= PLAYER_KEY[current_player_];
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

uint8_t GameState::evaluate() const {
    return score_;
}

uint64_t GameState::hash() const { return hash_; }

uint8_t GameState::current_player() const { return current_player_; }

uint8_t GameState::num_of_played_cards() const { return num_of_played_cards_; };

