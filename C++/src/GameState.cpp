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
home_ranks_{},
away_ranks_{},
current_player_(0),
num_of_played_cards_(0),
home_score_(0),
away_score_(0),
hash_(0)
{
    init_hashes();
    hash_ ^= PLAYER_KEY[current_player_];
    hash_ ^= SCORE_KEY[current_player_];

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            if (i == 0 || i == 2) home_ranks_[get_rank(player_cards_[i][j])]++;
            else if (i == 1 || i == 3) away_ranks_[get_rank(player_cards_[i][j])]++;
        }
    }
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
            if (current_player_ == 0 || current_player_ == 2) home_ranks_[get_rank(card)]--;
            else if (current_player_ == 1 || current_player_ == 3) away_ranks_[get_rank(card)]--;

            current_player_cards[i] = 0;
            player_indices_[num_of_played_cards_] = current_player_;
            card_indices_[num_of_played_cards_] = i;
            played_cards_[num_of_played_cards_++] = card;
            hash_ ^= CARD_KEY[card];
            hash_ ^= PLAYER_KEY[current_player_];
            if (num_of_played_cards_ % 4 == 0) {
                auto [trick_winner, trick_score] = get_trick_stats(&played_cards_[num_of_played_cards_ - 4]);
                current_player_ = (current_player_ + 1 + trick_winner) % 4;
                hash_ ^= SCORE_KEY[home_score_];
                uint8_t trick_idx = (num_of_played_cards_ / 4) - 1;
                if (num_of_played_cards_ == 32) trick_score += 10;
                if (current_player_ == 0 || current_player_ == 2) {
                    home_score_ += trick_score;
                } else {
                    away_score_ += trick_score;
                }
                trick_scores_[trick_idx] = trick_score;
                hash_ ^= SCORE_KEY[home_score_];
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
        hash_ ^= SCORE_KEY[home_score_];
        if (current_player_ == 0 || current_player_ == 2) home_score_ -= trick_scores_[(num_of_played_cards_ / 4) - 1];
        else away_score_ -= trick_scores_[(num_of_played_cards_ / 4) - 1];
        hash_ ^= SCORE_KEY[home_score_];
    }

    num_of_played_cards_--;
    uint8_t last_player = player_indices_[num_of_played_cards_];
    std::array<uint8_t, 8>& last_player_cards = player_cards_[last_player];

    last_player_cards[card_indices_[num_of_played_cards_]] = played_cards_[num_of_played_cards_];
    hash_ ^= CARD_KEY[played_cards_[num_of_played_cards_]];

    hash_ ^= PLAYER_KEY[current_player_];
    current_player_ = last_player;
    hash_ ^= PLAYER_KEY[current_player_];

    if (current_player_ == 0 || current_player_ == 2) home_ranks_[get_rank(played_cards_[num_of_played_cards_])]++;
    else if (current_player_ == 1 || current_player_ == 3) away_ranks_[get_rank(played_cards_[num_of_played_cards_])]++;
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

/*
away_num_A      -9.880322
home_num_J      -3.707041
home_num_7      -3.356345
home_num_9      -2.371558
away_num_10     -2.357482
home_num_Q      -2.352208
home_num_8      -1.871341
home_num_K      -0.727490
tricks_left      0.377068
score_diff       0.500055
away_num_7       1.180671
away_num_K       1.498702
away_num_J       2.060756
away_num_9       2.469041
away_num_8       2.723933
home_num_10      2.866840
away_num_Q       3.058839
is_home_turn     9.433691
home_num_A      12.273280
*/

int GameState::evaluate() const {
    return home_score_;
    // if (num_of_played_cards_ == 32) return home_score_;

    // double eval = (
    //     -9.880322 * away_ranks_[1] + 
    //     -3.707041 * home_ranks_[11] +
    //     -3.356345 * home_ranks_[7] +
    //     -2.371558 * home_ranks_[9] +
    //     -2.357482 * away_ranks_[10] +
    //     -2.352208 * home_ranks_[12] +
    //     -1.871341 * home_ranks_[8] +
    //     -0.727490 * home_ranks_[13] +
    //     0.377068 * (32 - num_of_played_cards_) / 4 +
    //     0.500055 * score_difference() +
    //     1.180671 * away_ranks_[7] +
    //     1.498702 * away_ranks_[13] +
    //     2.060756 * away_ranks_[11] +
    //     2.469041 * away_ranks_[9] +
    //     2.723933 * away_ranks_[8] +
    //     2.866840 * home_ranks_[10] +
    //     3.058839 * away_ranks_[12] +
    //     9.433691 * ((current_player_ == 0 || current_player_ == 2) ? 1 : 0) +
    //     12.273280 * home_ranks_[1]
    // );

    // eval = std::clamp(eval, 0.0, 130.0);

    // return static_cast<int>(std::round(eval));
}

uint64_t GameState::hash() const { return hash_; }

uint8_t GameState::current_player() const { return current_player_; }

uint8_t GameState::num_of_played_cards() const { return num_of_played_cards_; };

int GameState::score_difference() const { return home_score_ - away_score_; };

std::array<uint8_t, 14> GameState::home_ranks() const { return home_ranks_; };

std::array<uint8_t, 14> GameState::away_ranks() const { return away_ranks_; };

