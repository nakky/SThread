/******************************************************************/
/*!
	@file	ThreadDriver.h
	@brief
	@note
	@todo
	@bug
 
	@author Naoto Nakamura
	@date Sep. 15, 2010
 */
/******************************************************************/

#ifndef STHREAD_THREADDRIVER_H
#define STHREAD_THREADDRIVER_H

#include "SThread/Common.h"

#include "SThread/Lock.h"

namespace SThread{
    
    //////////////////////////////////////////////////
    //				forward declarations			//
    //////////////////////////////////////////////////
    //implemented
    class ThreadDriver;
    
    //required
    class Thread;
    
#if defined USE_WINDOWSTHREAD_INTERFACE
    typedef HANDLE ThreadHandle;
#elif defined USE_PTHREAD_INTERFACE
    typedef pthread_t ThreadHandle;
#endif

    enum ThreadPriority{
        PRIORITY_LOWEST,
        PRIORITY_BELOW_NORMAL,
        PRIORITY_NORMAL,
        PRIORITY_ABOVE_NORMAL,
        PRIORITY_HIGHEST
    };
    
    //////////////////////////////////////////////////
    //				class declarations				//
    //////////////////////////////////////////////////
    class ThreadDriver
    {
        friend class Thread;
        
    public:
        ThreadDriver(Thread *thread)
        :mThreadHandle(0),
        mThread(thread)
        {
        }
        
        virtual ~ThreadDriver(){}
        
    public:
        
        virtual void init(){}
        
        
        virtual void cleanup(){ mThreadHandle = 0; }
        virtual bool setPriority(int priority) = 0;
        
        virtual void startThread(const int bindIndex) = 0;
        virtual void cancelThread() = 0;
        virtual void shutdownThread() = 0;
                    
        virtual ResumeStatus join(unsigned long timeupMillSec) = 0;
        
        Thread *getThread(){return mThread;}
      
        static ThreadDriver *createDriver(Thread *thread);
    
        static ThreadHandle getCurrentThreadHandle();
    protected:
        
        ThreadHandle mThreadHandle;					//<! Thread hundle
        
        Thread *mThread;
    };
    
}; //namespace SThread

#endif //STHREAD_THREADDRIVER_H
