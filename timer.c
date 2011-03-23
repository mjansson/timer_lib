/* timer.c  -  v0.4  -  Public Domain  -  2011 Mattias Jansson / Rampant Pixels
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
 */

#include "timer.h"

#ifndef USE_FALLBACK
#define USE_FALLBACK 0
#endif

#if defined( _WIN32 ) || defined( _WIN64 )
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#  if USE_FALLBACK
#  include <mmsystem.h>
#  endif
static tick_t _timerlib_curtime_freq  = 0;
#elif __APPLE__
#  include <mach/mach_time.h>
#else
#  include <unistd.h>
#  include <time.h>
#  include <string.h>
#endif


int timer_lib_initialize()
{
#if defined( _WIN32 ) || defined( _WIN64 )
#if USE_FALLBACK
	timeBeginPeriod( 1U );
#endif
	tick_t unused;
	if( !QueryPerformanceFrequency( (LARGE_INTEGER*)&_timerlib_curtime_freq ) ||
	    !QueryPerformanceCounter( (LARGE_INTEGER*)&unused ) )
		return -1;
#elif __APPLE__
	mach_timebase_info_data_t info;
	if( mach_timebase_info( &info ) )
		return -1;
#else
	struct timespec ts = { .tv_sec = 0, .tv_nsec = 0 };
	if( clock_gettime( CLOCK_MONOTONIC, &ts ) )
		return -1;
#endif
	return 0;
}


void timer_lib_shutdown()
{
#if defined( _WIN32 ) || defined( _WIN64 )
#if USE_FALLBACK
	timeEndPeriod( 1 );
#endif
#endif
}


void timer_initialize( timer* time )
{
	memset( time, 0, sizeof( time ) );

#if defined( _WIN32 ) || defined( _WIN64 )
	QueryPerformanceFrequency( (LARGE_INTEGER*)&time->freq );
#elif __APPLE__
	time->freq   = 1000000000;
#else
	time->freq   = 1000000000;
#endif
	time->oofreq = (deltatime_t)( 1.0 / (double)time->freq );

	timer_reset( time );
}


void timer_reset( timer* time )
{
#if defined( _WIN32 ) || defined( _WIN64 )

	QueryPerformanceCounter( (LARGE_INTEGER*)&time->clock );
#if USE_FALLBACK
	time->ref = timeGetTime();
#endif

#elif __APPLE__

	absolutetime_to_nanoseconds( mach_absolute_time(), &time->clock );

#else

	struct timespec ts = { .tv_sec = 0, .tv_nsec = 0 };
	clock_gettime( CLOCK_MONOTONIC, &ts );
	time->clock = ( (tick_t)ts.tv_sec * 1000000000ULL ) + ts.tv_nsec;

#endif
}


deltatime_t timer_elapsed( timer* time, int reset )
{
	return (deltatime_t)timer_elapsed_ticks( time, reset ) * time->oofreq;
}


tick_t timer_elapsed_ticks( timer* time, int reset )
{
	tick_t dt = 0;

#if defined( _WIN32 ) || defined( _WIN64 )

	tick_t diff;
#if USE_FALLBACK
	tick_t refdiff;
	deltatime_t timerdiff;
	tick_t ref      = time->ref;
#endif
	tick_t curclock = time->clock;

	QueryPerformanceCounter( (LARGE_INTEGER*)&curclock );
#if USE_FALLBACK
	ref = timeGetTime();
#endif

	diff = curclock - time->clock;
#if USE_FALLBACK
	refdiff = ref - time->ref;

	if( ref < time->ref )
		refdiff = (tick_t)( 1000.0 * diff * time->oofreq ); //Catch looping of the millisecond counter

	timerdiff = (deltatime_t)( ( diff * time->oofreq ) - ( refdiff * 0.001 ) );
	if( ( diff < 0 ) || ( timerdiff > 0.1 ) || ( timerdiff < -0.1 ) )
		diff = (tick_t)( ( refdiff * 0.001 ) * time->freq ); //Performance counter messed up, transform reference to counter frequency
#endif

	dt = diff;

#if USE_FALLBACK
	if( reset )
		time->ref = ref;
#endif

#elif __APPLE__

	tick_t curclock = time->clock;
	absolutetime_to_nanoseconds( mach_absolute_time(), &curclock );

	dt = curclock - time->clock;

#else

	tick_t curclock;
	struct timespec ts = { .tv_sec = 0, .tv_nsec = 0 };
	clock_gettime( CLOCK_MONOTONIC, &ts );

	curclock = ( (tick_t)ts.tv_sec * 1000000000ULL ) + ts.tv_nsec;

	dt = curclock - time->clock;

#endif

	if( reset )
		time->clock = curclock;

	return dt;
}


tick_t timer_ticks_per_second( timer* time )
{
	return time->freq;
}


tick_t timer_current()
{
#if defined( _WIN32 ) || defined( _WIN64 )

	//TODO: Fallback to timeGetTime for messed up perf counter values
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


tick_t timer_current_ticks_per_second()
{
#if defined( _WIN32 ) || defined( _WIN64 )
	return _timerlib_curtime_freq;
#elif __APPLE__
	return 1000000000;
#else
	return 1000000000;
#endif
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

tick_t timer_system()
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
