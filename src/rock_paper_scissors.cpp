#include "rock_paper_scissors.hpp"

#include <filesystem>
#include <iostream>

#include "fixed_loop.hpp"

namespace rps {

static raylib::Sound rock_sound;
static raylib::Sound paper_sound;
static raylib::Sound scissors_sound;

enum class PieceType {
    e_rock,
    e_paper,
    e_scissors,
};

struct Piece {
    PieceType type;
    raylib::Vector2 prev_vel;
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
    const int samples = 10;
    const int max_loops = static_cast<int>(pieces.size());

    for (Piece& p : pieces) {
        p.prev_vel = p.pos - p.prev_pos;
        p.prev_pos = p.pos;
    }

    for (Piece& p : pieces) {

        float min_dist = std::numeric_limits<float>::max();
        int min_piece_index = -1;
        int sample_count = 0;
        for (int i = 0; i < max_loops; i++) {
            int index = GetRandomValue(0, static_cast<int>(pieces.size()) - 1);
            Piece& rand_piece = pieces.at(index);
            if (rand_piece.type == p.type) {
                continue;
            }
            sample_count++;
            float dist = p.prev_pos.DistanceSqr(rand_piece.prev_pos);
            if (dist < min_dist) {
                min_dist = dist;
                min_piece_index = index;
            }
            if (sample_count >= samples) {
                break;
            }
        }

        if (min_piece_index == -1) {
            continue;
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
            break;
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
            p.pos += p.prev_vel;
        }
        else {
            const float repel_speed = 1;
            const float attract_speed = 2;
            if (is_attracted) {
                raylib::Vector2 vel = (p2.prev_pos - p.prev_pos).Normalize() * attract_speed;
                p.pos += vel;
            }
            else {
                raylib::Vector2 vel = ((p2.prev_pos - p.prev_pos).Normalize() * repel_speed).Negate();
                p.pos += vel;
            }
        }

        p.pos.x = std::clamp(p.pos.x, 0.0f, static_cast<float>(screen_width) - static_cast<float>(piece_size));
        p.pos.y = std::clamp(p.pos.y, 0.0f, static_cast<float>(screen_height) - static_cast<float>(piece_size));
    }
}

void piece_change_sound(PieceType type)
{
    switch (type) {

    case PieceType::e_rock:
        rock_sound.Play();
        break;
    case PieceType::e_paper:
        paper_sound.Play();
        break;
    case PieceType::e_scissors:
        scissors_sound.Play();
        break;
    }
}

void update_pieces_collision(std::vector<Piece>& pieces, int piece_size)
{
    float inner_padding = static_cast<float>(piece_size) * 0.2f;
    raylib::Vector2 size(
        static_cast<float>(piece_size) - inner_padding, static_cast<float>(piece_size) - inner_padding);

    bool rock_changed = false;
    bool paper_changed = false;
    bool scissors_changed = false;

    for (int p1 = 0; p1 < pieces.size() - 1; p1++) {
        for (int p2 = p1 + 1; p2 < pieces.size(); p2++) {
            raylib::Rectangle p1_rect(pieces.at(p1).pos, size);
            raylib::Rectangle p2_rect(pieces.at(p2).pos, size);

            if (p1_rect.CheckCollision(p2_rect)) {
                switch (pieces.at(p1).type) {
                case PieceType::e_rock:
                    switch (pieces.at(p2).type) {
                    case PieceType::e_rock:
                        break;
                    case PieceType::e_paper:
                        pieces.at(p1).type = PieceType::e_paper;
                        paper_changed = true;
                        break;
                    case PieceType::e_scissors:
                        pieces.at(p2).type = PieceType::e_rock;
                        rock_changed = true;
                        break;
                    }
                    break;
                case PieceType::e_paper:
                    switch (pieces.at(p2).type) {
                    case PieceType::e_rock:
                        pieces.at(p2).type = PieceType::e_paper;
                        paper_changed = true;
                        break;
                    case PieceType::e_paper:
                        break;
                    case PieceType::e_scissors:
                        pieces.at(p1).type = PieceType::e_scissors;
                        scissors_changed = true;
                        break;
                    }
                    break;
                case PieceType::e_scissors:
                    switch (pieces.at(p2).type) {
                    case PieceType::e_rock:
                        pieces.at(p1).type = PieceType::e_rock;
                        rock_changed = true;
                        break;
                    case PieceType::e_paper:
                        pieces.at(p2).type = PieceType::e_scissors;
                        scissors_changed = true;
                        break;
                    case PieceType::e_scissors:
                        break;
                    }
                    break;
                }
            }
        }
    }

    if (rock_changed) {
        piece_change_sound(PieceType::e_rock);
    }
    if (paper_changed) {
        piece_change_sound(PieceType::e_paper);
    }
    if (scissors_changed) {
        piece_change_sound(PieceType::e_scissors);
    }
}

std::optional<int> get_piece_from_click(std::vector<Piece>& pieces, int piece_size, raylib::Vector2 mouse_pos)
{
    raylib::Vector2 size(static_cast<float>(piece_size), static_cast<float>(piece_size));
    int i = 0;
    for (Piece& p : pieces) {
        raylib::Rectangle rect(p.pos, size);
        if (rect.CheckCollision(mouse_pos)) {
            return i;
        }
        i++;
    }
    return {};
}

void run(const RockPaperScissorsConfig& config)
{
    int width = config.screen_width;
    int height = config.screen_height;

    SetConfigFlags(ConfigFlags::FLAG_VSYNC_HINT);
    SetConfigFlags(ConfigFlags::FLAG_WINDOW_RESIZABLE);

    raylib::Window window(config.screen_width, config.screen_height, "Rock Paper Scissors");

    raylib::AudioDevice audio_device;

    audio_device.SetVolume(0.5);

    SetExitKey(KEY_ESCAPE);

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

    rock_sound = raylib::Sound((res_path / "rock.wav").string());
    paper_sound = raylib::Sound((res_path / "paper.wav").string());
    scissors_sound = raylib::Sound((res_path / "scissors.wav").string());

    std::vector<Piece> pieces = init_pieces(config.piece_count, config.screen_width, config.screen_height);

    bool is_paused = false;

    std::optional<int> selected_piece_index;

    fixed_loop.set_callback([&]() {
        if (!is_paused) {
            update_pieces_pos(pieces, width, height, config.piece_size);
            update_pieces_collision(pieces, config.piece_size);
        }
    });

    while (!window.ShouldClose()) {

        if (IsKeyPressed(KEY_SPACE)) {
            pieces = init_pieces(config.piece_count, width, height);
        }

        if (window.IsResized()) {
            height = window.GetHeight();
            width = window.GetWidth();
        }

        if (IsKeyPressed(KEY_F)) {
            int display = GetCurrentMonitor();
            window.SetSize(GetMonitorWidth(display), GetMonitorHeight(display));
            height = window.GetHeight();
            width = window.GetWidth();
            window.ToggleFullscreen();
        }

        if (IsKeyPressed(KEY_P)) {
            if (is_paused) {
                is_paused = false;
            }
            else {
                is_paused = true;
            }
        }

        if (IsMouseButtonUp(MOUSE_BUTTON_LEFT) && selected_piece_index.has_value()) {
            selected_piece_index.reset();
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            selected_piece_index = get_piece_from_click(pieces, config.piece_size, GetMousePosition());
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && selected_piece_index.has_value()) {
            pieces.at(selected_piece_index.value()).pos = raylib::Vector2(GetMousePosition())
                - raylib::Vector2(static_cast<float>(config.piece_size) / 2.0f,
                                  static_cast<float>(config.piece_size) / 2.0f);
        }

        fixed_loop.update();

        BeginDrawing();
        {
            window.ClearBackground(raylib::Color::RayWhite());

            float blend = fixed_loop.blend();
            if (is_paused) {
                blend = 1;
            }

            for (Piece& p : pieces) {
                switch (p.type) {
                case PieceType::e_rock:
                    rock_texture.Draw(p.prev_pos.Lerp(p.pos, blend));
                    break;
                case PieceType::e_paper:
                    paper_texture.Draw(p.prev_pos.Lerp(p.pos, blend));
                    break;
                case PieceType::e_scissors:
                    scissors_texture.Draw(p.prev_pos.Lerp(p.pos, blend));
                    break;
                }
            }

            DrawFPS(10, 10);
        }
        EndDrawing();
    }
}

}