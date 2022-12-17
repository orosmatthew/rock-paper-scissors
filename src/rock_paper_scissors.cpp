#include "rock_paper_scissors.hpp"

namespace rps {

RockPaperScissors::RockPaperScissors(const RockPaperScissorsConfig& config)
    : m_screen_width(config.screen_width)
    , m_screen_height(config.screen_height)
    , m_fixed_loop(config.simulation_rate)
{
}

void RockPaperScissors::run()
{
    SetConfigFlags(ConfigFlags::FLAG_VSYNC_HINT);

    raylib::Window window(m_screen_width, m_screen_height, "raylib [core] example - basic window");

    m_fixed_loop.reset();

    while (!window.ShouldClose()) {
        m_fixed_loop.update();
        while (m_fixed_loop.is_ready()) {

            m_fixed_loop.update();
        }

        BeginDrawing();
        {
            window.ClearBackground(raylib::Color::RayWhite());
            DrawText("Congrats! You created your first window!", 190, 200, 20, raylib::Color::LightGray());
            DrawFPS(10, 10);
        }
        EndDrawing();
    }
}

}