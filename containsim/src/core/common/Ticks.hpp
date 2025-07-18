#pragma once

#include <chrono>
#include <cstdint>

namespace ch = std::chrono;

namespace core {
    class TickController;
}

class TickClockKey {
    TickClockKey() = default;
    TickClockKey(const TickClockKey&) = default;
    
    friend class core::TickController;
};

class TickClock {
public:
    using rep = int64_t;
    // The tick is arbitrary compared to seconds so just make the ratio 1
    using period = std::ratio<1, 1>;
    using duration = ch::duration<rep, period>;
    using time_point = ch::time_point<TickClock>;
    
    static constexpr bool is_steady = true;

private:
    inline static rep m_now{};

public:
    TickClock() = default;

    static time_point now() {
        return time_point(duration(m_now));
    }

    static void SetNow(const TickClockKey&, rep init) {
        m_now = init;
    }

    static void Progress(const TickClockKey&) {
        m_now++;
    }
};

using TickRep = TickClock::rep;
using TickPeriod = TickClock::period;
using TickDuration = TickClock::duration;
using TickPoint = TickClock::time_point;