

#include "SThread/Timer.h"

#include <time.h>

namespace SThread{
    /****************************************/
    /*!
        @brief	Sleep
        @note

        @param	milliSec sleep time (millisec)

        @author	Naoto Nakamura
        @date	Aug. 11, 2009
    */
    /****************************************/
    void Timer::sleep(unsigned int milliSec)
    {
#if defined COMPILER_MSVC
        timeBeginPeriod(1);
        Sleep(milliSec);
        timeEndPeriod(1);
#elif defined COMPILER_GCC
        struct timespec time;
        time.tv_sec = milliSec / 1000;
        time.tv_nsec = (milliSec % 1000) * 1000000L;
        nanosleep(&time, NULL);
#endif

    }

};	// namespace SThread
