#ifndef __STOP_WATCH__
#define __STOP_WATCH__

#include <cassert>

#include <stdint.h>
#include <time.h>

#define NSEC_PER_SEC 1e9

class stop_watch
{
public:
    stop_watch() {
	assert(clock_gettime(CLOCK_MONOTONIC, &tp) == 0);
    }

    void restart() {
	assert(clock_gettime(CLOCK_MONOTONIC, &tp) == 0);
    }

    uint64_t elapsed_ns() __attribute__((always_inline)) {
	struct timespec now;
	uint64_t diff;

	assert(clock_gettime(CLOCK_MONOTONIC, &now) == 0);
	diff = NSEC_PER_SEC * (int64_t)((int) now.tv_sec - (int) tp.tv_sec);
	diff += ((int) now.tv_nsec - (int) tp.tv_nsec);
	return diff;
    }

private:
    struct timespec tp;
};

#endif
