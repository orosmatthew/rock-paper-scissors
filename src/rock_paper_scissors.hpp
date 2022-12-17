#pragma once

#include <raylib-cpp.hpp>

namespace rps {

struct RockPaperScissorsConfig {
    int screen_width;
    int screen_height;
    float simulation_rate;
    int piece_size;
};

void run(const RockPaperScissorsConfig& config);

}