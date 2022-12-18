#pragma once

#include <raylib-cpp.hpp>

namespace rps {

struct RockPaperScissorsConfig {
    int screen_width;
    int screen_height;
    float simulation_rate;
    int piece_size;
    int piece_count;
    float volume;
};

void run(const RockPaperScissorsConfig& config);

}