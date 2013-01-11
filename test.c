#include "timer.h"

#include <stdio.h>


int main( int argc, char** argv )
{
	tick_t start, time;
	tick_t tick, freq, last, res;
	
	printf( "Timer test\n" );

	timer_lib_initialize();

	last  = 0;
	res   = 0xFFFFFFFFFFFFFFFFULL;
	freq  = timer_ticks_per_second();
	start = timer_current();
	
	while( 1 )
	{
		time = timer_current();
		do {
			tick = timer_elapsed_ticks( time );
		} while( !tick );

		if( tick < res )
			res = tick - last;

		last = tick;

		if( timer_elapsed( start ) > 10.0 )
			break;
	}

	printf( "Resolution: %lfms\n", 1000.0 * (double)timer_ticks_to_seconds( res ) );

	timer_lib_shutdown();
	
	return 0;
}

