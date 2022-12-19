#include <iostream>

#include "rock_paper_scissors.hpp"

int main()
{
    // Initial simulation configuration
    rps::RockPaperScissorsConfig config {
        .screen_width = 1200,
        .screen_height = 800,
        .simulation_rate = 45,
        .piece_size = 28,
        .piece_count = 125,
        .volume = 0.5f,
        .piece_samples = 10,
    };

    // Run game
    try {
        rps::run(config);
    }
    catch (std::exception& e) {
        std::cerr << "[ERROR] " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
