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

#include "timer.h"

#if defined( _WIN32 ) || defined( _WIN64 )
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
static tick_t _timerlib_freq  = 0;
#elif __APPLE__
#  include <mach/mach_time.h>
#  include <string.h>
static mach_timebase_info_data_t _timerlib_info;
static void absolutetime_to_nanoseconds (uint64_t mach_time, uint64_t* clock ) { *clock = mach_time * _timerlib_info.numer / _timerlib_info.denom; }
#else
#  include <unistd.h>
#  include <time.h>
#  include <string.h>
#endif

static double _timerlib_oofreq  = 0;


int timer_lib_initialize( void )
{
#if defined( _WIN32 ) || defined( _WIN64 )
	tick_t unused;
	if( !QueryPerformanceFrequency( (LARGE_INTEGER*)&_timerlib_freq ) ||
	    !QueryPerformanceCounter( (LARGE_INTEGER*)&unused ) )
		return -1;
#elif __APPLE__
	if( mach_timebase_info( &_timerlib_info ) )
		return -1;
#else
	struct timespec ts = { .tv_sec = 0, .tv_nsec = 0 };
	if( clock_gettime( CLOCK_MONOTONIC, &ts ) )
		return -1;
#endif

	_timerlib_oofreq = 1.0 / (double)timer_ticks_per_second();

	return 0;
}


void timer_lib_shutdown( void )
{
}


tick_t timer_current( void )
{
#if defined( _WIN32 ) || defined( _WIN64 )

	tick_t curclock;
	QueryPerformanceCounter( (LARGE_INTEGER*)&curclock );
	return curclock;

#elif __APPLE__

	tick_t curclock = 0;
	absolutetime_to_nanoseconds( mach_absolute_time(), &curclock );
	return curclock;

#else

	struct timespec ts = { .tv_sec = 0, .tv_nsec = 0 };
	clock_gettime( CLOCK_MONOTONIC, &ts );
	return ( (uint64_t)ts.tv_sec * 1000000000ULL ) + ts.tv_nsec;

#endif
}


tick_t timer_ticks_per_second( void )
{
#if defined( _WIN32 ) || defined( _WIN64 )
	return _timerlib_freq;
#elif __APPLE__
	return 1000000000ULL;
#else
	return 1000000000ULL;
#endif
}


deltatime_t timer_elapsed( const tick_t t )
{
	return (deltatime_t)( (double)timer_elapsed_ticks( t ) * _timerlib_oofreq );
}


tick_t timer_elapsed_ticks( const tick_t t )
{
	tick_t dt = 0;

#if defined( _WIN32 ) || defined( _WIN64 )

	tick_t curclock = t;
	QueryPerformanceCounter( (LARGE_INTEGER*)&curclock );
	dt = curclock - t;

#elif __APPLE__

	tick_t curclock = t;
	absolutetime_to_nanoseconds( mach_absolute_time(), &curclock );
	dt = curclock - t;

#else

	tick_t curclock;
	struct timespec ts = { .tv_sec = 0, .tv_nsec = 0 };
	clock_gettime( CLOCK_MONOTONIC, &ts );

	curclock = ( (tick_t)ts.tv_sec * 1000000000ULL ) + ts.tv_nsec;
	dt = curclock - t;

#endif

	return dt;
}


deltatime_t timer_ticks_to_seconds( const tick_t dt )
{
	return (deltatime_t)( (double)dt * _timerlib_oofreq );
}


#if defined( _WIN32 ) || defined( _WIN64 )
#  if _MSC_VER
#    include <sys/timeb.h>
#  else
struct __timeb64 {
	__time64_t time;
	unsigned short millitm;
	short timezone;
	short dstflag;
	};
_CRTIMP errno_t __cdecl _ftime64_s(_Out_ struct __timeb64 * _Time);
#  endif
#endif

tick_t timer_system( void )
{
#if defined( _WIN32 ) || defined( _WIN64 )

	struct __timeb64 tb;
	_ftime64_s( &tb );
	return ( (tick_t)tb.time * 1000ULL ) + (tick_t)tb.millitm;

#elif __APPLE__

	tick_t curclock = 0;
	absolutetime_to_nanoseconds( mach_absolute_time(), &curclock );
	return ( curclock / 1000000ULL );

#else

	struct timespec ts = { .tv_sec = 0, .tv_nsec = 0 };
	clock_gettime( CLOCK_REALTIME, &ts );
	return ( (uint64_t)ts.tv_sec * 1000ULL ) + ( ts.tv_nsec / 1000000ULL );

#endif
}
