#include "fixed_loop.hpp"

#include <chrono>
#include <iostream>

namespace util {

FixedLoop::FixedLoop(float rate)
{
    m_start = std::chrono::steady_clock::now();
    m_end = std::chrono::steady_clock::now();
    m_delta = 0;
    m_is_ready = false;
    m_rate = static_cast<int64_t>((static_cast<double>(1.0 / rate)) * static_cast<int64_t>(1000000000));
}

void FixedLoop::update()
{
    m_start = std::chrono::steady_clock::now();
    m_delta += std::chrono::duration_cast<std::chrono::nanoseconds>(m_start - m_end).count();
    m_end = m_start;
    if (m_delta >= m_rate) {
        m_is_ready = true;
        m_delta -= m_rate;
    }
    else {
        m_is_ready = false;
    }
    m_blend = static_cast<double>(m_delta) / static_cast<double>(m_rate);
}

bool FixedLoop::is_ready() const
{
    return m_is_ready;
}

void FixedLoop::set_rate(float rate)
{
    m_rate = static_cast<int64_t>((static_cast<double>(1.0f / rate)) * static_cast<int64_t>(1000000000));
}

float FixedLoop::blend() const
{
    return static_cast<float>(m_blend);
}

void FixedLoop::reset()
{
    m_start = std::chrono::steady_clock::now();
    m_end = std::chrono::steady_clock::now();
    m_delta = 0;
    m_is_ready = false;
}

}
