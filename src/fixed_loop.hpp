#pragma once

#include <chrono>
#include <functional>

namespace util {

/**
 * @brief A fixed timestep loop that uses a callback
 */
class FixedLoop {

public:
    /**
     * @brief Construct FixedLoop
     * @param rate - Rate (Steps per second)
     */
    explicit FixedLoop(float rate);

    /**
     * @brief Set rate
     * @param rate - Rate (Steps per second)
     */
    void set_rate(float rate);

    /**
     * @brief Set callback
     * @param callback - Function for callback
     */
    void set_callback(std::function<void()> callback);

    /**
     * @brief Clear current callback
     */
    void remove_callback();

    /**
     * @brief Reset time delta (Used in case timestep is too far behind)
     */
    void reset();

    /**
     * @brief Get blend interpolation fraction (Useful for movement interpolation between steps)
     * @return - Returns blend fraction between 0.0f and 1.0f
     */
    [[nodiscard]] float blend() const;

    FixedLoop(FixedLoop const& other) = delete;
    FixedLoop& operator=(FixedLoop const& other) = delete;

    /**
     * @brief Update loop and callback
     */
    void update();

private:
    std::chrono::time_point<std::chrono::steady_clock> m_start;
    std::chrono::time_point<std::chrono::steady_clock> m_end;
    int64_t m_delta;
    bool m_is_ready;
    int64_t m_rate;
    double m_blend;
    std::optional<std::function<void()>> m_callback;

    void update_state();
};

}