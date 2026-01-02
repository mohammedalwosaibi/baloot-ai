#include "ISMCTS.h"
#include "constants.h"
#include "utils.h"
#include <random>
#include <iostream>
#include <limits>
#include <cassert>

static constexpr std::array<uint8_t, 52> INDEX_52_TO_32 = {
    0, 255, 255, 255, 255, 255, 1, 2, 3, 4, 5, 6, 7,
    8, 255, 255, 255, 255, 255, 9, 10, 11, 12, 13, 14, 15,
    16, 255, 255, 255, 255, 255, 17, 18, 19, 20, 21, 22, 23,
    24, 255, 255, 255, 255, 255, 25, 26, 27, 28, 29, 30, 31
};

ISMCTS::ISMCTS(const std::array<uint8_t, 8>& current_player_cards, uint8_t player_id, std::array<std::array<uint8_t, 8>, 4> cards) :
current_player_cards_(current_player_cards),
nodes_{},
player_id_(player_id),
game_state_(cards),
state_copy_(cards),
sample_generator_(current_player_cards_, player_id),
rng_(std::random_device{}()),
mast_visits{},
mast_total_score{}
{}

void ISMCTS::randomize_cards() {
    std::array<std::array<uint8_t, 8>, 4> sample;
    sample_generator_.generate_sample(sample);
    state_copy_.set_player_cards(sample);
}

void ISMCTS::set_current_player(uint8_t player_id) {
    game_state_.set_current_player(player_id);
}

void ISMCTS::play_card(uint8_t card, uint8_t player_id) {
    sample_generator_.play_card(card, player_id);
    game_state_.make_move(card);
}

uint8_t ISMCTS::pick_rollout_move(uint8_t acting_player, size_t bucket, const std::array<uint8_t,8>& moves, uint8_t num_moves) {
    uint32_t N = 0;
    for (uint8_t i = 0; i < num_moves; i++) N += mast_visits[bucket][acting_player][moves[i]];
    if (N == 0) N = 1;

    double best_ucb = -std::numeric_limits<double>::infinity();
    uint8_t best = moves[0];

    std::uniform_real_distribution<float> uni01(0.0f, 1.0f);

    double C_MAST = 70.0 * (32 - state_copy_.num_of_played_cards()) / 32.0f;
    if (player_id_ % 2 == 0) C_MAST = 70.0 * (32 - state_copy_.num_of_played_cards()) / 32.0f;

    for (uint8_t i = 0; i < num_moves; i++) {
        uint8_t m = moves[i];
        uint32_t n = mast_visits[bucket][acting_player][m];

        double ucb;
        if (n == 0) {
            ucb = std::numeric_limits<double>::infinity();
        } else {
            double mean = (double) mast_total_score[bucket][acting_player][m] / (double) n;
            double bonus = C_MAST * std::sqrt(std::log((double) N) / (double) n);
            ucb = mean + bonus;
        }

        if (ucb > best_ucb) {
            best_ucb = ucb;
            best = m;
        } else if (ucb == best_ucb && uni01(rng_) < 0.5f) {
            best = m;
        }
    }

    int pos = state_copy_.num_of_played_cards() % 4;
    int cur_winner = state_copy_.currently_winning();
    int has_suit = state_copy_.current_has_suit() ? 1 : 0;
    int can_win = state_copy_.current_can_win() ? 1 : 0;
    int guaranteed_winner = state_copy_.guaranteed_winner() ? 1 : 0;

    if (pos > 1 && !can_win && cur_winner != acting_player % 2 || guaranteed_winner && cur_winner != acting_player % 2) {
        if (has_suit) {
            for (size_t i = 1; i < num_moves; i++) {
                if (RANK_ORDER[get_rank(moves[i])] < RANK_ORDER[get_rank(best)]) {
                    best = moves[i];
                }
            }
            
            return best;
        } else {
            std::array<uint8_t, 4> lowest_in_suit = {NO_CARD, NO_CARD, NO_CARD, NO_CARD};

            for (uint8_t i = 0; i < num_moves; i++) {
                uint8_t card = moves[i];
                uint8_t suit = get_suit(card);
                if (lowest_in_suit[suit] == NO_CARD || RANK_ORDER[get_rank(card)] < RANK_ORDER[get_rank(lowest_in_suit[suit])]) {
                    lowest_in_suit[suit] = card;
                }
            }

            best_ucb = -std::numeric_limits<double>::infinity();
            for (uint8_t card : lowest_in_suit) if (card != NO_CARD) {
                uint8_t m = card;
                uint32_t n = mast_visits[bucket][acting_player][m];

                double ucb;
                if (n == 0) {
                    ucb = std::numeric_limits<double>::infinity();
                } else {
                    double mean = (double) mast_total_score[bucket][acting_player][m] / (double) n;
                    double bonus = C_MAST * ((32 - state_copy_.num_of_played_cards()) / 32) * std::sqrt(std::log((double) N) / (double) n);
                    ucb = mean + bonus;
                }

                if (ucb > best_ucb) {
                    best_ucb = ucb;
                    best = m;
                } else if (ucb == best_ucb && uni01(rng_) < 0.5f) {
                    best = m;
                }
            }
        }
    }

    return best;
}

size_t ISMCTS::epic_key(const GameState& s) {
    // int pos = s.num_of_played_cards() % 4;
    int cur_winner = s.currently_winning();
    int has_suit = s.current_has_suit() ? 1 : 0;
    int can_win = s.current_can_win() ? 1 : 0;
    // int early = (s.num_of_played_cards() / 4) < 4 ? 1 : 0;


    return static_cast<size_t>(has_suit << 2 | can_win << 1 | cur_winner);
}

void ISMCTS::run(double max_duration) {
    nodes_.clear();
    nodes_.emplace_back();
    for (size_t b = 0; b < MAX_BUCKETS; b++) {
        for (size_t i = 0; i < 4; i++) {
            mast_total_score[b][i].fill(0);
            mast_visits[b][i].fill(0);
        }
    }
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    while (duration.count() < max_duration) {
        state_copy_ = game_state_;
        randomize_cards();

        std::vector<uint32_t> path = {0};
        std::vector<uint8_t> path_players = {state_copy_.current_player()};
        uint32_t current_idx = 0;
        std::vector<uint8_t> rollout_moves;
        std::vector<uint8_t> rollout_players;
        std::vector<size_t> rollout_buckets;

        while (state_copy_.num_of_played_cards() != 32) {
            std::array<uint8_t, 8> moves;
            uint8_t num_moves = state_copy_.get_legal_moves(moves);

            std::shuffle(moves.begin(), moves.begin() + num_moves, rng_);

            float best_ucb1 = -std::numeric_limits<float>::infinity();
            uint8_t best_move = moves[0];
            uint32_t best_idx = 0;

            for (uint8_t i = 0; i < num_moves; i++) {
                uint8_t move_52 = moves[i];
                uint8_t move_32 = INDEX_52_TO_32[move_52];
                uint32_t idx = nodes_[current_idx].child_indices[move_32];

                if (idx == 0) {
                    nodes_.emplace_back();
                    idx = static_cast<uint32_t>(nodes_.size()) - 1;
                    best_idx = idx;
                    nodes_[current_idx].child_indices[move_32] = idx;

                    best_ucb1 = std::numeric_limits<float>::infinity();
                    best_move = move_52;
                } else {
                    float child_ucb1 = nodes_[idx].ucb1(player_id_, 32 - state_copy_.num_of_played_cards());
                    if (child_ucb1 > best_ucb1) {
                        best_ucb1 = child_ucb1;
                        best_move = move_52;
                        best_idx = idx;
                    }
                }
                nodes_[idx].availability++;
            }

            int pos = state_copy_.num_of_played_cards() % 4;
            int cur_winner = state_copy_.currently_winning();
            int has_suit = state_copy_.current_has_suit() ? 1 : 0;
            int can_win = state_copy_.current_can_win() ? 1 : 0;
            int guaranteed_winner = state_copy_.guaranteed_winner() ? 1 : 0;

            if (pos > 1 && !can_win && cur_winner != state_copy_.current_player() % 2 || guaranteed_winner && cur_winner != state_copy_.current_player() % 2) {
                if (has_suit) {
                    for (size_t i = 0; i < num_moves; i++) {
                        if (RANK_ORDER[get_rank(moves[i])] < RANK_ORDER[get_rank(best_move)]) {
                            best_move = moves[i];
                            best_idx = nodes_[current_idx].child_indices[INDEX_52_TO_32[best_move]];
                            assert(best_idx != 0);
                        }
                    }
                } else {
                    std::array<uint8_t, 4> lowest_in_suit = {NO_CARD, NO_CARD, NO_CARD, NO_CARD};

                    for (uint8_t i = 0; i < num_moves; i++) {
                        uint8_t card = moves[i];
                        uint8_t suit = get_suit(card);
                        if (lowest_in_suit[suit] == NO_CARD || RANK_ORDER[get_rank(card)] < RANK_ORDER[get_rank(lowest_in_suit[suit])]) {
                            lowest_in_suit[suit] = card;
                        }
                    }

                    best_ucb1 = -std::numeric_limits<float>::infinity();
                    for (uint8_t card : lowest_in_suit) if (card != NO_CARD) {
                        uint8_t move_32 = INDEX_52_TO_32[card];
                        assert(move_32 != 255);
                        uint32_t idx = nodes_[current_idx].child_indices[move_32];
                        assert(idx != 0);
                        float child_ucb1 = nodes_[idx].ucb1(player_id_, 32 - state_copy_.num_of_played_cards());
                        if (child_ucb1 > best_ucb1) {
                            best_ucb1 = child_ucb1;
                            best_move = card;
                            best_idx = idx;
                        }
                    }
                }
            }

            path.push_back(best_idx);
            path_players.push_back(state_copy_.current_player());
            rollout_moves.push_back(best_move);
            rollout_buckets.push_back(epic_key(state_copy_));
            rollout_players.push_back(state_copy_.current_player());
            state_copy_.make_move(best_move);
            current_idx = best_idx;

            if (nodes_[best_idx].visits == 0) {
                while (state_copy_.num_of_played_cards() != 32) {
                    num_moves = state_copy_.get_legal_moves(moves);

                    size_t bucket = epic_key(state_copy_);

                    uint8_t chosen = pick_rollout_move(state_copy_.current_player(), bucket, moves, num_moves);
                    rollout_moves.push_back(chosen);
                    rollout_buckets.push_back(bucket);
                    rollout_players.push_back(state_copy_.current_player());

                    state_copy_.make_move(chosen);
                }
                break;
            }
        }

        int home_score = state_copy_.evaluate();
        if (home_score == 130) home_score = 220;
        int away_score = 220 - home_score;

        for (uint8_t i = 0; i < path.size(); i++) {
            uint32_t idx = path[i];
            int final_score = path_players[i] % 2 == 0 ? home_score : away_score;
            Node& node = nodes_[idx];
            node.visits++;
            node.total_score += static_cast<uint64_t>(final_score);
        }

        for (uint8_t i = 0; i < rollout_moves.size(); i++) {
            uint8_t acting_player = rollout_players[i];
            size_t bucket = rollout_buckets[i];
            uint8_t move = rollout_moves[i];
            mast_visits[bucket][acting_player][move]++;
            int final_score = acting_player % 2 == 0 ? home_score : away_score;
            mast_total_score[bucket][acting_player][move] += static_cast<uint64_t>(final_score);
        }

        end = std::chrono::high_resolution_clock::now();
        duration = end - start;
    }
}

uint8_t ISMCTS::best_move() {
    std::array<uint8_t, 8> moves;
    uint8_t num_moves = game_state_.get_legal_moves(moves);

    uint32_t best_visits = 0;
    uint8_t selected_move = moves[0];

    for (uint8_t i = 0; i < num_moves; i++) {
        uint8_t move_52 = moves[i];
        uint8_t move_32 = INDEX_52_TO_32[move_52];

        uint32_t child_idx = nodes_[0].child_indices[move_32];

        if (child_idx == 0) continue;

        uint32_t visits = nodes_[child_idx].visits;
        std::cout << RANK_NAMES[get_rank(move_52)] << SUIT_SYMBOLS[get_suit(move_52)] << " " << visits << std::endl;
        if (visits > best_visits) {
            best_visits = visits;
            selected_move = move_52;
        }
    }

    return selected_move;
}