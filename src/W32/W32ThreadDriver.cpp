
#include "SThread/W32/W32ThreadDriver.h"

#ifdef USE_WINDOWSTHREAD_INTERFACE

#include "SThread/Timer.h"
#include "SThread/Thread.h"

namespace SThread{

    bool W32ThreadDriver::setPriority(int priority)
    {
        int pri = 0;
        switch (priority)
        {
        case PRIORITY_LOWEST:
            pri = THREAD_PRIORITY_IDLE;
            break;
        case PRIORITY_BELOW_NORMAL:
            pri = THREAD_PRIORITY_BELOW_NORMAL;
            break;
        case PRIORITY_NORMAL:
            pri = THREAD_PRIORITY_NORMAL;
            break;
        case PRIORITY_ABOVE_NORMAL:
            pri = THREAD_PRIORITY_ABOVE_NORMAL;
            break;
        case PRIORITY_HIGHEST:
            pri = THREAD_PRIORITY_TIME_CRITICAL;
            break;
        }
        if(mThreadHandle != NULL) return ::SetThreadPriority(mThreadHandle, priority) == 0;
        else return false;
    }
    
    void W32ThreadDriver::startThread(const int bindIndex)
    {

        mThreadHandle = (HANDLE)::_beginthreadex(NULL, 0, _staticRun, this, CREATE_SUSPENDED, NULL);
        
        if (bindIndex >= 0) {
            DWORD cpu = 1 << bindIndex;
            SetThreadAffinityMask(mThreadHandle, cpu);
        }

        mJoinHandle = mThreadHandle;

        ResumeThread(mThreadHandle);
    }

    void W32ThreadDriver::cancelThread()
    {
        ::TerminateThread(mThreadHandle, false);
    }


    void W32ThreadDriver::shutdownThread()
    {
       if(mThreadHandle != NULL)::CloseHandle(mThreadHandle);
       mThreadHandle = NULL;
    }
    
    ResumeStatus W32ThreadDriver::join(unsigned long timeupMillSec)
    {
        //if (mThread->getState() == Thread::THREAD_STOPED) return RESUME_JOINED;

        mJoinCondition.lock();

        if (mJoinHandle == NULL){
            mJoinCondition.unlock();
            return RESUME_JOINED;
        }

        ResumeStatus ret = mJoinCondition.timedwait(timeupMillSec);
        mJoinCondition.unlock();

        return ret;
    }

    unsigned int __stdcall W32ThreadDriver::_staticRun(void *instance)
    {
        W32ThreadDriver *driver = (W32ThreadDriver*)instance;
        Thread *thread = driver->getThread();
        
        thread->runContainer();
        
        driver->mJoinCondition.lock();
        Timer::sleep(1);
        driver->mJoinHandle = NULL;
        driver->mJoinCondition.signalAll();
        driver->mJoinCondition.unlock();

        return 0;
    }
    
}; //namespace SThread

#endif //USE_WINDOWSTHREAD_INTERFACE


