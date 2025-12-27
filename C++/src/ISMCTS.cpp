#include "ISMCTS.h"
#include "constants.h"
#include "utils.h"
#include <random>
#include <iostream>
#include <limits>

static constexpr std::array<uint8_t, 52> INDEX_52_TO_32 = {
    0, 255, 255, 255, 255, 255, 1, 2, 3, 4, 5, 6, 7,
    8, 255, 255, 255, 255, 255, 9, 10, 11, 12, 13, 14, 15,
    16, 255, 255, 255, 255, 255, 17, 18, 19, 20, 21, 22, 23,
    24, 255, 255, 255, 255, 255, 25, 26, 27, 28, 29, 30, 31
};

static constexpr float EPS = 0.2f;

ISMCTS::ISMCTS(const std::array<uint8_t, 8>& current_player_cards, uint8_t player_id) :
current_player_cards_(current_player_cards),
nodes_{},
// player_id_(player_id),
game_state_({}),
sample_generator_(current_player_cards_, player_id),
rng_(std::random_device{}()),
mast_visits{},
mast_total_score{}
{}

void ISMCTS::randomize_cards() {
    std::array<std::array<uint8_t, 8>, 4> sample;
    sample_generator_.generate_sample(sample);
    game_state_.set_player_cards(sample);
}

void ISMCTS::set_player_cards(std::array<std::array<uint8_t, 8>, 4>& sample) {
    game_state_.set_player_cards(sample);
}

void ISMCTS::set_current_player(uint8_t player_id) {
    game_state_.set_current_player(player_id);
}

void ISMCTS::play_card(uint8_t card, uint8_t player_id) {
    game_state_.make_move(card);
    sample_generator_.play_card(card, player_id);
}

uint8_t ISMCTS::pick_rollout_move(const std::array<uint8_t,8>& moves, uint8_t num_moves) {
    std::uniform_real_distribution<float> uni01(0.0f, 1.0f);
    if (uni01(rng_) < EPS) {
        std::uniform_int_distribution<int> uni(0, num_moves - 1);
        return moves[uni(rng_)];
    }

    double best_score = -std::numeric_limits<double>::infinity();
    uint8_t best = moves[0];

    for (int i = 0; i < num_moves; i++) {
        uint8_t move = moves[i];
        uint32_t visits = mast_visits[move];
        double score = (visits == 0) ? std::numeric_limits<double>::infinity() : ((double) mast_total_score[move] / visits);
        if (score > best_score) {
            best_score = score;
            best = move;
        } else if (score == best_score && uni01(rng_) < 0.5f) best = move;
    }
    return best;
}

void ISMCTS::run(uint8_t max_duration) {
    nodes_.clear();
    nodes_.emplace_back();
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    while (duration.count() < max_duration) {
        randomize_cards();

        std::vector<uint32_t> path = {0};
        std::vector<uint8_t> players = {game_state_.current_player()};
        GameState state_copy = game_state_;
        uint32_t current_idx = 0;
        std::vector<uint8_t> rollout_moves;

        while (state_copy.num_of_played_cards() != 32) {
            std::array<uint8_t, 8> moves;
            uint8_t num_moves = state_copy.get_legal_moves(moves);

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
                    if (child_ucb1 > best_ucb1) {
                        best_ucb1 = child_ucb1;
                        best_move = move_52;
                        best_idx = idx;
                    }
                }
                nodes_[idx].availability++;
            }

            path.push_back(best_idx);
            players.push_back(state_copy.current_player());
            state_copy.make_move(best_move);
            current_idx = best_idx;

            if (expanded) {
                while (state_copy.num_of_played_cards() != 32) {
                    num_moves = state_copy.get_legal_moves(moves);

                    uint8_t chosen = pick_rollout_move(moves, num_moves);
                    rollout_moves.push_back(chosen);
                    players.push_back(state_copy.current_player());
                    state_copy.make_move(chosen);
                }
                break;
            }
        }

        int home_score = state_copy.evaluate();
        int away_score = 130 - home_score;
        uint8_t path_size = path.size();

        for (uint8_t i = 0; i < path_size; i++) {
            uint32_t idx = path[i];
            int final_score = players[i] % 2 == 0 ? home_score : away_score;
            Node& node = nodes_[idx];
            node.visits++;
            node.total_score += static_cast<uint64_t>(final_score);
        }

        for (uint8_t i = 0; i < rollout_moves.size(); i++) {
            uint8_t acting_player = players[i + path_size];
            uint8_t move = rollout_moves[i];
            mast_visits[move]++;
            mast_total_score[move] += acting_player % 2 == 0 ? home_score : away_score;
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