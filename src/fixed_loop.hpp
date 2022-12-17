#pragma once

#include <chrono>
#include <functional>

namespace util {

class FixedLoop {

public:
    explicit FixedLoop(float rate);

    void set_rate(float rate);

    void set_callback(std::function<void()> callback);

    void remove_callback();

    void reset();

    [[nodiscard]] float blend() const;

    FixedLoop(FixedLoop const& other) = delete;
    FixedLoop& operator=(FixedLoop const& other) = delete;

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