/******************************************************************/
/*!
	@file	W32ThreadDriver.h
	@brief
	@note
	@todo
	@bug
 
	@author Naoto Nakamura
	@date Sep. 15, 2010
 */
/******************************************************************/

#ifndef STHREAD_W32_W32THREADDRIVER_H
#define STHREAD_W32_W32THREADDRIVER_H

#include "SThread/Common.h"

#ifdef USE_WINDOWSTHREAD_INTERFACE

#include "SThread/ThreadDriver.h"

namespace SThread{
    //////////////////////////////////////////////////
    //				forward declarations			//
    //////////////////////////////////////////////////
    //implemented
    class W32ThreadDriver;
    
    //////////////////////////////////////////////////
    //				class declarations				//
    //////////////////////////////////////////////////
    class W32ThreadDriver  : public ThreadDriver
    {
    public:
        W32ThreadDriver(Thread *thread)
            :ThreadDriver(thread)
        {
        }
        
    public:
        
        virtual bool setPriority(int priority);

        virtual void startThread(const int bindIndex);
        virtual void cancelThread();


        virtual void shutdownThread();
        
        virtual ResumeStatus join(unsigned long timeupMillSec);
      
    private:
        static unsigned int __stdcall _staticRun(void *instance);

    private:

        ThreadHandle mJoinHandle;
        Condition mJoinCondition;
    };
    
}; //namespace SThread

#endif //USE_WINDOWSTHREAD_INTERFACE

#endif //STHREAD_W32_W32THREADDRIVER_H
