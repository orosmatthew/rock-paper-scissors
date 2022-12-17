#include "rock_paper_scissors.hpp"

#include <filesystem>

#include "fixed_loop.hpp"

namespace rps {

enum class PieceType {
    e_rock,
    e_paper,
    e_scissors,
};

struct Piece {
    PieceType type;
    raylib::Vector2 prev_pos;
    raylib::Vector2 pos;
};

std::vector<Piece> init_pieces(int count, int screen_width, int screen_height)
{
    std::vector<Piece> pieces;
    pieces.reserve(count);

    for (int i = 0; i < count; i++) {

        raylib::Vector2 random_pos(
            static_cast<float>(GetRandomValue(0, screen_width)), static_cast<float>(GetRandomValue(0, screen_height)));

        Piece p {
            .type = static_cast<PieceType>(i % 3),
            .prev_pos = random_pos,
            .pos = random_pos,
        };
        pieces.push_back(p);
    }

    return pieces;
}

void update_pieces_pos(std::vector<Piece>& pieces, int screen_width, int screen_height, int piece_size)
{
    const int sample_count = 10;

    for (Piece& p : pieces) {
        raylib::Vector2 prev_vel = p.pos - p.prev_pos;
        p.prev_pos = p.pos;

        float min_dist = std::numeric_limits<float>::max();
        int min_piece_index;
        for (int i = 0; i < sample_count; i++) {
            int index = GetRandomValue(0, static_cast<int>(pieces.size()) - 1);
            Piece& rand_piece = pieces.at(index);
            float dist = p.pos.DistanceSqr(rand_piece.pos);
            if (dist < min_dist) {
                min_dist = dist;
                min_piece_index = index;
            }
        }

        Piece& p2 = pieces.at(min_piece_index);

        bool is_attracted;
        bool is_same = false;

        switch (p.type) {
        case PieceType::e_rock:
            switch (p2.type) {
            case PieceType::e_rock:
                is_same = true;
                break;
            case PieceType::e_paper:
                is_attracted = false;
                break;
            case PieceType::e_scissors:
                is_attracted = true;
                break;
            }
        case PieceType::e_paper:
            switch (p2.type) {
            case PieceType::e_rock:
                is_attracted = true;
                break;
            case PieceType::e_paper:
                is_same = true;
                break;
            case PieceType::e_scissors:
                is_attracted = false;
                break;
            }
            break;
        case PieceType::e_scissors:
            switch (p2.type) {
            case PieceType::e_rock:
                is_attracted = false;
                break;
            case PieceType::e_paper:
                is_attracted = true;
                break;
            case PieceType::e_scissors:
                is_same = true;
                break;
            }
            break;
        }

        if (is_same) {
            p.pos += prev_vel;
        }
        else {
            const float speed = 1.5;
            if (is_attracted) {
                raylib::Vector2 vel = (p2.pos - p.pos).Normalize() * speed;
                p.pos += vel;
            }
            else {
                raylib::Vector2 vel = (p2.pos - p.pos).Normalize() * speed;
                p.pos -= vel;
            }
        }

        p.pos.x = std::clamp(p.pos.x, 0.0f, static_cast<float>(screen_width) - static_cast<float>(piece_size));
        p.pos.y = std::clamp(p.pos.y, 0.0f, static_cast<float>(screen_height) - static_cast<float>(piece_size));
    }
}

void run(const RockPaperScissorsConfig& config)
{
    SetConfigFlags(ConfigFlags::FLAG_VSYNC_HINT);

    raylib::Window window(config.screen_width, config.screen_height, "Rock Paper Scissors");

    SetExitKey(KEY_NULL);

    std::filesystem::path res_path = std::filesystem::path(GetApplicationDirectory()) / "../" / "res";

    util::FixedLoop fixed_loop(config.simulation_rate);

    raylib::Image rock_image((res_path / "rock.png").string());
    rock_image.Resize(config.piece_size, config.piece_size);

    raylib::Image paper_image((res_path / "paper.png").string());
    paper_image.Resize(config.piece_size, config.piece_size);

    raylib::Image scissors_image((res_path / "scissors.png").string());
    scissors_image.Resize(config.piece_size, config.piece_size);

    raylib::Texture2D rock_texture(rock_image);
    raylib::Texture2D paper_texture(paper_image);
    raylib::Texture2D scissors_texture(scissors_image);

    std::vector<Piece> pieces = init_pieces(config.piece_count, config.screen_width, config.screen_height);

    fixed_loop.set_callback([&]() {
        update_pieces_pos(pieces, config.screen_width, config.screen_height, config.piece_size);
    });

    while (!window.ShouldClose()) {

        if (IsKeyPressed(KEY_SPACE)) {
            pieces = init_pieces(config.piece_count, config.screen_width, config.screen_height);
        }

        fixed_loop.update();

        BeginDrawing();
        {
            window.ClearBackground(raylib::Color::RayWhite());

            for (Piece& p : pieces) {
                switch (p.type) {
                case PieceType::e_rock:
                    rock_texture.Draw(p.prev_pos.Lerp(p.pos, fixed_loop.blend()));
                    break;
                case PieceType::e_paper:
                    paper_texture.Draw(p.prev_pos.Lerp(p.pos, fixed_loop.blend()));
                    break;
                case PieceType::e_scissors:
                    scissors_texture.Draw(p.prev_pos.Lerp(p.pos, fixed_loop.blend()));
                    break;
                }
            }

            DrawFPS(10, 10);
        }
        EndDrawing();
    }
}

}