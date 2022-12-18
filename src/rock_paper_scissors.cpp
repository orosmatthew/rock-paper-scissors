#include "rock_paper_scissors.hpp"

#include <filesystem>
#include <iostream>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#include "fixed_loop.hpp"

namespace rps {

/**
 * @brief Types of pieces
 */
enum class PieceType {
    e_rock,
    e_paper,
    e_scissors,
};

/**
 * @brief Piece state
 */
struct Piece {
    PieceType type;
    raylib::Vector2 prev_vel;
    raylib::Vector2 prev_pos;
    raylib::Vector2 pos;
};

/**
 * @brief Options for simulation
 */
struct Options {
    int piece_count;
    int simulation_rate;
    int piece_size;
};

/**
 * @brief Contains all simulation resources
 */
struct Resources {
    raylib::Sound rock_sound;
    raylib::Sound paper_sound;
    raylib::Sound scissors_sound;

    raylib::Image rock_image;
    raylib::Image paper_image;
    raylib::Image scissors_image;

    raylib::Texture2D rock_texture;
    raylib::Texture2D paper_texture;
    raylib::Texture2D scissors_texture;
};

/**
 * @brief Initialize pieces list
 * @param count - Number of pieces
 * @param screen_width
 * @param screen_height
 * @return - Returns list of pieces
 */
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

/**
 * @brief Calculate new pieces positions
 * @param pieces
 * @param screen_width
 * @param screen_height
 * @param piece_size
 */
void update_pieces_pos(std::vector<Piece>& pieces, int screen_width, int screen_height, int piece_size)
{
    // How many pieces to sample from to calculate movement
    const int samples = 10;
    // Maximum loops in case number of sample cannot be reached
    const int max_loops = static_cast<int>(pieces.size());

    for (Piece& p : pieces) {
        p.prev_vel = p.pos - p.prev_pos;
        p.prev_pos = p.pos;
    }

    for (Piece& p : pieces) {

        // Get the closest piece from a number of samples
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

        // Calculate interactions
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

        // Update positions
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

        // Clamp positions so they cannot leave the screen
        p.pos.x = std::clamp(p.pos.x, 0.0f, static_cast<float>(screen_width) - static_cast<float>(piece_size));
        p.pos.y = std::clamp(p.pos.y, 30.0f, static_cast<float>(screen_height) - static_cast<float>(piece_size));
    }
}

/**
 * @brief Play pieces sounds
 * @param res - Resources struct
 * @param type - Type of piece
 */
void play_piece_sound(Resources& res, PieceType type)
{
    switch (type) {

    case PieceType::e_rock:
        res.rock_sound.Play();
        break;
    case PieceType::e_paper:
        res.paper_sound.Play();
        break;
    case PieceType::e_scissors:
        res.scissors_sound.Play();
        break;
    }
}

/**
 * @brief Calculate pieces collisions and update their types
 * @param pieces
 * @param piece_size
 * @param res - Resources struct
 */
void update_pieces_collision(std::vector<Piece>& pieces, int piece_size, Resources& res)
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
        play_piece_sound(res, PieceType::e_rock);
    }
    if (paper_changed) {
        play_piece_sound(res, PieceType::e_paper);
    }
    if (scissors_changed) {
        play_piece_sound(res, PieceType::e_scissors);
    }
}

/**
 * @brief Get index of selected piece from mouse position
 * @param pieces
 * @param piece_size
 * @param mouse_pos
 * @return - Returns optional with either the index of piece of null if no piece is selected
 */
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

/**
 * @brief Update resources struct with new piece size
 * @param res - Resources to update
 * @param piece_size - New piece size
 */
static void update_resources_piece_size(Resources& res, int piece_size)
{
    raylib::Image rock_image_resized = res.rock_image;
    rock_image_resized.Resize(piece_size, piece_size);

    raylib::Image paper_image_resized = res.paper_image;
    paper_image_resized.Resize(piece_size, piece_size);

    raylib::Image scissors_image_resized = res.scissors_image;
    scissors_image_resized.Resize(piece_size, piece_size);

    res.rock_texture = raylib::Texture2D(rock_image_resized);
    res.paper_texture = raylib::Texture2D(paper_image_resized);
    res.scissors_texture = raylib::Texture2D(scissors_image_resized);
}

/**
 * @brief Initialize resources
 * @param piece_size - Size of piece
 * @return - Returns structure with all resources
 */
static Resources init_resources(int piece_size)
{
    std::filesystem::path res_path = std::filesystem::path(GetApplicationDirectory()) / "res";

    raylib::Image rock_image((res_path / "rock.png").string());
    raylib::Image paper_image((res_path / "paper.png").string());
    raylib::Image scissors_image((res_path / "scissors.png").string());

    Resources res {
        .rock_sound = raylib::Sound((res_path / "rock.wav").string()),
        .paper_sound = raylib::Sound((res_path / "paper.wav").string()),
        .scissors_sound = raylib::Sound((res_path / "scissors.wav").string()),

        .rock_image = rock_image,
        .paper_image = paper_image,
        .scissors_image = scissors_image,
    };

    update_resources_piece_size(res, piece_size);

    return res;
}

/**
 * @brief Update pieces list with new count
 * @param pieces - Original pieces list
 * @param new_count - New number of pieces
 * @param screen_width
 * @param screen_height
 * @return - Returns new list of pieces
 */
static std::vector<Piece> update_piece_count(
    std::vector<Piece>& pieces, int new_count, int screen_width, int screen_height)
{
    std::vector<Piece> new_pieces;
    new_pieces.reserve(new_count);

    if (pieces.size() < new_count) {
        for (Piece& piece : pieces) {
            new_pieces.push_back(piece);
        }
        std::vector<Piece> extras
            = init_pieces(static_cast<int>(new_count - pieces.size()), screen_width, screen_height);
        for (Piece& p : extras) {
            new_pieces.push_back(p);
        }
        return new_pieces;
    }
    else {
        for (int i = 0; i < new_count; i++) {
            new_pieces.push_back(pieces.at(i));
        }
        return new_pieces;
    }
}

void run(const RockPaperScissorsConfig& config)
{
    int screen_width = config.screen_width;
    int screen_height = config.screen_height;

    Options options {
        .piece_count = config.piece_count,
        .simulation_rate = static_cast<int>(config.simulation_rate),
        .piece_size = config.piece_size,
    };

    SetConfigFlags(ConfigFlags::FLAG_VSYNC_HINT);
    SetConfigFlags(ConfigFlags::FLAG_WINDOW_RESIZABLE);

    raylib::Window window(config.screen_width, config.screen_height, "Rock Paper Scissors");

    raylib::AudioDevice audio_device;
    float volume = config.volume;
    audio_device.SetVolume(volume);

    SetExitKey(KEY_ESCAPE);

    util::FixedLoop fixed_loop(static_cast<float>(options.simulation_rate));

    Resources res = init_resources(options.piece_size);

    std::vector<Piece> pieces = init_pieces(options.piece_count, screen_width, screen_height);

    bool is_paused = false;

    // Index of piece if selected by mouse
    std::optional<int> selected_piece_index;

    // Stored for when fullscreen is toggled off
    raylib::Vector2 previous_windowed_size;

    bool hud_shown = true;

    // Fixed timestep for simulation calculations
    fixed_loop.set_callback([&]() {
        if (!is_paused) {
            update_pieces_pos(pieces, screen_width, screen_height, options.piece_size);
            update_pieces_collision(pieces, options.piece_size, res);
        }
    });

    while (!window.ShouldClose()) {

        // Update screen size
        if (window.IsResized()) {
            screen_height = window.GetHeight();
            screen_width = window.GetWidth();
        }

        // Pause with keyboard shortcut
        if (IsKeyPressed(KEY_P)) {
            if (is_paused) {
                is_paused = false;
            }
            else {
                is_paused = true;
            }
        }

        // De-selecting piece with mouse
        if (IsMouseButtonUp(MOUSE_BUTTON_LEFT) && selected_piece_index.has_value()) {
            selected_piece_index.reset();
            raylib::Mouse::SetCursor(MOUSE_CURSOR_DEFAULT);
        }

        // Select piece with mouse
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            selected_piece_index = get_piece_from_click(pieces, options.piece_size, GetMousePosition());
            if (selected_piece_index.has_value()) {
                raylib::Mouse::SetCursor(MOUSE_CURSOR_POINTING_HAND);
            }
        }

        // Dragging piece with mouse
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && selected_piece_index.has_value()) {
            pieces.at(selected_piece_index.value()).pos = raylib::Vector2(GetMousePosition())
                - raylib::Vector2(static_cast<float>(options.piece_size) / 2.0f,
                                  static_cast<float>(options.piece_size) / 2.0f);
        }

        fixed_loop.update();

        // UI elements states
        int new_piece_size;
        int new_rate;
        int new_piece_count;

        bool fullscreen_pressed;
        bool restart_pressed;
        bool hud_pressed;
        bool defaults_pressed;

        BeginDrawing();
        {
            window.ClearBackground(raylib::Color::RayWhite());

            // Get blend value for position interpolation
            float blend = fixed_loop.blend();
            if (is_paused) {
                blend = 1;
            }

            // Draw pieces
            for (Piece& p : pieces) {
                switch (p.type) {
                case PieceType::e_rock:
                    res.rock_texture.Draw(p.prev_pos.Lerp(p.pos, blend));
                    break;
                case PieceType::e_paper:
                    res.paper_texture.Draw(p.prev_pos.Lerp(p.pos, blend));
                    break;
                case PieceType::e_scissors:
                    res.scissors_texture.Draw(p.prev_pos.Lerp(p.pos, blend));
                    break;
                }
            }

            // Draw UI
            if (hud_shown) {
                // Toolbar
                DrawRectangle(0, 0, screen_width, 30, raylib::Color::LightGray());

                // FPS
                raylib::DrawText(std::format("{} FPS", GetFPS()), 10, 6, 20, raylib::Color::DarkGreen());

                const int controls_offset = 125;

                // Pause button
                is_paused = GuiToggle(raylib::Rectangle(controls_offset, 2, 70, 25), "#132#pause", is_paused);

                // Restart button
                restart_pressed = GuiButton(raylib::Rectangle(controls_offset + 80, 2, 70, 25), "#77#restart");

                // Rate slider
                new_rate = static_cast<int>(GuiSlider(
                    raylib::Rectangle(controls_offset + 200, 2, 100, 25),
                    "Rate",
                    "",
                    static_cast<float>(options.simulation_rate),
                    1,
                    250));

                // Count slider
                new_piece_count = static_cast<int>(GuiSlider(
                    raylib::Rectangle(controls_offset + 350, 2, 100, 25),
                    "Count",
                    "",
                    static_cast<float>(options.piece_count),
                    3,
                    1000));

                // Size slider
                new_piece_size = static_cast<int>(GuiSlider(
                    raylib::Rectangle(controls_offset + 500, 2, 100, 25),
                    "Size",
                    "",
                    static_cast<float>(options.piece_size),
                    1,
                    100));

                // Defaults button
                defaults_pressed = GuiButton(raylib::Rectangle(controls_offset + 620, 2, 70, 25), "Defaults");

                // Hide HUD button
                hud_pressed = GuiButton(raylib::Rectangle(static_cast<float>(screen_width - 30), 2, 25, 25), "#44#");

                // Fullscreen button
                fullscreen_pressed
                    = GuiButton(raylib::Rectangle(static_cast<float>(screen_width - 65), 2, 25, 25), "#69#");

                // Volume slider
                volume = GuiSlider(
                    raylib::Rectangle(static_cast<float>(screen_width - 185), 2, 100, 25), "#122#", "", volume, 0, 1);
            }
            else { // If HUD is hidden
                // Show HUD button
                hud_pressed = GuiButton(raylib::Rectangle(static_cast<float>(screen_width - 30), 2, 25, 25), "#45#");
            }
        }
        EndDrawing();

        audio_device.SetVolume(volume);

        if (defaults_pressed) {
            new_rate = static_cast<int>(config.simulation_rate);
            new_piece_size = config.piece_size;
            new_piece_count = config.piece_count;
        }

        // Toggle HUD
        if (hud_pressed || IsKeyPressed(KEY_H)) {
            if (hud_shown) {
                hud_shown = false;
            }
            else {
                hud_shown = true;
            }
        }

        // Restart
        if (restart_pressed || IsKeyPressed(KEY_SPACE)) {
            pieces = init_pieces(options.piece_count, screen_width, screen_height);
        }

        // Toggle fullscreen
        if (fullscreen_pressed || IsKeyPressed(KEY_F)) {
            if (!window.IsFullscreen()) {
                previous_windowed_size = window.GetSize();
                int display = GetCurrentMonitor();
                window.SetSize(GetMonitorWidth(display), GetMonitorHeight(display));
                screen_height = window.GetHeight();
                screen_width = window.GetWidth();
                window.ToggleFullscreen();
            }
            else {
                window.ToggleFullscreen();
                window.SetSize(previous_windowed_size);
            }
        }

        // Simulation rate
        if (new_rate != options.simulation_rate) {
            options.simulation_rate = new_rate;
            fixed_loop.set_rate(static_cast<float>(options.simulation_rate));
        }

        // Piece size
        if (new_piece_size != options.piece_size) {
            options.piece_size = new_piece_size;
            update_resources_piece_size(res, options.piece_size);
        }

        // Piece count
        if (new_piece_count != options.piece_count) {
            options.piece_count = new_piece_count;
            pieces = update_piece_count(pieces, options.piece_count, screen_width, screen_height);
        }
    }
}

}