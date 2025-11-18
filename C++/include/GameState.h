#pragma once
#include <array>
#include <cstdint>

class GameState {
public:
    GameState(const std::array<std::array<uint8_t, 8>, 4>& player_cards);
    void view_player_cards();

private:
    std::array<std::array<uint8_t, 8>, 4> player_cards_;
};