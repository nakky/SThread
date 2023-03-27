
#include "SThread/Lock.h"

#include <stdio.h>

#include <time.h>

#if defined OS_MACOSX || defined OS_IPHONE
#include <sys/time.h>
#include <CoreFoundation/CoreFoundation.h>
#endif

#if defined OS_LINUX || defined OS_ANDROID
#include <sys/time.h>
#endif


namespace SThread{

    //////////////////////////////////////////////////////////////////////
    //							Condition								//
    //////////////////////////////////////////////////////////////////////

    ResumeStatus Condition::timedwait(unsigned long timeoutMilliSec, Mutex *mutex)
    {
#if defined USE_WINDOWSTHREAD_INTERFACE
        DWORD res;
        ResumeStatus ret;
        unsigned int wake = 0;
        unsigned long generation;
        
        ::EnterCriticalSection(&mCriticalSection);
        mNumWaiting++;
        generation = mGeneration;
        ::LeaveCriticalSection(&mCriticalSection);
        
        
        int isLocking = FALSE;
        
        ::EnterCriticalSection(&mCriticalSection);
        if(mutex != NULL){
            if(mutex->isLocking())isLocking = TRUE;
            mutex->unlock();
        }else{
            if(this->isLocking())isLocking = TRUE;
            unlock();
        }
        ::LeaveCriticalSection(&mCriticalSection);
        
        do {
            
            res = ::WaitForSingleObject(mSemaphore, timeoutMilliSec);
            
            ::EnterCriticalSection(&mCriticalSection);
            
            if (mNumWake > 0) {
                if (mGeneration != generation) {
                    mNumWake--;
                    mNumWaiting--;
                    ret = RESUME_SIGNALED;
                    break;
                } else {
                    wake = 1;
                }
            }
            else if (res == WAIT_TIMEOUT) {
                mNumWaiting--;
                ret = RESUME_TIMEDOUT;
                break;
            }
            
            ::LeaveCriticalSection(&mCriticalSection);
            
            if (wake) {
                wake = 0;
                ::ReleaseSemaphore(mSemaphore, 1, NULL);
            }
        } while (true);
        
        ::LeaveCriticalSection(&mCriticalSection);
        
        ::EnterCriticalSection(&mCriticalSection);
        if(isLocking){
            if(mutex != NULL)mutex->lock();
            else lock();
        }
        ::LeaveCriticalSection(&mCriticalSection);
        
        return ret;
        
#elif defined USE_PTHREAD_INTERFACE
        
        struct timespec timeout;
        struct timeval currentTime;
        
        gettimeofday(&currentTime, 0);

        timeout.tv_sec = timeoutMilliSec / 1000;
        timeout.tv_sec += currentTime.tv_sec;
        timeout.tv_nsec = (timeoutMilliSec % 1000) * 1000000;
        timeout.tv_nsec += currentTime.tv_usec * 1000;

        if(timeout.tv_nsec >= 1e9){
            timeout.tv_nsec -= 1e9;
            timeout.tv_sec++;
        }

        int err = 0;
        
        if(mutex == NULL){
            err = pthread_cond_timedwait(&mCondition, &mMutex, &timeout);
        }else{
            pthread_mutex_t pMutex = mutex->getMutexHandle();
            err = pthread_cond_timedwait(&mCondition, &pMutex, &timeout);
        }
        
        //if(!isLocking())unlock();

        if(err != 0)return RESUME_TIMEDOUT;
        else return RESUME_SIGNALED;
        
#endif

    }
}; //namespace SThread

