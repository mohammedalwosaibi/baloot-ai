#include "constants.h"
#include "utils.h"

const std::array<uint8_t, 14> RANK_VALUES = {0, 11, 0, 0, 0, 0, 0, 0, 0, 0, 10, 2, 3, 4};

const std::array<uint8_t, 14> RANK_ORDER = {0, 7, 0, 0, 0, 0, 0, 0, 1, 2, 6, 3, 4, 5};

uint64_t CARD_KEY[NUM_CARDS + 1];
uint64_t PLAYER_KEY[NUM_PLAYERS];
uint64_t SCORE_KEY[MAX_SCORE + 1];

void init_hashes() {
    for (int i = 0; i < NUM_CARDS; i++) CARD_KEY[i] = random_hash();
    for (int i = 0; i < NUM_PLAYERS; i++) PLAYER_KEY[i] = random_hash();
    for (int i = 0; i <= MAX_SCORE; i++) SCORE_KEY[i] = random_hash();
}