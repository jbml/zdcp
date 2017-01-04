#ifndef COMMON_H_
#define COMMON_H_

#include <string>
#include <stdexcept> 
#include <sys/time.h>

using namespace std;

//exception throw macro
#define EXCEPTION_FILE_LINE_1(f,l) f "::" #l ":"
#define EXCEPTION_FILE_LINE(f,l) EXCEPTION_FILE_LINE_1(f,l) 

#define THROW_RUNTIME_ERROR(N) \
	do{ \
		throw std::runtime_error( string(EXCEPTION_FILE_LINE(__FILE__,__LINE__)) + (N) ); \
	}while(0);

//time watch macro
#if 1

#define TIME_WATCH_DECLARE(TAG) struct timeval _time_watch_tv_##TAG[2]; double _time_watch_##TAG = 0.0;

#define TIME_WATCH_BEGIN(TAG) gettimeofday(&_time_watch_tv_##TAG[0], NULL);
#define TIME_WATCH_END(TAG) do {\
	gettimeofday(&_time_watch_tv_##TAG[1], NULL); \
	_time_watch_##TAG += (_time_watch_tv_##TAG[1].tv_sec - _time_watch_tv_##TAG[0].tv_sec)  * 1000 \
	+ (_time_watch_tv_##TAG[1].tv_usec - _time_watch_tv_##TAG[0].tv_usec) / 1000;\
} while(0);
#define TIME_WATCH_PRINT(TAG) fprintf(stderr, "*Timer %s: %fms\n", #TAG, _time_watch_##TAG);

#define TIME_WATCH_START(TAG) TIME_WATCH_DECLARE(TAG) TIME_WATCH_BEGIN(TAG)
#define TIME_WATCH_STOP(TAG) TIME_WATCH_END(TAG) TIME_WATCH_PRINT(TAG)

#else

#define TIME_WATCH_DECLARE(TAG)
#define TIME_WATCH_BEGIN(TAG)
#define TIME_WATCH_END(TAG)
#define TIME_WATCH_PRINT(TAG)
#define TIME_WATCH_START(TAG)
#define TIME_WATCH_STOP(TAG)

#endif

#endif // COMMON_H_
//vim: ts=4 sw=4 cindent ai et
