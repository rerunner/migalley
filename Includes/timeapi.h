#ifndef __TIMEAPI_H__
#define __TIMEAPI_H__

/*
 * Portable replacement for Windows <timeapi.h>
 * For use on Linux with GCC (-m32 or -m64).
 *
 * Provides:
 *   - TIMECAPS struct
 *   - timeGetTime()
 *   - timeBeginPeriod() / timeEndPeriod()
 *   - timeGetDevCaps()
 *
 * These are stubs using POSIX clock_gettime().
 */

#include <stdint.h>
#include <time.h>

/* Windows-style typedefs */
typedef uint32_t MMRESULT;
typedef uint32_t UINT;
//typedef uint32_t DWORD;

/* Return codes */
#define TIMERR_NOERROR 0
#define TIMERR_NOCANDO 1

/* TIMECAPS structure */
typedef struct timecaps_tag {
    UINT wPeriodMin;
    UINT wPeriodMax;
} TIMECAPS, *LPTIMECAPS;

/* ------------------------------------------------------------------
   Portable implementations
   ------------------------------------------------------------------ */

/* timeGetTime: returns system uptime in milliseconds */
static inline DWORD timeGetTime(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (DWORD)((ts.tv_sec * 1000u) + (ts.tv_nsec / 1000000u));
}

/* timeBeginPeriod / timeEndPeriod:
   On Windows, these request higher timer resolution.
   On Linux, we can ignore and return success. */
static inline MMRESULT timeBeginPeriod(UINT uPeriod) {
    (void)uPeriod;
    return TIMERR_NOERROR;
}

static inline MMRESULT timeEndPeriod(UINT uPeriod) {
    (void)uPeriod;
    return TIMERR_NOERROR;
}

/* timeGetDevCaps: reports timer resolution capabilities */
static inline MMRESULT timeGetDevCaps(LPTIMECAPS ptc, UINT cbtc) {
    if (!ptc || cbtc < sizeof(TIMECAPS))
        return TIMERR_NOCANDO;

    ptc->wPeriodMin = 1;    /* 1 ms resolution */
    ptc->wPeriodMax = 1000; /* 1 second */
    return TIMERR_NOERROR;
}

#endif /* __TIMEAPI_H__ */
