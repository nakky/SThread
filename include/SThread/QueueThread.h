/******************************************************************/
/*!
	@file	QueueThread.h
	@brief	Worker thread implementation(QueueThread, WorkerThread)
	@note	Worker thread pattern is a kind of patterns of
			thread classes, which idle(wait) when they has no task,
			and if a task is put, they resume and process the task.
	@todo
	@bug

	@author	Naoto Nakamura
	@date	Sep. 15, 2008
*/
/******************************************************************/

#ifndef STHREAD_QUEUERTHREAD_H
#define STHREAD_QUEUERTHREAD_H

#include "SThread/Common.h"

#include <set>
#include <deque>
#include <atomic>

#include "SThread/Thread.h"


namespace SThread{
    //////////////////////////////////////////////////
    //				forward declarations			//
    //////////////////////////////////////////////////
    //implemented
    class WorkRequest;

    class QueueThread;
    class WorkerThread;

    //////////////////////////////////////////////////
    //				class declarations				//
    //////////////////////////////////////////////////
    /****************************************/
    /*!
        @class	WorkRequestAbstract
        @brief	Task class request for the queue/worker thread

        @author	Naoto Nakamura
        @date	Sep. 15, 2008
    */
    /****************************************/
    class WorkRequest
    {
        friend class QueueThread;
        friend class WorkerThread;
    public:
        //! priority for worker thread class
        enum WorkPriority{
            PRIORITY_IMMEDIATE = 0x7FFFFFFF,
            PRIORITY_HIGHBITS = 0x70000000,
            PRIORITY_URGENT = 0x40000000,
            PRIORITY_HIGH = 0x30000000,
            PRIORITY_NORMAL = 0x20000000,
            PRIORITY_LOW = 0x10000000,
            PRIORITY_LOWEST = 0x0FFFFFFF
        };

        enum WorkState{
            WORK_VOID,			//!< The void task(only return parameter)
            WORK_NOTPROGRESS,	//!< The task do not progress
            WORK_INPROGRESS,	//!< The task progress
            WORK_COMPLETED,		//!< The task is done
            WORK_INCOMPLETED,	//!< The task is done, but the task do not return OK
            WORK_ABORTED		//!< The task is aborted
        };

        explicit WorkRequest(int priority = PRIORITY_NORMAL, bool isAutoDeleteObject = TRUE)
        :mState(WORK_NOTPROGRESS),
        mPriority(priority),
        mAutoDeletedObject(isAutoDeleteObject),
        mIsReseted(TRUE),
        mIsChecked(FALSE),
        mCondition(NULL)
        {
        }
        virtual ~WorkRequest(){};

    private:
        virtual bool work() = 0;

    protected:
        void setState(WorkState state){
            WorkState oldState;
            do{
                oldState = mState;
            }while(!mState.compare_exchange_weak(oldState, state));
        }
        
        virtual void onChecked(){
            mIsChecked = true;
        }


    public:

        virtual void init(){}
        virtual void cleanup(){}

        WorkState getState(){
            return mState.load();
        }
        void resetState(){
            setState(WORK_NOTPROGRESS);
            mIsChecked = false;
            mIsReseted = true;
        }

        int getPriority(){return mPriority;}
        void setPriority(int priority){ mPriority = priority; }
        
        bool isDone()
        {
            if (!mIsChecked) return false;

            WorkState state =mState.load();
            bool ret = state == WORK_COMPLETED || state == WORK_INCOMPLETED || state == WORK_ABORTED;
            return ret;
        }

        virtual void setAbort(){
            setState(WORK_ABORTED);
        }

        bool isAutoDeletedObject(){return mAutoDeletedObject;}

        bool higherPriority(const WorkRequest &target)const{
            if ( mPriority == target.mPriority)
                return this > &target;
            else
                return mPriority > target.mPriority;
        }

        bool isReseted() {
            return mIsReseted;
        }

    protected:
        std::atomic<WorkState> mState;
        int mPriority;				//!< Priority which is effective for WorkerThread(enum WorkRequestAbstract::WorkPriority)

        bool mAutoDeletedObject;	//!< Flag describe if the request is deleted when processing is done
        bool mIsReseted;

        bool mIsChecked;
        
        Condition *mCondition;
    };
    
    /****************************************/
    /*!
     @class    RequestContainer
     @brief
     
     @author    Naoto Nakamura
     @date    Sep. 15, 2008
     */
    /****************************************/
    class RequestContainer
    {
    public:
        RequestContainer()
        {}
        
        virtual ~RequestContainer(){}
        
    public:
        virtual bool add(WorkRequest *req) = 0;
        virtual bool erase(WorkRequest *req) = 0;

        virtual void init() = 0;
        virtual void cleanup() = 0;

        virtual void clear() = 0;

        virtual int getNum() = 0;
        virtual WorkRequest* pop() = 0;
    };
    
    class QueueRequestContainer : public RequestContainer
    {
    public:
        QueueRequestContainer()
        :RequestContainer()
        {}
        
        virtual ~QueueRequestContainer(){}
        
    public:
        virtual bool add(WorkRequest *req)
        {
            mLocker.lock();
            mRequestQueue.push_back(req);
            mLocker.unlock();
            return TRUE;
        }
        
        virtual bool erase(WorkRequest *req)
        {
            mLocker.lock();
            std::deque<WorkRequest*>::iterator ite = mRequestQueue.begin();
            while(mRequestQueue.end() != ite){
                if(*ite == req){
                    mRequestQueue.erase(ite);
                    mLocker.unlock();
                    return TRUE;
                }
                ite++;
            }
            mLocker.unlock();
            return FALSE;
        }
        
        virtual int getNum()
        {
            mLocker.lock();
            int ret = (int)mRequestQueue.size();
            mLocker.unlock();
            return ret;
        }

        virtual void init(){}
        virtual void cleanup(){}

        virtual void clear()
        {
            mLocker.lock();
            mRequestQueue.clear();
            mLocker.unlock();
        }
        
        virtual WorkRequest* pop()
        {
            mLocker.lock();
            if(mRequestQueue.size() == 0){
                mLocker.unlock();
                return NULL;
            }
            WorkRequest *ret = mRequestQueue.front();
            mRequestQueue.pop_front();
            mLocker.unlock();
            return ret;
        }
        
    private:
        std::deque<WorkRequest*> mRequestQueue;
        
        SpinLock mLocker;
    };
    
    class WorkerRequestContainer : public RequestContainer
    {
    public:
        WorkerRequestContainer()
        :RequestContainer()
        {}
        
        virtual ~WorkerRequestContainer(){}
        
        struct RequestLess
        {
            bool operator()(const WorkRequest* lhs, const WorkRequest* rhs) const
            {
                return lhs->higherPriority(*rhs);
            }
        };

        
    public:
        virtual bool add(WorkRequest *req)
        {
            mRequestSet.insert(req);
            return TRUE;
        }
        
        virtual bool erase(WorkRequest *req)
        {
            mRequestSet.erase(req);
            return FALSE;
        }
        
        virtual int getNum()
        {
            return (int)mRequestSet.size();
        }
        
        virtual void clear()
        {
            mRequestSet.clear();
        }
        
        virtual WorkRequest* pop()
        {
            if(mRequestSet.size() == 0) return NULL;
            WorkRequest *ret  = *mRequestSet.begin();
            mRequestSet.erase(ret);
            return ret;
        }
        
    private:
        std::multiset<WorkRequest*, RequestLess> mRequestSet;

    };


    /****************************************/
    /*!
        @class	QueueThread
        @brief	Thread which contains tasks "FIFO"

        @author	Naoto Nakamura
        @date	Sep. 15, 2008
    */
    /****************************************/
    class QueueThread : public Thread
    {
    public:
        QueueThread(
                    RequestContainer *container = NULL,
                    bool isComtainerAutoDelete = TRUE,
                    const unsigned long idleTime = 0xFFFFFFFF,
                    Condition *sharedCondition = NULL,
                    const int priority = PRIORITY_NORMAL,
                    const int bindIndex = -1);
        
        virtual ~QueueThread(){}
        
    protected:
        virtual void run();
        virtual WorkRequest::WorkState processNextWork();
        virtual bool workRequest(WorkRequest *request);

    public:
        virtual void init();
        virtual void cleanup();
    
        int getNumWork(){
            mRequestCondition.lock();
            int ret = (int)mRequestContainer->getNum();
            mRequestCondition.unlock();
            return ret;
        }

        bool isProcessing(){
            mProcessingLocker->lock();
            bool ret = mIsProcessing;
            mProcessingLocker->unlock();
            return ret;
        }

        virtual bool addRequest(WorkRequest *req, const bool resume = TRUE);
        virtual bool eraseRequest(WorkRequest *req);

        void clearAllRequest();

        virtual bool shutdown();

        virtual bool suspend();
        virtual bool resume();

        void signalAll(){ mRequestCondition.signalAll(); }
    protected:
        Condition mRequestCondition;

        ResourceLock *mWorkLocker;
        ResourceLock *mProcessingLocker;

        std::atomic<bool> mIsSuspended;
        Condition mSupendCondition;

        bool mIsProcessing;
        
        RequestContainer *mRequestContainer;
        bool mIsComtainerAutoDelete;
        
        unsigned long mIdleTime;
    };

    
}; //namespace SThread


#endif //STHREAD_QUEUERTHREAD_H
