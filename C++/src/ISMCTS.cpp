#include "ISMCTS.h"
#include <random>

static constexpr std::array<uint8_t, 52> INDEX_52_TO_32 = {
    0, 255, 255, 255, 255, 255, 1, 2, 3, 4, 5, 6, 7,
    8, 255, 255, 255, 255, 255, 9, 10, 11, 12, 13, 14, 15,
    16, 255, 255, 255, 255, 255, 17, 18, 19, 20, 21, 22, 23,
    24, 255, 255, 255, 255, 255, 25, 26, 27, 28, 29, 30, 31
};

ISMCTS::ISMCTS(const std::array<uint8_t, 8>& current_player_cards, uint8_t player_id) :
current_player_cards_(current_player_cards),
player_id_(player_id),
nodes_{},
game_state_({}),
sample_generator_(current_player_cards_, player_id),
rng_(std::random_device{}())
{
    Node root = Node();
    nodes_.push_back(root);
    root_ = root;
}

void ISMCTS::randomize_cards() {
    std::array<std::array<uint8_t, 8>, 4> sample;
    sample_generator_.generate_sample(sample);
    game_state_.set_player_cards(sample);
}

void ISMCTS::play_card(uint8_t card, uint8_t player_id) {
    game_state_.make_move(card);
    sample_generator_.play_card(card, player_id);
}

void ISMCTS::run(uint8_t max_duration) {
    auto start = std::chrono::high_resolution_clock::now();
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    while (duration.count() < max_duration) {
        randomize_cards();

        Node& curr = root_;
        std::vector<Node> path = {curr}; // fix copying
        GameState state_copy = game_state_;

        while (state_copy.num_of_played_cards() != 32) {
            std::array<uint8_t, 8> moves;
            state_copy.get_legal_moves(moves);

            std::shuffle(moves.begin(), moves.end(), rng_);

            float best_ucb1 = -std::numeric_limits<float>::infinity();
            uint8_t best_move = 255;

            for (uint8_t move_52 : moves) {
                uint8_t move_32 = INDEX_52_TO_32[move_52];

                Node child_node;
                if (curr.child_indices[move_32] == 0) {
                    child_node = Node();
                    nodes_.push_back(child_node);
                    curr.child_indices[move_32] = nodes_.size() - 1;

                    best_ucb1 = std::numeric_limits<float>::infinity();
                    best_move = move_52;
                } else {
                    child_node = nodes_[curr.child_indices[move_32]]; // fix copying
                    if (nodes_[curr.child_indices[move_32]].ucb1() > best_ucb1) {
                        best_ucb1 = nodes_[curr.child_indices[move_32]].ucb1();
                        best_move = move_52;
                    }
                }
                child_node.availability++;
            }


        }

        // backpropagation on path


        end = std::chrono::high_resolution_clock::now();
        duration = end - start;
    }
}