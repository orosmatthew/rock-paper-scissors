#pragma once

#include <raylib-cpp.hpp>

namespace rps {

struct RockPaperScissorsConfig {
    int screen_width;
    int screen_height;
    float simulation_rate;
};

class RockPaperScissors {

public:
    explicit RockPaperScissors(const RockPaperScissorsConfig& config);

    void run();

private:
    const int m_screen_width;
    const int m_screen_height;
    const float m_simulation_rate;
};

}