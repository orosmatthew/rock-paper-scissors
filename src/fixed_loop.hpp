#pragma once

#include <chrono>

class FixedLoop {

public:
    explicit FixedLoop(double rate);

    void set_rate(double rate);

    void reset();

    [[nodiscard]] double blend() const;

    [[nodiscard]] double rate() const;

    FixedLoop(FixedLoop const& other) = delete;
    FixedLoop& operator=(FixedLoop const& other) = delete;

    void update();

    [[nodiscard]] bool is_ready() const;

private:
    std::chrono::time_point<std::chrono::steady_clock> m_start;
    std::chrono::time_point<std::chrono::steady_clock> m_end;
    int64_t m_delta;
    bool m_is_ready;
    int64_t m_rate;
};
