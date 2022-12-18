#pragma once

#include <raylib-cpp.hpp>

namespace rps {

/**
 * @brief Initial simulation configuration
 */
struct RockPaperScissorsConfig {
    int screen_width;
    int screen_height;
    float simulation_rate;
    int piece_size;
    int piece_count;
    float volume;
};

/**
 * @brief Run simulation
 * @param config - Initial configuration to start
 */
void run(const RockPaperScissorsConfig& config);

}