#pragma once

#include <chrono>
#include <functional>
#include <optional>

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
    void update(int max_loops, std::optional<std::function<void()>> callback);

private:
    std::chrono::time_point<std::chrono::steady_clock> m_start;
    std::chrono::time_point<std::chrono::steady_clock> m_end;
    int64_t m_delta;
    bool m_is_ready;
    int64_t m_rate;
    double m_blend;

    void update_state();
};

}