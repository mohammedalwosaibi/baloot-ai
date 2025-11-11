#include <iostream>
#include <cstdint>
#include <array>
#include <tuple>

uint8_t calculate_score(const std::array<uint8_t, 32>& round) {
    std::cout << +round[0] << std::endl;
    return 100;
}

int main() {
    const std::array<uint8_t, 32> round = {32};
    calculate_score(round);
    return 0;
}