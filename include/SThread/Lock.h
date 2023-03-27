/******************************************************************/
/*!
	@file	Lock.h
	@brief	Locker for multithread environment
	@note
	@todo
	@bug

	@author Naoto Nakamura
	@date Sep. 15, 2008
*/
/******************************************************************/


#ifndef STHREAD_LOCK_H
#define STHREAD_LOCK_H

#include "SThread/Common.h"

#if defined USE_WINDOWSTHREAD_INTERFACE
#include <process.h>
#include <winbase.h>
#elif defined USE_PTHREAD_INTERFACE
#include <pthread.h>
#ifdef OS_MACOSX
#include <libkern/OSAtomic.h>
#endif
#endif


namespace SThread{

    //////////////////////////////////////////////////
    //				forward declarations			//
    //////////////////////////////////////////////////
    //implemented
    class Mutex;
    class SpinLock;
    class Condition;

    //////////////////////////////////////////////////
    //				enum declarations				//
    //////////////////////////////////////////////////
    enum ResumeStatus{
        RESUME_TIMEDOUT,
        RESUME_SIGNALED,
        RESUME_JOINED
    };

#if defined USE_WINDOWSTHREAD_INTERFACE
    typedef HANDLE MutexHandle;
#elif defined USE_PTHREAD_INTERFACE
    typedef pthread_mutex_t MutexHandle;
#endif

    //////////////////////////////////////////////////
    //				class declarations				//
    //////////////////////////////////////////////////
    /****************************************/
    /*!
        @class AbstractResourceLocker
        @brief Abstract locker class

        @author Naoto Nakamura
        @date Sep. 15, 2008
    */
    /****************************************/
    class ResourceLock
    {
    public:
        enum LOCK_TYPE{
            LOCK_MUTEX,
            LOCK_SPIN,
        };

    public:
        ResourceLock(){}
        virtual ~ResourceLock(){}

    public:
        virtual bool isLocking() = 0;

        virtual void lock() = 0;
        virtual void unlock() = 0;

    protected:
        LOCK_TYPE mType;	//<! Locker type
    };

    /****************************************/
    /*!
        @class Mutex
        @brief Mutex class

        @author Naoto Nakamura
        @date Sep. 15, 2008
    */
    /****************************************/
    class Mutex : public ResourceLock
    {
    public:
        Mutex():ResourceLock(), mIsLocking(FALSE){
            mType = LOCK_MUTEX;
#if defined USE_WINDOWSTHREAD_INTERFACE
            mMutex = ::CreateMutex(NULL, FALSE, NULL);
#elif defined USE_PTHREAD_INTERFACE
            pthread_mutex_init(&mMutex, NULL);
#endif
        }

        virtual ~Mutex(){
            unlock();
#if defined USE_WINDOWSTHREAD_INTERFACE
            ::CloseHandle(mMutex);
#elif defined USE_PTHREAD_INTERFACE
            pthread_mutex_destroy(&mMutex);
#endif
        }

    public:
        virtual bool isLocking(){return mIsLocking;}

        MutexHandle getMutexHandle(){return mMutex;}

        virtual void lock(){
#if defined USE_WINDOWSTHREAD_INTERFACE
            ::WaitForSingleObject(mMutex, 0xffffffff);
#elif defined USE_PTHREAD_INTERFACE
            pthread_mutex_lock(&mMutex);
#endif
            mIsLocking = TRUE;
        }

        virtual void unlock(){
#if defined USE_WINDOWSTHREAD_INTERFACE
            if(::ReleaseMutex(mMutex) != 0 && mIsLocking)mIsLocking = FALSE;
#elif defined USE_PTHREAD_INTERFACE
            pthread_mutex_unlock(&mMutex);
#endif
            mIsLocking = FALSE;
        }

    private:
        bool mIsLocking;			//<! Flog describing if object i locking

    protected:
        MutexHandle mMutex;				//<! Mutex hundle
    };


    /****************************************/
    /*!
        @class SpinLock
        @brief Locker class with no wait

        @author Naoto Nakamura
        @date Sep. 15, 2008
    */
    /****************************************/
    class SpinLock : public ResourceLock
    {
    public:
        SpinLock()
            :ResourceLock(), mIsLocked(0){};
        
        virtual ~SpinLock(){unlock();}

    public:
        bool tryLock()
        {
#if defined COMPILER_MSVC
            long ret = InterlockedExchange(&mIsLocked, 1 );
            MemoryBarrier();
            return ret == 0;
#elif defined COMPILER_GCC
            return __sync_bool_compare_and_swap(&mIsLocked, 0, 1);
#endif
        }

        virtual void lock()
        {
            while(!tryLock()){
#if defined COMPILER_MSVC
                Sleep(0);
#elif defined COMPILER_GCC
                sched_yield();
#endif
            }
        }

        virtual void unlock()
        {
#if defined COMPILER_MSVC
            MemoryBarrier();
#elif defined COMPILER_GCC
#endif
            *const_cast< long volatile* >( &mIsLocked ) = 0;
        }

        virtual bool isLocking(){return mIsLocked != 0;}

    private:
        long mIsLocked;

    };
    /****************************************/
    /*!
        @class	Condition
        @brief	Condition variable

        @author Naoto Nakamura
        @date Sep. 15, 2008
    */
    /****************************************/
    class Condition : public Mutex
    {
    public:
        Condition():Mutex()
            {
#if defined USE_WINDOWSTHREAD_INTERFACE
                mNumWaiting = 0;
                mGeneration = 0;
                mNumWaiting = 0;
                mNumWake = 0;
                mSemaphore = ::CreateSemaphore(NULL, 0, LONG_MAX, NULL);
                ::InitializeCriticalSection(&mCriticalSection);
#elif defined USE_PTHREAD_INTERFACE
            pthread_cond_init(&mCondition, NULL);
#endif

        }
        virtual ~Condition(){
#if defined USE_WINDOWSTHREAD_INTERFACE
#elif defined USE_PTHREAD_INTERFACE
            pthread_cond_destroy(&mCondition);
#endif
        }

    public:

        void wait(unsigned long time = 0xffffff, Mutex *mutex = NULL){timedwait(time, mutex);}
        ResumeStatus timedwait(unsigned long timeoutMilliSec, Mutex *mutex = NULL);

        void signal(){
#if defined USE_WINDOWSTHREAD_INTERFACE
            unsigned int wake = 0;
            ::EnterCriticalSection(&mCriticalSection);
            if (mNumWaiting > mNumWake) {
                wake = 1;
                mNumWake++;
                mGeneration++;
            }
            ::LeaveCriticalSection(&mCriticalSection);

            if (wake) {
                ::ReleaseSemaphore(mSemaphore, 1, NULL);
            }
#elif defined USE_PTHREAD_INTERFACE
            pthread_cond_signal(&mCondition);
#endif
        }

        void signalAll(){

#if defined USE_WINDOWSTHREAD_INTERFACE
            unsigned long numWake = 0;

            ::EnterCriticalSection(&mCriticalSection);
            if (mNumWaiting > mNumWake) {
                numWake = mNumWaiting - mNumWake;
                mNumWake = mNumWaiting;
                mGeneration++;
            }
            ::LeaveCriticalSection(&mCriticalSection);

            if (numWake) {
                ::ReleaseSemaphore(mSemaphore, numWake, NULL);
            }
#elif defined USE_PTHREAD_INTERFACE
            pthread_cond_broadcast(&mCondition);
#endif

        }

    private:
#if defined USE_WINDOWSTHREAD_INTERFACE
        HANDLE mSemaphore;					//<! Semaphore for manage threads
        CRITICAL_SECTION mCriticalSection;	//<! Critical section for thread safe
        unsigned long mNumWaiting;			//<! The number of waiting threads
        unsigned long mGeneration;			//<! The generation of blocking
        unsigned long mNumWake;				//<! The number of threads sending signal
#elif defined USE_PTHREAD_INTERFACE
        pthread_cond_t mCondition;		//<! Condition descriptor
#endif
    };

    class LockHolder
    {
    public:
        LockHolder(ResourceLock *locker)
        :mLocker(locker)
        {
            mLocker->lock();
        }
        
        ~LockHolder()
        {
            mLocker->unlock();
        }
        
    private:
        ResourceLock *mLocker;

    };

}; //namespace SThread

#endif //STHREAD_LOCK_H
