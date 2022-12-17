#pragma once

#include <raylib-cpp.hpp>

#include "fixed_loop.hpp"

namespace rps {

struct RockPaperScissorsConfig {
    int screen_width;
    int screen_height;
    double simulation_rate;
};

class RockPaperScissors {

public:
    RockPaperScissors(const RockPaperScissorsConfig& config);

    void run();

private:
    const int m_screen_width;
    const int m_screen_height;

    FixedLoop m_fixed_loop;
};

}