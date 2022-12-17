#include "rock_paper_scissors.hpp"

#include <filesystem>

#include "fixed_loop.hpp"

namespace rps {

RockPaperScissors::RockPaperScissors(const RockPaperScissorsConfig& config)
    : m_screen_width(config.screen_width)
    , m_screen_height(config.screen_height)
    , m_simulation_rate(config.simulation_rate)
{
}

void RockPaperScissors::run()
{
    SetConfigFlags(ConfigFlags::FLAG_VSYNC_HINT);

    raylib::Window window(m_screen_width, m_screen_height, "Rock Paper Scissors");

    std::filesystem::path res_path = std::filesystem::path(GetApplicationDirectory()) / "../" / "res";

    util::FixedLoop fixed_loop(m_simulation_rate);

    raylib::Image scissors_image((res_path / "rock.png").string());
    scissors_image.Resize(40, 40);

    raylib::Texture2D scissors_texture(scissors_image);

    raylib::Vector2 prev_pos;
    raylib::Vector2 pos(50, 50);

    fixed_loop.set_callback([&]() {
        prev_pos = pos;
        pos.x += 1;
    });

    while (!window.ShouldClose()) {

        fixed_loop.update();

        BeginDrawing();
        {
            window.ClearBackground(raylib::Color::RayWhite());

            scissors_texture.Draw(prev_pos.Lerp(pos, fixed_loop.blend()));

            DrawFPS(10, 10);
        }
        EndDrawing();
    }
}

}