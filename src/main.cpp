#include <iostream>

#include "rock_paper_scissors.hpp"

int main()
{
    rps::RockPaperScissorsConfig config {
        .screen_width = 1200,
        .screen_height = 800,
        .simulation_rate = 60,
    };

    rps::RockPaperScissors game(config);

    try {
        game.run();
    }
    catch (std::exception& e) {
        std::cerr << "[ERROR] " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
