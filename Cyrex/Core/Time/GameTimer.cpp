#include "GameTimer.h"

Cyrex::GameTimer::GameTimer() noexcept {
    m_oldTimePoint = std::chrono::high_resolution_clock::now();
}

void Cyrex::GameTimer::Tick() noexcept {
    auto newTimePoint = std::chrono::high_resolution_clock::now();
    m_delta = newTimePoint - m_oldTimePoint;
    m_total += m_delta;
    m_oldTimePoint = newTimePoint;
}

void Cyrex::GameTimer::Reset() noexcept {
    m_oldTimePoint = std::chrono::high_resolution_clock::now();
    m_delta = std::chrono::high_resolution_clock::duration();
    m_total = std::chrono::high_resolution_clock::duration();
}

void Cyrex::GameTimer::ResetDelta() noexcept {
    m_delta = std::chrono::high_resolution_clock::duration();
}

double Cyrex::GameTimer::GetDeltaNanoseconds() const noexcept {
    return std::chrono::duration<double,std::nano>(m_delta).count();
}

double Cyrex::GameTimer::GetDeltaMicroseconds() const noexcept {
    return std::chrono::duration<double, std::micro>(m_delta).count();
}

double Cyrex::GameTimer::GetDeltaMilliseconds() const noexcept {
    return std::chrono::duration<double, std::milli>(m_delta).count();
}

double Cyrex::GameTimer::GetDeltaSeconds() const noexcept {
    return std::chrono::duration<double, std::ratio<1>> (m_delta).count();
}

double Cyrex::GameTimer::GetTotalNanoseconds() const noexcept {
    return std::chrono::duration<double, std::nano>(m_total).count();
}

double Cyrex::GameTimer::GetTotalMicroseconds() const noexcept {
    return std::chrono::duration<double, std::micro>(m_total).count();
}

double Cyrex::GameTimer::GetTotalMilliSeconds() const noexcept {
    return std::chrono::duration<double, std::milli>(m_total).count();
}

double Cyrex::GameTimer::GetTotalSeconds() const noexcept {
    return std::chrono::duration<double, std::ratio<1>>(m_total).count();
}
