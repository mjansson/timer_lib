/* timer.h  -  v0.6  -  Public Domain  -  2011 Mattias Jansson / Rampant Pixels
 * 
 * This library provides a cross-platform interface to measure
 * elapsed time with (at least) millisecond accuracy.
 *
 * This library is put in the public domain; you can redistribute
 * it and/or modify it without any restrictions.
 *
 * VERSION HISTORY
 *
 * 0.1  (2011-03-15)  Initial release
 * 0.2  (2011-03-20)  Removed timer type (always high precision)
 *                    Fixed timer_ticks_per_second declaration
 *                    Added test application
 * 0.3  (2011-03-22)  Removed unused error checks in POSIX code
 *                    Made timeGetTime fallback optional in Windows code (define USE_FALLBACK to 1)
 *                    Fixed check of QPC weirdness (signed issue)
 * 0.4  (2011-03-23)  timer_lib_initialize() returns non-zero if failed (no high precision timer available)
 *                    Changed POSIX path to use CLOCK_MONOTONIC
 *                    POSIX timer_system use CLOCK_REALTIME for actual timestamp
 *                    Addded Mach-O path for MacOS X
 *                    Changed POSIX path to use nanosecond frequency as returned by clock_gettime
 * 0.5  (2012-10-01)  Merged (cleaned up) MacOSX build fixes from Nicolas Léveillé
 * 0.6  (2013-01-11)  Simplified API, only using tick_t type as timestamp and removing custom timer struct
 *                    Removed old legacy fallback code for Windows platform
 */

#pragma once

/*! \file timer.h
    Timer measuring deltatimes */

#if TIMER_COMPILE
#define TIMER_API
#else
#if __cplusplus
#define TIMER_API extern "C"
#else
#define TIMER_API extern
#endif
#endif

//! Deltaticks type
#if defined( _WIN32 ) || defined( _WIN64 )
typedef unsigned __int64 tick_t;
#else
#include <stdint.h>
typedef uint64_t tick_t;
#endif

//! Deltatime type (float or double)
//typedef float deltatime_t;
typedef double deltatime_t;


/*! Initialize timer library */
TIMER_API int            timer_lib_initialize( void );

/*! Shutdown timer library */
TIMER_API void           timer_lib_shutdown( void );

/*! Get current timestamp, in ticks of system-specific frequency (queryable with timer_ticks_per_second), measured from some system-specific base timestamp
    and not in sync with other timestamps
    \return              Current timestamp */
TIMER_API tick_t         timer_current( void );

/*! Get elapsed time since given timestamp
    \param t             Timestamp
    \return              Number of seconds elapsed */
TIMER_API deltatime_t    timer_elapsed( const tick_t t );

/*! Get elapsed ticks since given timestamp
    \param t             Timestamp
    \return              Number of ticks elapsed */
TIMER_API tick_t         timer_elapsed_ticks( const tick_t t );

/*! Get timer frequency, as number of ticks per second
    \return              Ticks per second */
TIMER_API tick_t         timer_ticks_per_second( void );

/*! Get ticks as seconds (effectively calculating ticks/timer_ticks_per_second())
	\param dt            Deltatime in ticks
    \return              Deltatime in seconds */
TIMER_API deltatime_t    timer_ticks_to_seconds( const tick_t dt );

/*! Get system time, in milliseconds since the epoch (UNIX time)
    \return  Current timestamp, in milliseconds */
TIMER_API tick_t         timer_system( void );

