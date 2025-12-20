#include "constants.h"
#include <random>

uint64_t CARD_KEY[NUM_CARDS];
uint64_t PLAYER_KEY[NUM_PLAYERS];
uint64_t SCORE_KEY[MAX_SCORE + 1];

static uint64_t random_hash() {
    static std::mt19937_64 rng(0); // fixed seed
    return rng();
}

void init_hashes() {
    for (int i = 0; i < NUM_CARDS; i++) CARD_KEY[i] = random_hash();
    for (int i = 0; i < NUM_PLAYERS; i++) PLAYER_KEY[i] = random_hash();
    for (int i = 0; i <= MAX_SCORE; i++) SCORE_KEY[i] = random_hash();
}