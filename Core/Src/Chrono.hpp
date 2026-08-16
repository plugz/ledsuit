#ifndef __CHRONO_HPP__
#define __CHRONO_HPP__

#include <chrono>
#include <cstdint>

#include "tim.h"

namespace Chrono {

inline void init() {
//    HAL_TIM_Base_Start(&htim5);
}

using Milliseconds = std::chrono::duration<uint32_t, std::ratio<1, 1000>>;
//using Microseconds = std::chrono::duration<uint32_t, std::ratio<1, 1000000>>;

template<typename TDuration>
struct Clock
{
    using duration = TDuration;
    using rep = TDuration::rep;
    using period = TDuration::period;
    using time_point = std::chrono::time_point<Clock>;

    inline static time_point now();
    inline static void delay(duration const& d) {
        auto startPoint = now();
        while (now() - startPoint < d) {}
    }

    static constexpr bool is_steady = false;
};

template<typename TClock>
class Timer {
public:
    Timer(TClock::duration const& period) : _period(period) {}
    Timer() : _period(0) {}

    void reset() {
        _startTime = TClock::now();
    }

    void reset(TClock::duration const& newPeriod) {
        reset();
        _period = newPeriod;
    };

    void advance(TClock::duration const& period) {
        auto const elapsed = elapsedTime();
        auto const advanceTime = elapsed - (elapsed % period);
        _startTime += advanceTime;
    }

    void advance() {
        advance(_period);
    }

    TClock::duration elapsedTime() const {
        return TClock::now() - _startTime;
    }

    bool done(TClock::duration const& period) const {
        // handle overflow automagically thanks to unsigned arithmetic
        return TClock::now() - _startTime >= period;
    }

    bool done() const {
        return done(_period);
    }

    TClock::duration _period;
    TClock::time_point _startTime;

};

template<typename TDuration>
inline void delay(TDuration const& duration) {
    Clock<TDuration>::delay(duration);
}

using MsClock = Clock<Milliseconds>;
// using UsClock = Clock<Microseconds>;

using MsTimePoint = MsClock::time_point;
//using UsTimePoint = UsClock::time_point;

using MsTimer = Timer<MsClock>;
//using UsTimer = Timer<UsClock>;

template<>
inline MsTimePoint MsClock::now() {
    return time_point(duration(HAL_GetTick()));
}

//template<>
//inline UsTimePoint UsClock::now() {
//    return time_point(duration(__HAL_TIM_GET_COUNTER(&htim5)));
//}

} // namespace Chrono

#endif
