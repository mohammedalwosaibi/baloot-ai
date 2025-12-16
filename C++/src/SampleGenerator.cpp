#include "SampleGenerator.h"

SampleGenerator::SampleGenerator(const std::array<uint8_t, 8>& current_player_cards, int player_id) :
current_player_cards_(current_player_cards),
player_id_(player_id),
allowed_players_{},
played_cards_{},
player_has_suit_{},
current_player_(0)
{}

