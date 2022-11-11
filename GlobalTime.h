#ifndef GLOBAL_TIME_H
#define GLOBAL_TIME_H

#include <cstdint>

class GlobalTime {
public:
    uint64_t getCurrTime() const {
        return curr_time;
    }

    void tick() {
        ++curr_time;
    }

    static GlobalTime &Instance() {
        static GlobalTime time;
        return time;
    }

private:
    GlobalTime()
        : curr_time(0)
    {
    }
    GlobalTime(const GlobalTime &) = delete;
    GlobalTime &operator =(const GlobalTime &) = delete;

    uint64_t curr_time;
};

#endif /* GLOBAL_TIME_H */