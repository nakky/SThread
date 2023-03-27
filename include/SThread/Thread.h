
/******************************************************************/
/*!
	@file	BaseThread.h
	@brief	Thread class definition
	@note
	@todo
	@bug

	@author Naoto Nakamura
	@date Sep. 15, 2008
*/
/******************************************************************/

#ifndef STHREAD_BASETHREAD_H
#define STHREAD_BASETHREAD_H

#include "SThread/Common.h"

#include <atomic>
#include <string>

#include "SThread/Lock.h"
#include "SThread/ThreadDriver.h"


namespace SThread{

    //////////////////////////////////////////////////
    //				forward declarations			//
    //////////////////////////////////////////////////
    //implemented
    class Thread;
    
    //required
    class Locker;

    //////////////////////////////////////////////////
    //				class declarations				//
    //////////////////////////////////////////////////
    /****************************************/
    /*!
        @class Thread
        @brief Thread class

        @author Naoto Nakamura
        @date Sep. 15, 2008
    */
    /****************************************/
    class Thread
    {
    public:
        static const int STATICRUN_TIME_OUT = 100; //minumum time for go through _staticRun
        
        static const int NUM_THREAD_STATE = 4096;
    public:
        enum ThreadState{
            THREAD_RUNNING,
            THREAD_WAITING,
            THREAD_STOPED,
            THREAD_QUITTING
        };

    public:
        explicit Thread(Condition *sharedCondition = NULL, const int priority = PRIORITY_NORMAL, const int bindIndex = -1);
        virtual ~Thread();

    private:
#if defined USE_WINDOWSTHREAD_INTERFACE
        static unsigned int __stdcall _staticRun(void *instance);
#endif
    protected:
        virtual void run() = 0;

    public:
        virtual void init();
        virtual void cleanup();

        ThreadState getState() const {
            return mState;
        }
        
        ThreadHandle getHandle() const {return mDriver->mThreadHandle;}

        bool setPriority(int priority);

        virtual bool start();
        virtual bool shutdown();
        
        ResumeStatus join(unsigned long timeupMillSec = 0xffffffff);
        
        void runContainer();

        static ThreadHandle getCurrentThreadHandle(){
            return ThreadDriver::getCurrentThreadHandle();
        }
        

        void setName(const std::string &name){mName = name;}

    protected:
        void setState(ThreadState state)
        {
            ThreadState oldState;
            do{
                oldState = mState.load();
            }while(!mState.compare_exchange_weak(oldState, state));
        }
        

    protected:
        std::atomic<ThreadState> mState;				//<! Thread state
        int mPriority;					//<! Thread priority(enum ThreadPriority)
        
        int mBindIndex;
        
        Condition *mThreadCondition;	//<! Condition for controlling thread
        bool mCondiionShared;
        
        ResourceLock *mControlLocker;	//<! Condition for controlling internal thread
        
        ThreadDriver* mDriver;

        std::string mName;
    };

}; //namespace SThread


#endif //STHREAD_BASETHREAD_H
