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

static constexpr double C_MAST = 184.0;

ISMCTS::ISMCTS(const std::array<uint8_t, 8>& current_player_cards, uint8_t player_id, std::array<std::array<uint8_t, 8>, 4>& cards) :
current_player_cards_(current_player_cards),
player_id_(player_id),
nodes_{},
game_state_(cards),
state_copy_(cards),
sample_generator_(current_player_cards_, player_id),
belief_manager_(current_player_cards_, player_id),
rng_(std::random_device{}()),
mast_visits{},
mast_total_score{}
{}

uint16_t ISMCTS::randomize_cards() {
    std::array<std::array<uint8_t, 8>, 4> sample;
    sample_generator_.generate_sample(sample);
    state_copy_.set_player_cards(sample);
    return 1;
}

void ISMCTS::set_current_player(uint8_t player_id) {
    game_state_.set_current_player(player_id);
}

void ISMCTS::play_card(uint8_t card, uint8_t player_id) {
    sample_generator_.play_card(card, player_id);
    game_state_.make_move(card);
}

uint8_t ISMCTS::pick_rollout_move(uint8_t acting_player, uint8_t bucket, const std::array<uint8_t,8>& moves, uint8_t num_moves) {
    uint32_t N = 0;
    for (size_t c = 0; c < 52; c++) N += mast_visits[bucket][acting_player][c];
    if (N == 0) N = 1;

    double best_ucb = -std::numeric_limits<double>::infinity();
    uint8_t best = moves[0];

    std::uniform_real_distribution<float> uni01(0.0f, 1.0f);

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

    return best;
}

static uint8_t epic_key(const GameState& s) {
    uint8_t trick = s.num_of_played_cards() / 4;
    uint8_t leader = (s.num_of_played_cards() % 4 == 0) ? 1 : 0;
    return (trick << 1) | leader;
}

void ISMCTS::run(double max_duration) {
    nodes_.clear();
    nodes_.emplace_back();
    for (size_t b = 0; b < 16; b++) {
        for (size_t i = 0; i < 4; i++) {
            mast_total_score[b][i].fill(0);
            mast_visits[b][i].fill(0);
        }
    }
    root_stats_.clear();
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    while (duration.count() < max_duration) {
        state_copy_ = game_state_;

        uint16_t determinization_idx;
        if (game_state_.current_player() % 2 == 1) determinization_idx = randomize_cards();

        std::vector<uint32_t> path = {0};
        std::vector<uint8_t> path_players = {state_copy_.current_player()};
        uint32_t current_idx = 0;
        std::vector<uint8_t> rollout_moves;
        std::vector<uint8_t> rollout_players;
        std::vector<uint8_t> rollout_buckets;

        while (state_copy_.num_of_played_cards() != 32) {
            std::array<uint8_t, 8> moves;
            uint8_t num_moves = state_copy_.get_legal_moves(moves);

            std::shuffle(moves.begin(), moves.begin() + num_moves, rng_);

            float best_ucb1 = -std::numeric_limits<float>::infinity();
            uint8_t best_move = 255;
            uint32_t best_idx = 0;
            bool expanded = false;

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
                    expanded = true;
                } else {
                    float child_ucb1 = nodes_[idx].ucb1();
                    if (nodes_[idx].visits == 0) expanded = true;
                    if (child_ucb1 > best_ucb1) {
                        best_ucb1 = child_ucb1;
                        best_move = move_52;
                        best_idx = idx;
                    }
                }
                nodes_[idx].availability++;
            }

            if (current_idx == 0 && state_copy_.current_player() != player_id_ && state_copy_.current_player() % 2 == 1) {
                root_stats_.C_a[best_move]++;
                root_stats_.C_ad[best_move][determinization_idx]++;
                root_stats_.total_iters++;
            }

            path.push_back(best_idx);
            path_players.push_back(state_copy_.current_player());
            rollout_moves.push_back(best_move);
            rollout_buckets.push_back(epic_key(state_copy_));
            rollout_players.push_back(state_copy_.current_player());
            state_copy_.make_move(best_move);
            current_idx = best_idx;

            if (expanded) {
                while (state_copy_.num_of_played_cards() != 32) {
                    num_moves = state_copy_.get_legal_moves(moves);

                    uint8_t bucket = epic_key(state_copy_);

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
        int away_score = 130 - home_score;

        for (uint8_t i = 0; i < path.size(); i++) {
            uint32_t idx = path[i];
            int final_score = path_players[i] % 2 == 0 ? home_score : away_score;
            Node& node = nodes_[idx];
            node.visits++;
            node.total_score += static_cast<uint64_t>(final_score);
        }

        for (uint8_t i = 0; i < rollout_moves.size(); i++) {
            uint8_t acting_player = rollout_players[i];
            uint8_t bucket = rollout_buckets[i];
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