

#include <stdio.h>
#include <signal.h>

#include "SThread/Timer.h"
#include "SThread/Thread.h"

namespace SThread{

    /****************************************/
    /*!
        @brief	Constructor
        @note

        @author	Naoto Nakamura
        @date	Sep. 15, 2008
    */
    /****************************************/
    Thread::Thread(Condition *sharedCondition, const int priority, int bindIndex):
    mState(THREAD_STOPED),
    mThreadCondition(sharedCondition),
    mCondiionShared(false),
    mPriority(priority),
    mBindIndex(bindIndex)
    {
        if(sharedCondition != NULL) mCondiionShared = true;
    }

    /****************************************/
    /*!
        @brief	Destructor
        @note

        @author	Naoto Nakamura
        @date	Sep. 15, 2008
    */
    /****************************************/
    Thread::~Thread()
    {
    }

    /****************************************/
    /*!
        @brief	Initialize
        @note

        @author	Naoto Nakamura
        @date	Sep. 15, 2008
    */
    /****************************************/
    void Thread::init()
    {
        if(mThreadCondition == NULL){
            mThreadCondition = new Condition();
            mCondiionShared = false;
        }

        mControlLocker = new Mutex();

        mDriver = ThreadDriver::createDriver(this);
        mDriver->init();
    }

    /****************************************/
    /*!
        @brief	Cleanup
        @note

        @author	Naoto Nakamura
        @date	Sep. 15, 2008
    */
    /****************************************/
    void Thread::cleanup()
    {
        shutdown();

        mDriver->cleanup();
        SAFE_DELETE(mDriver);
        
        SAFE_DELETE(mControlLocker);

        if(!mCondiionShared) SAFE_DELETE(mThreadCondition);
    }

    /****************************************/
    /*!
        @brief Set priority to the thread
        @note

        @param priority set priority

        @author	Naoto Nakamura
        @date	Sep. 15, 2008
    */
    /****************************************/
    bool Thread::setPriority(int priority)
    {
        if( mDriver->setPriority(priority) ){
            mPriority = priority;
            return true;
        }
        return false;
    }

    /****************************************/
    /*!
        @brief Start the thread
        @note

        @author	Naoto Nakamura
        @date	Sep. 15, 2008
    */
    /****************************************/
    bool Thread::start()
    {
        if(mState.load() != THREAD_STOPED){
            return FALSE;
        }
        setState(THREAD_RUNNING);

        mControlLocker->lock();

        mDriver->startThread(mBindIndex);

        mControlLocker->unlock();

        return setPriority(mPriority);

    }

    /****************************************/
    /*!
        @brief	Joint the thread
        @note	A thread calling this function
                wait untill "this" thread has ended.

        @author	Naoto Nakamura
        @date	Sep. 15, 2008
    */
    /****************************************/
    ResumeStatus Thread::join(unsigned long timeupMillSec)
    {
        if(mState.load() == THREAD_STOPED){
            return RESUME_JOINED;
        }
        return mDriver->join(timeupMillSec);
    }

    void Thread::runContainer()
    {
        run();
        
        setState(THREAD_STOPED);
    }

    /****************************************/
    /*!
        @brief	Shut down the thread
        @note

        @author	Naoto Nakamura
        @date	Sep. 15, 2008
    */
    /****************************************/
    bool Thread::shutdown()
    {
        ThreadHandle handle = getHandle();
        if (handle == NULL) return false;

        if(mState.load() != THREAD_STOPED){
            setState(THREAD_QUITTING);
        }
        
        join();

        Timer::sleep(10);

        if(mState.load() != THREAD_STOPED){
            setState(THREAD_STOPED);
        }
                    
        if(mDriver->mThread != NULL){
            mDriver->shutdownThread();
            mDriver->mThread = NULL;
        }

        return true;
    }

}; //namespace SThread

