/* timer.c  -  v0.1  -  Public Domain  -  2011 Mattias Jansson / Rampant Pixels
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
 */

#include "timer.h"

#if defined( _WIN32 ) || defined( _WIN64 )
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#  include <mmsystem.h>
static tick_t _timerlib_curtime_freq  = 0;
#else
#  include <unistd.h>
#  include <time.h>
#  include <string.h>
#endif


void timer_lib_initialize()
{
#if defined( _WIN32 ) || defined( _WIN64 )
	timeBeginPeriod( 1U );
	QueryPerformanceFrequency( (LARGE_INTEGER*)&_timerlib_curtime_freq );
#endif
}


void timer_lib_shutdown()
{
#if defined( _WIN32 ) || defined( _WIN64 )
	timeEndPeriod( 1 );
#endif
}


void timer_initialize( timer* time )
{
	memset( time, 0, sizeof( time ) );

#if defined( _WIN32 ) || defined( _WIN64 )
	QueryPerformanceFrequency( (LARGE_INTEGER*)&time->freq );
#else
	time->freq   = 1000000;
#endif
	time->oofreq = (deltatime_t)( 1.0 / (double)time->freq );

	timer_reset( time );
}


void timer_reset( timer* time )
{
#if defined( _WIN32 ) || defined( _WIN64 )

	QueryPerformanceCounter( (LARGE_INTEGER*)&time->clock );
	time->ref = timeGetTime();

#else

	struct timespec ts = { .tv_sec = 0, .tv_nsec = 0 };
	int err = clock_gettime( CLOCK_REALTIME, &ts );
	if( err < 0 )
		return;

	time->clock = ( (tick_t)ts.tv_sec * 1000000ULL ) + ( ts.tv_nsec / 1000ULL );

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

	tick_t diff, refdiff;
	deltatime_t timerdiff;
	tick_t curclock = time->clock;
	tick_t ref      = time->ref;

	QueryPerformanceCounter( (LARGE_INTEGER*)&curclock );
	ref = timeGetTime();

	diff    = curclock - time->clock;
	refdiff = ref - time->ref;

	if( refdiff < 0 )
		refdiff = (tick_t)( 1000.0 * diff * time->oofreq ); //Catch looping of the millisecond counter

	timerdiff = (deltatime_t)( ( diff * time->oofreq ) - ( refdiff * 0.001 ) );
	if( ( diff < 0 ) || ( timerdiff > 0.1 ) || ( timerdiff < -0.1 ) )
		diff = (tick_t)( ( refdiff * 0.001 ) * time->freq ); //Performance counter messed up, transform reference to counter frequency

	dt = diff;

	if( reset )
		time->ref = ref;

#else

	tick_t curclock;
	struct timespec ts = { .tv_sec = 0, .tv_nsec = 0 };
	int err = clock_gettime( CLOCK_REALTIME, &ts );
	if( err < 0 )
		return;

	curclock = ( (tick_t)ts.tv_sec * 1000000ULL ) + ( ts.tv_nsec / 1000ULL );

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

#else

	struct timespec ts = { .tv_sec = 0, .tv_nsec = 0 };
	int err = clock_gettime( CLOCK_REALTIME, &ts );
	if( err < 0 )
		return 0;

	return ( (uint64_t)ts.tv_sec * 1000000ULL ) + ( ts.tv_nsec / 1000ULL );

#endif
}


tick_t timer_current_ticks_per_second()
{
#if defined( _WIN32 ) || defined( _WIN64 )
	return _timerlib_curtime_freq;
#else
	return 1000000;
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

#else
	return timer_current() / 1000ULL;
#endif
}
