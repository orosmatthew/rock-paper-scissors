#include <iostream>

#include "rock_paper_scissors.hpp"

int main()
{
    rps::RockPaperScissorsConfig config {
        .screen_width = 1200,
        .screen_height = 800,
        .simulation_rate = 30,
        .piece_size = 27,
        .piece_count = 125,
    };

    try {
        rps::run(config);
    }
    catch (std::exception& e) {
        std::cerr << "[ERROR] " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
