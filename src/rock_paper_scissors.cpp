#include "rock_paper_scissors.hpp"

#include <filesystem>
#include <optional>

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
    raylib::Vector2 prev_pos;
    raylib::Vector2 pos;
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
 * @brief UI states
 */
struct UIStates {
    int piece_size;
    int rate;
    int piece_count;

    bool fullscreen_pressed;
    bool restart_pressed;
    bool hud_pressed;
    bool defaults_pressed;
};

/**
 * @brief Options for simulation
 */
struct GameState {
    int piece_count;
    int simulation_rate;
    int piece_size;

    int screen_width;
    int screen_height;

    bool is_paused;
    bool hud_shown;
    float volume;

    UIStates ui_states;

    std::vector<Piece> pieces;
    Resources resources;

    // Selected piece by mouse
    std::optional<int> selected_piece_index;

    // Previous windowed size for when fullscreen is toggled off
    raylib::Vector2 previous_windowed_size;
};

/**
 * @brief Initialize pieces list
 * @param count - Number of pieces
 * @param screen_width
 * @param screen_height
 * @return - Returns list of pieces
 */
static std::vector<Piece> init_pieces(int count, int screen_width, int screen_height)
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
 * @brief Gets closest piece from a number of random samples
 * @param pieces - Pieces list
 * @param piece_index - Piece to search from
 * @param samples - Number of samples to search
 * @return - Returns index of estimated random piece or null if one could not be found
 */
static std::optional<int> estimate_closest_diff_piece(std::vector<Piece>& pieces, int piece_index, int samples)
{
    float min_dist = std::numeric_limits<float>::max();
    std::optional<int> min_piece_index;
    int sample_count = 0;
    for (int i = 0; i < pieces.size(); i++) {
        // Get random piece
        int rand_index = GetRandomValue(0, static_cast<int>(pieces.size()) - 1);
        Piece& rand_piece = pieces.at(rand_index);

        // If same type, skip
        if (rand_piece.type == pieces.at(piece_index).type) {
            continue;
        }
        sample_count++;
        float dist = pieces.at(piece_index).prev_pos.DistanceSqr(rand_piece.prev_pos);
        if (dist < min_dist) {
            min_dist = dist;
            min_piece_index = rand_index;
        }
        if (sample_count >= samples) {
            break;
        }
    }

    return min_piece_index;
}

/**
 * @brief Determine if pieces are attracted
 * @param p1 - Piece 1
 * @param p2 - Piece 2
 * @return - Returns a bool optional, true if attracted, false if repelled, null if no interaction
 */
static std::optional<bool> are_pieces_attracted(const Piece& p1, const Piece& p2)
{
    switch (p1.type) {
    case PieceType::e_rock:
        switch (p2.type) {
        case PieceType::e_rock:
            return {};
        case PieceType::e_paper:
            return false;
        case PieceType::e_scissors:
            return true;
        }
    case PieceType::e_paper:
        switch (p2.type) {
        case PieceType::e_rock:
            return true;
        case PieceType::e_paper:
            return {};
        case PieceType::e_scissors:
            return false;
        }
    case PieceType::e_scissors:
        switch (p2.type) {
        case PieceType::e_rock:
            return false;
        case PieceType::e_paper:
            return true;
        case PieceType::e_scissors:
            return {};
        }
    }
    return {};
}

/**
 * @brief Calculate new pieces positions
 * @param pieces
 * @param screen_width
 * @param screen_height
 * @param piece_size
 */
static void update_pieces_pos(
    std::vector<Piece>& pieces,
    int screen_width,
    int screen_height,
    int piece_size,
    int close_samples,
    bool is_hud_shown)
{
    // Update previous positions before updating them
    for (Piece& p : pieces) {
        p.prev_pos = p.pos;
    }

    for (int i = 0; i < pieces.size(); i++) {
        // Get the closest different piece from a number of samples
        std::optional<int> min_piece_index = estimate_closest_diff_piece(pieces, i, close_samples);

        // If a close piece cannot be found
        if (!min_piece_index.has_value()) {
            continue;
        }

        Piece& p1 = pieces.at(i);
        Piece& p2 = pieces.at(min_piece_index.value());

        // Calculate interaction
        std::optional<bool> is_attracted = are_pieces_attracted(p1, p2);

        // If pieces are the same, skip
        if (!is_attracted.has_value()) {
            continue;
        }

        const float repel_speed = 1;
        const float attract_speed = 2;

        if (is_attracted.value()) {
            raylib::Vector2 vel = (p2.prev_pos - p1.prev_pos).Normalize() * attract_speed;
            p1.pos += vel;
        }
        else {
            raylib::Vector2 vel = ((p2.prev_pos - p1.prev_pos).Normalize() * repel_speed).Negate();
            p1.pos += vel;
        }

        // Clamp positions so they cannot leave the screen
        p1.pos.x = std::clamp(p1.pos.x, 0.0f, static_cast<float>(screen_width) - static_cast<float>(piece_size));
        float hud_offset = 30.0f;
        if (!is_hud_shown) {
            hud_offset = 0.0f;
        }
        p1.pos.y = std::clamp(p1.pos.y, hud_offset, static_cast<float>(screen_height) - static_cast<float>(piece_size));
    }
}

/**
 * @brief Play pieces sounds
 * @param res - Resources struct
 * @param type - Type of piece
 */
static void play_piece_sound(Resources& res, PieceType type)
{
    switch (type) {
    case PieceType::e_rock:
        res.rock_sound.Play();
        return;
    case PieceType::e_paper:
        res.paper_sound.Play();
        return;
    case PieceType::e_scissors:
        res.scissors_sound.Play();
        return;
    }
}

/**
 * @brief Update pieces if they collide
 * @param p1 - Piece 1
 * @param p2 - Piece 2
 * @param piece_size - Size of piece
 * @param res - Resources for playing sounds
 */
static void update_piece_types(Piece& p1, Piece& p2, int piece_size, Resources& res)
{
    // Quick exit if pieces are far apart
    if (p1.pos.DistanceSqr(p2.pos) > (powf(static_cast<float>(piece_size), 2) * 2)) {
        return;
    }

    const float inner_padding = static_cast<float>(piece_size) * 0.15f;
    const raylib::Vector2 piece_size_vec(
        static_cast<float>(piece_size) - inner_padding, static_cast<float>(piece_size) - inner_padding);

    const raylib::Rectangle p1_rect(p1.pos, piece_size_vec);
    const raylib::Rectangle p2_rect(p2.pos, piece_size_vec);

    if (!p1_rect.CheckCollision(p2_rect)) {
        return;
    }

    switch (p1.type) {
    case PieceType::e_rock:
        switch (p2.type) {
        case PieceType::e_rock:
            return;
        case PieceType::e_paper:
            p1.type = PieceType::e_paper;
            play_piece_sound(res, PieceType::e_paper);
            return;
        case PieceType::e_scissors:
            p2.type = PieceType::e_rock;
            play_piece_sound(res, PieceType::e_rock);
            return;
        }
    case PieceType::e_paper:
        switch (p2.type) {
        case PieceType::e_rock:
            p2.type = PieceType::e_paper;
            play_piece_sound(res, PieceType::e_paper);
            return;
        case PieceType::e_paper:
            return;
        case PieceType::e_scissors:
            p1.type = PieceType::e_scissors;
            play_piece_sound(res, PieceType::e_scissors);
            return;
        }
    case PieceType::e_scissors:
        switch (p2.type) {
        case PieceType::e_rock:
            p1.type = PieceType::e_rock;
            play_piece_sound(res, PieceType::e_rock);
            return;
        case PieceType::e_paper:
            p2.type = PieceType::e_scissors;
            play_piece_sound(res, PieceType::e_scissors);
            return;
        case PieceType::e_scissors:
            return;
        }
    }
}

/**
 * @brief Loop through all pairs from vector and call function
 * @tparam T - Vector type
 * @param vec - Vector
 * @param func - Function
 */
template <typename T>
static void for_all_pairs(std::vector<T>& vec, std::function<void(T&, T&)> func)
{
    for (int i = 0; i < vec.size() - 1; i++) {
        for (int j = i + 1; j < vec.size(); j++) {
            std::invoke(func, vec.at(i), vec.at(j));
        }
    }
}

/**
 * @brief Get index of selected piece from mouse position
 * @param pieces
 * @param piece_size
 * @param mouse_pos
 * @return - Returns optional with either the index of piece of null if no piece is selected
 */
static std::optional<int> get_piece_from_click(std::vector<Piece>& pieces, int piece_size, raylib::Vector2 mouse_pos)
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

/**
 * @brief Draw pieces
 * @param pieces - Pieces list
 * @param res - Resources for piece textures
 * @param blend - Blend fraction for position interpolation
 */
static void draw_pieces(std::vector<Piece>& pieces, Resources& res, float blend)
{
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
}

/**
 * @brief Draw HUD at the top of the screen
 * @param game_state
 * @param ui_states
 */
static void draw_hud(GameState& game_state, UIStates& ui_states)
{
    // Toolbar
    DrawRectangle(0, 0, game_state.screen_width, 30, raylib::Color::LightGray());

    // FPS
    raylib::DrawText(std::to_string(GetFPS()) + " FPS", 10, 6, 20, raylib::Color::DarkGreen());

    const int controls_offset = 125;

    // Pause button
    game_state.is_paused = GuiToggle(raylib::Rectangle(controls_offset, 2, 70, 25), "#132#pause", game_state.is_paused);

    // Restart button
    ui_states.restart_pressed = GuiButton(raylib::Rectangle(controls_offset + 80, 2, 70, 25), "#77#restart");

    // Rate slider
    ui_states.rate = static_cast<int>(GuiSlider(
        raylib::Rectangle(controls_offset + 200, 2, 100, 25),
        "Rate",
        "",
        static_cast<float>(game_state.simulation_rate),
        1,
        250));

    // Count slider
    ui_states.piece_count = static_cast<int>(GuiSlider(
        raylib::Rectangle(controls_offset + 350, 2, 100, 25),
        "Count",
        "",
        static_cast<float>(game_state.piece_count),
        3,
        1000));

    // Size slider
    ui_states.piece_size = static_cast<int>(GuiSlider(
        raylib::Rectangle(controls_offset + 500, 2, 100, 25),
        "Size",
        "",
        static_cast<float>(game_state.piece_size),
        1,
        100));

    // Defaults button
    ui_states.defaults_pressed = GuiButton(raylib::Rectangle(controls_offset + 620, 2, 70, 25), "Defaults");

    // Hide HUD button
    ui_states.hud_pressed
        = GuiButton(raylib::Rectangle(static_cast<float>(game_state.screen_width - 30), 2, 25, 25), "#44#");

    // Fullscreen button
    ui_states.fullscreen_pressed
        = GuiButton(raylib::Rectangle(static_cast<float>(game_state.screen_width - 65), 2, 25, 25), "#69#");

    // Volume slider
    game_state.volume = GuiSlider(
        raylib::Rectangle(static_cast<float>(game_state.screen_width - 185), 2, 100, 25),
        "#122#",
        "",
        game_state.volume,
        0,
        1);
}

/**
 * @brief Main game loop
 * @param config
 * @param window
 * @param audio_device
 * @param fixed_loop
 * @param game_state
 */
static void main_loop(
    const RockPaperScissorsConfig& config,
    raylib::Window& window,
    raylib::AudioDevice& audio_device,
    util::FixedLoop& fixed_loop,
    GameState& game_state)
{
    // Update screen size
    if (window.IsResized()) {
        game_state.screen_height = window.GetHeight();
        game_state.screen_width = window.GetWidth();
    }

    // Pause with keyboard shortcut
    if (IsKeyPressed(KEY_P)) {
        if (game_state.is_paused) {
            game_state.is_paused = false;
        }
        else {
            game_state.is_paused = true;
        }
    }

    fixed_loop.update(20, [&]() {
        if (game_state.is_paused) {
            return;
        }
        update_pieces_pos(
            game_state.pieces,
            game_state.screen_width,
            game_state.screen_height,
            game_state.piece_size,
            config.piece_samples,
            game_state.hud_shown);
        for_all_pairs<Piece>(game_state.pieces, [&](Piece& p1, Piece& p2) {
            update_piece_types(p1, p2, game_state.piece_size, game_state.resources);
        });
    });

    // De-selecting piece with mouse
    if (IsMouseButtonUp(MOUSE_BUTTON_LEFT) && game_state.selected_piece_index.has_value()) {
        game_state.selected_piece_index.reset();
        raylib::Mouse::SetCursor(MOUSE_CURSOR_DEFAULT);
    }

    // Select piece with mouse
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        game_state.selected_piece_index
            = get_piece_from_click(game_state.pieces, game_state.piece_size, GetMousePosition());
        if (game_state.selected_piece_index.has_value()) {
            raylib::Mouse::SetCursor(MOUSE_CURSOR_POINTING_HAND);
        }
    }

    // Dragging piece with mouse
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && game_state.selected_piece_index.has_value()) {
        const raylib::Vector2 piece_middle(
            static_cast<float>(game_state.piece_size) / 2.0f, static_cast<float>(game_state.piece_size) / 2.0f);
        game_state.pieces.at(game_state.selected_piece_index.value()).pos
            = raylib::Vector2(GetMousePosition()) - piece_middle;
    }

    BeginDrawing();
    {
        window.ClearBackground(raylib::Color::RayWhite());

        // Get blend value for position interpolation
        float blend = fixed_loop.blend();
        if (game_state.is_paused) {
            blend = 1.0f;
        }

        draw_pieces(game_state.pieces, game_state.resources, blend);

        // Draw UI
        if (game_state.hud_shown) {
            draw_hud(game_state, game_state.ui_states);
        }
        else {
            raylib::Rectangle hud_show_rect(static_cast<float>(game_state.screen_width - 30), 2, 25, 25);
            game_state.ui_states.hud_pressed = GuiButton(hud_show_rect, "#45#");
        }
    }
    EndDrawing();

    audio_device.SetVolume(game_state.volume);

    // Defaults
    if (game_state.ui_states.defaults_pressed) {
        game_state.ui_states.rate = static_cast<int>(config.simulation_rate);
        game_state.ui_states.piece_size = config.piece_size;
        game_state.ui_states.piece_count = config.piece_count;
    }

    // Toggle HUD
    if (game_state.ui_states.hud_pressed || IsKeyPressed(KEY_H)) {
        if (game_state.hud_shown) {
            game_state.hud_shown = false;
        }
        else {
            game_state.hud_shown = true;
        }
    }

    // Restart
    if (game_state.ui_states.restart_pressed || IsKeyPressed(KEY_SPACE)) {
        game_state.pieces = init_pieces(game_state.piece_count, game_state.screen_width, game_state.screen_height);
    }

    // Toggle fullscreen
    if (game_state.ui_states.fullscreen_pressed || IsKeyPressed(KEY_F)) {
        if (!window.IsFullscreen()) {
            game_state.previous_windowed_size = window.GetSize();
            int display = GetCurrentMonitor();
            window.SetSize(GetMonitorWidth(display), GetMonitorHeight(display));
            game_state.screen_height = window.GetHeight();
            game_state.screen_width = window.GetWidth();
            window.ToggleFullscreen();
        }
        else {
            window.ToggleFullscreen();
            window.SetSize(game_state.previous_windowed_size);
        }
    }

    // Simulation rate
    if (game_state.ui_states.rate != game_state.simulation_rate) {
        game_state.simulation_rate = game_state.ui_states.rate;
        fixed_loop.set_rate(static_cast<float>(game_state.simulation_rate));
    }

    // Piece size
    if (game_state.ui_states.piece_size != game_state.piece_size) {
        game_state.piece_size = game_state.ui_states.piece_size;
        update_resources_piece_size(game_state.resources, game_state.piece_size);
    }

    // Piece count
    if (game_state.ui_states.piece_count != game_state.piece_count) {
        game_state.piece_count = game_state.ui_states.piece_count;
        game_state.pieces = update_piece_count(
            game_state.pieces, game_state.piece_count, game_state.screen_width, game_state.screen_height);
    }
}

void run(const RockPaperScissorsConfig& config)
{
    GameState game_state {};

    game_state.piece_count = config.piece_count;
    game_state.simulation_rate = static_cast<int>(config.simulation_rate);
    game_state.piece_size = config.piece_size;
    game_state.is_paused = false;
    game_state.hud_shown = true;
    game_state.volume = 0.5f;
    game_state.selected_piece_index = {};

    SetConfigFlags(ConfigFlags::FLAG_VSYNC_HINT);
    SetConfigFlags(ConfigFlags::FLAG_WINDOW_RESIZABLE);

    raylib::Window window(config.screen_width, config.screen_height, "Rock Paper Scissors");

    game_state.screen_width = window.GetWidth();
    game_state.screen_height = window.GetHeight();

    game_state.previous_windowed_size = window.GetSize();

    raylib::AudioDevice audio_device;
    float volume = config.volume;
    audio_device.SetVolume(volume);

    SetExitKey(KEY_ESCAPE);

    util::FixedLoop fixed_loop(static_cast<float>(game_state.simulation_rate));

    game_state.resources = init_resources(game_state.piece_size);

    game_state.pieces = init_pieces(game_state.piece_count, game_state.screen_width, game_state.screen_height);

    while (!window.ShouldClose()) {
        main_loop(config, window, audio_device, fixed_loop, game_state);
    }
}

}