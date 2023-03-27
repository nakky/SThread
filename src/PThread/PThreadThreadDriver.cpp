
#include "SThread/PThread/PThreadThreadDriver.h"

#include <unistd.h>

#ifdef USE_PTHREAD_INTERFACE

#include "SThread/Timer.h"
#include "SThread/Thread.h"

#if defined OS_MACOSX
#include <mach/mach.h>
#include <mach/thread_policy.h>
#elif defined OS_ANDROID
#include <sched.h>
#endif


#if defined OS_LINUX
#include <sys/types.h>
#include <sys/syscall.h>
#include <pthread.h>

pid_t gettid(void) {
    return syscall(SYS_gettid);
}
#endif

namespace SThread{

    bool PThreadThreadDriver::setPriority(int priority)
    {
        struct sched_param param;
        param.sched_priority = 0;

        int policy = SCHED_OTHER;

        if(::pthread_setschedparam(mThreadHandle, policy, &param) != 0){
            return false;
        }

        int min = sched_get_priority_min(policy);
        int max = sched_get_priority_max(policy);
        int mid = (min + max) / 2;

        int pri = 0;

        switch(priority){
            case PRIORITY_LOWEST:
            pri = min;
            break;
            case PRIORITY_BELOW_NORMAL:
            pri = mid - (mid - min) / 2;
            break;
            case PRIORITY_NORMAL:
            pri = mid;
            break;
            case PRIORITY_ABOVE_NORMAL:
            pri = mid + (max - mid) / 2;
            break;
            case PRIORITY_HIGHEST:
            pri = max;
            break;
        }

        if(::pthread_getschedparam(mThreadHandle, &policy, &param) != 0)return false;
        param.sched_priority =  pri;

        if(::pthread_setschedparam(mThreadHandle, policy, &param) != 0){
            return false;
        }
        else return true;
    }

    void PThreadThreadDriver::startThread(const int bindIndex)
    {
        mBindIndex = bindIndex;

#if defined OS_MACOSX
        pthread_create_suspended_np(&mThreadHandle, NULL, _staticRun, this);
        mach_port_t mach_thread = pthread_mach_thread_np(mThreadHandle);
        kern_return_t rc = 0;
        if(bindIndex >= 0){
            thread_affinity_policy_data_t policy = { bindIndex + 1 };
            rc = thread_policy_set(mach_thread, THREAD_AFFINITY_POLICY, (thread_policy_t)&policy, THREAD_AFFINITY_POLICY_COUNT);
        }

        thread_resume(mach_thread);
//#elif defined OS_LINUX
       // pthread_create_suspended_np(&mThreadHandle, NULL, _staticRun, this);

#else
        ::pthread_create(&mThreadHandle, NULL, _staticRun, this);
        mJoinHandle = mThreadHandle;
#endif

    }

    void PThreadThreadDriver::cancelThread()
    {
#if !defined OS_ANDROID
        ::pthread_cancel(mThreadHandle);
#endif //!defined OS_ANDROID

        mJoinCondition.signalAll();
    }

    void PThreadThreadDriver::shutdownThread()
    {
        ::pthread_detach(mThreadHandle);
    }

    ResumeStatus PThreadThreadDriver::join(unsigned long timeupMillSec)
    {
        //void *theradReturn = NULL;

        //int retval = ::pthread_join(mThreadHandle, &theradReturn);
        mJoinCondition.lock();

        if (mJoinHandle == NULL){
            mJoinCondition.unlock();
            return RESUME_JOINED;
        }

        ResumeStatus ret = mJoinCondition.timedwait(timeupMillSec);
        mJoinCondition.unlock();

        return ret;
    }

    /****************************************/
    /*!
     @brief	Callback run function
     @note	static

     @param	this pointor describing the instance
     @return	Do not care

     @author	Naoto Nakamura
     @date	Sep. 15, 2008
     */
    /****************************************/
    void *PThreadThreadDriver::_staticRun(void *instance)
    {
        PThreadThreadDriver *driver = (PThreadThreadDriver*)instance;
        Thread *pThread = driver->getThread();

#if defined OS_ANDROID || defined OS_LINUX

        if(driver->mBindIndex >= 0){
            cpu_set_t cpu_set;
            int result;

            CPU_ZERO(&cpu_set);
            CPU_SET(driver->mBindIndex, &cpu_set);

            pid_t pid = gettid();

            if (sched_setaffinity(pid, sizeof(cpu_set_t), &cpu_set) != 0) {
                //SBLOG("sched_setaffinity failed! ID:%d", driver->mBindIndex);
                exit(0);
            }
        }

#endif

        pThread->runContainer();

        driver->mJoinCondition.lock();
        Timer::sleep(1);
        driver->mJoinHandle = NULL;
        driver->mJoinCondition.signalAll();
        driver->mJoinCondition.unlock();
        return 0;
    }
    
}; //namespace SThread

#endif //USE_PTHREAD_INTERFACE


