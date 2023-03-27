/******************************************************************/
/*!
	@file	PThreadThreadDriver.h
	@brief
	@note
	@todo
	@bug
 
	@author Naoto Nakamura
	@date Sep. 15, 2010
 */
/******************************************************************/

#ifndef STHREAD_PTHREAD_PTHREADTHREADDRIVER_H
#define STHREAD_PTHREAD_PTHREADTHREADDRIVER_H

#include "SThread/Common.h"

#ifdef USE_PTHREAD_INTERFACE

#include "SThread/ThreadDriver.h"


namespace SThread{
    //////////////////////////////////////////////////
    //				forward declarations			//
    //////////////////////////////////////////////////
    //implemented
    class PThreadThreadDriver;

    
    //////////////////////////////////////////////////
    //				class declarations				//
    //////////////////////////////////////////////////
    class PThreadThreadDriver : public ThreadDriver
    {
    public:
        PThreadThreadDriver(Thread *thread)
        :ThreadDriver(thread),
         mBindIndex(-1)
        {
        }
        
        virtual ~PThreadThreadDriver(){}
        
    public:
        virtual bool setPriority(int priority);
        
        virtual void startThread(const int bindIndex);
        virtual void cancelThread();

        virtual void shutdownThread();
        
        virtual ResumeStatus join(unsigned long timeupMillSec);
        
    private:
        static void *_staticRun(void *instance);
      
    private:
        
        ThreadHandle mJoinHandle;
        Condition mJoinCondition;

        int mBindIndex;
    };
    
}; //namespace SThread

#endif //USE_PTHREAD_INTERFACE

#endif //STHREAD_PTHREAD_PTHREADTHREADDRIVER_H
