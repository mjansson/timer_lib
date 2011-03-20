#include "timer.h"

#include <stdio.h>
#include <string.h>


int main( int argc, char** argv )
{
	timer* time;
	tick_t tick, freq, last, res;
	
	printf( "Timer test\n" );

	timer_lib_initialize();
	
	time = malloc( sizeof( timer ) );
	timer_initialize( time );

	last = 0;
	res  = 0xFFFFFFFFFFFFFFFFULL;
	freq = timer_ticks_per_second( time );
	
	while( 1 )
	{
		tick = timer_elapsed_ticks( time, 0 );

		if( tick > last )
		{
			if( tick - last < res )
				res = tick - last;
		}

		last = tick;

		if( (double)tick / (double)freq > 10.0 )
			break;
	}

	printf( "Resolution: %lfms\n", 1000.0 * (double)res / (double)freq );

	timer_lib_shutdown();
	
	return 0;
}

