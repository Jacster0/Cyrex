#pragma once
#include <chrono>

namespace Cyrex {
    class GameTimer {
    public:
        GameTimer() noexcept;
    public:
        void Tick() noexcept;
        void Reset() noexcept;
        void ResetDelta() noexcept;
    public:
        double GetDeltaNanoseconds() const noexcept;
        double GetDeltaMicroseconds() const noexcept;
        double GetDeltaMilliseconds() const noexcept;
        double GetDeltaSeconds() const noexcept;

        double GetTotalNanoseconds() const noexcept;
        double GetTotalMicroseconds() const noexcept;
        double GetTotalMilliSeconds() const noexcept;
        double GetTotalSeconds() const noexcept;
    private:
        std::chrono::high_resolution_clock::time_point m_oldTimePoint;
        std::chrono::high_resolution_clock::duration m_delta{0};
        std::chrono::high_resolution_clock::duration m_total{0};
    };
}
