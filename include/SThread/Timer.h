/******************************************************************/
/*!
	@file	Timer.h
	@brief	Classes for managing time schedule
	@note
	@todo
	@bug

	@author	Naoto Nakamura
	@date	Mar. 30, 2009
*/
/******************************************************************/

#ifndef STHREAD_TIMER_H
#define STHREAD_TIMER_H

#include "SThread/Common.h"

#if defined OS_WINDOWS
#include <mmsystem.h>
#elif defined OS_MACOSX || defined OS_IPHONE
#include <CoreFoundation/CoreFoundation.h>
#elif defined OS_LINUX || defined OS_ANDROID
#include <sys/time.h>
#endif


namespace SThread{
    //////////////////////////////////////////////////////
    //				forward declarations				//
    //////////////////////////////////////////////////////
    //implemented
    class Timer;

    //////////////////////////////////////////////////////
    //				class declarations					//
    //////////////////////////////////////////////////////
    /****************************************/
    /*!
        @class	Timer
        @brief	Timer class
        @note

        @author	Naoto Nakamura
        @date	Mar. 23, 2009
    */
    /****************************************/
    class Timer
    {
    public:
        Timer()
#if defined OS_WINDOWS
        :mBaseTime(0)
#elif defined OS_MACOSX || defined OS_IPHONE
        :mBaseTime(0.0)
#endif
        {
#if defined OS_WINDOWS
            timeBeginPeriod(1);
#endif
            reset();
        }

        ~Timer(){
#if defined OS_WINDOWS
            timeEndPeriod(1);
#endif
        }
    public:
        inline void reset();

        inline unsigned int getElapsedTime();
        static void sleep(unsigned int milliSec);

    private:
#if defined OS_WINDOWS
        unsigned int mBaseTime;
#elif defined OS_MACOSX || defined OS_IPHONE
        double mBaseTime;
#elif defined OS_LINUX || defined OS_ANDROID
        struct timeval mBaseTime;
        struct timeval mElapsedTime;
#endif

    };

    /****************************************/
    /*!
        @brief	Reset timer
        @note	Reset internal time stump (base time)

        @author	Naoto Nakamura
        @date	Aug. 11, 2009
    */
    /****************************************/
    inline void Timer::reset()
    {
#if defined OS_WINDOWS
        mBaseTime = timeGetTime();
#elif defined OS_MACOSX || defined OS_IPHONE
        mBaseTime = CFAbsoluteTimeGetCurrent();
#elif defined OS_LINUX || defined OS_ANDROID
        gettimeofday(&mBaseTime, NULL);
#endif
    }

    /****************************************/
    /*!
        @brief	Get Current time
        @note

        @return	Got time (millisec)

        @author	Naoto Nakamura
        @date	Aug. 11, 2009
    */
    /****************************************/
    inline unsigned int Timer::getElapsedTime()
    {
#if defined OS_WINDOWS
        return timeGetTime() - mBaseTime;
#elif defined OS_MACOSX || defined OS_IPHONE
        return (unsigned int)((CFAbsoluteTimeGetCurrent() - mBaseTime) * 1000.0);
#elif defined OS_LINUX || defined OS_ANDROID
        gettimeofday(&mElapsedTime, NULL);
        return (unsigned int)(mElapsedTime.tv_sec - mBaseTime.tv_sec) % 0xffffff * 1000 + (mElapsedTime.tv_usec - mBaseTime.tv_usec) / 1000;
#endif
    }


};// namespace SThread


#endif //STHREAD_TIMER_H
