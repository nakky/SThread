
#include "SThread/QueueThread.h"

namespace SThread{

    //////////////////////////////////////////////////////////////////////
    //						WorkRequestAbstract							//
    //////////////////////////////////////////////////////////////////////

    
    //////////////////////////////////////////////////////////////////////
    //							QueueThread								//
    //////////////////////////////////////////////////////////////////////
    /****************************************/
    /*!
        @brief	Constructor
        @note

        @author	Naoto Nakamura
        @date	Sep. 15, 2008
    */
    /****************************************/
    QueueThread::QueueThread(
                             RequestContainer *container,
                             bool isComtainerAutoDelete,
                             const unsigned long idleTime,
                             Condition *sharedCondition,
                             const int priority,
                             const int bindIndex
                             )
    :Thread(sharedCondition, priority, bindIndex),
    mIsSuspended(FALSE),
    mSupendCondition(),
    mIsProcessing(FALSE),
    mRequestContainer(container),
    mIsComtainerAutoDelete(isComtainerAutoDelete),
    mIdleTime(idleTime)
    {
    }

    void QueueThread::init()
    {
        if(mRequestContainer == NULL){
            mRequestContainer = new QueueRequestContainer();
            mRequestContainer->init();
        }
        
        mWorkLocker = new Mutex();
        mProcessingLocker = new SpinLock();

        Thread::init();
    }
    
    /****************************************/
    /*!
        @brief	Cleanup
        @note

        @author	Naoto Nakamura
        @date	Sep. 15, 2008
    */
    /****************************************/
    void QueueThread::cleanup()
    {
        clearAllRequest();
        Thread::cleanup();
        
        SAFE_DELETE(mWorkLocker);
        SAFE_DELETE(mProcessingLocker);
        
        if(mIsComtainerAutoDelete){
            mRequestContainer->cleanup();
            SAFE_DELETE(mRequestContainer);
        }
    }

    /****************************************/
    /*!
        @brief	Shutdown
        @note

        @author	Naoto Nakamura
        @date	Sep. 15, 2008
    */
    /****************************************/
    bool QueueThread::shutdown()
    {
        if (mState.load() != THREAD_STOPED) {
            setState(THREAD_QUITTING);
        }
        
        resume();
        mRequestCondition.signalAll();

        mWorkLocker->lock();
        Thread::shutdown();
        mWorkLocker->unlock();

        return TRUE;
    }

    /****************************************/
    /*!
        @brief	Suspend the thread
        @note

        @author	Naoto Nakamura
        @date	Sep. 15, 2008
    */
    /****************************************/
    bool QueueThread::suspend()
    {
        
        if(mIsSuspended.load())return FALSE;
        bool expect = FALSE;
        while (!mIsSuspended.compare_exchange_weak(expect, TRUE));
        return TRUE;
    }

    /****************************************/
    /*!
        @brief	Resume the thread
        @note

        @author	Naoto Nakamura
        @date	Sep. 15, 2008
    */
    /****************************************/
    bool QueueThread::resume()
    {
        
        if(!mIsSuspended.load())return FALSE;
        bool expect = TRUE;
        while (!mIsSuspended.compare_exchange_weak(expect, FALSE));
        mSupendCondition.signalAll();
        
        return TRUE;
    }

    /****************************************/
    /*!
        @brief	Function Block which process the thread
        @note	virtual

        @author	Naoto Nakamura
        @date	Sep. 15, 2008
    */
    /****************************************/
    void QueueThread::run()
    {
        int complete = 0;

        while(1){
            mRequestCondition.lock();
            if(mRequestContainer->getNum() <= 0){
                mRequestCondition.wait(mIdleTime);
            }
            mRequestCondition.unlock();

            if(mState.load() != THREAD_RUNNING) break;

            if(mIsSuspended.load()){
                mSupendCondition.wait();
            }

            
            if(mState.load() != THREAD_RUNNING) break;
            mWorkLocker->lock();
            complete = processNextWork();
            mWorkLocker->unlock();

            if(mState.load() != THREAD_RUNNING) break;
        }
    }

    /****************************************/
    /*!
        @brief	Process next task which is waiting
        @note	virtual

        @return Request processing state (enum WorkRequestAbstract::WorkState)

        @author	Naoto Nakamura
        @date	Sep. 15, 2008
    */
    /****************************************/
    WorkRequest::WorkState QueueThread::processNextWork()
    {
        WorkRequest *currentRequest;

        //Get next request
        mRequestCondition.lock();

        if(mRequestContainer->getNum() <= 0){
            mRequestCondition.unlock();
            return WorkRequest::WORK_VOID;
        }

        currentRequest = mRequestContainer->pop();
        if(currentRequest == NULL){
            mRequestCondition.unlock();
            return WorkRequest::WORK_VOID;
        }

        mProcessingLocker->lock();
        mIsProcessing = true;
        mProcessingLocker->unlock();
        
        mRequestCondition.unlock();
        
        WorkRequest::WorkState state = currentRequest->getState();
        switch(state){
            case WorkRequest::WORK_NOTPROGRESS:
                {
                    //process the achieved request
                    currentRequest->setState(WorkRequest::WORK_INPROGRESS);
                    currentRequest->mIsReseted = FALSE;

                    bool isOK = workRequest(currentRequest);

                    if(!currentRequest->isReseted()){
                        if(isOK)currentRequest->setState(WorkRequest::WORK_COMPLETED);
                        else currentRequest->setState(WorkRequest::WORK_INCOMPLETED);
                    }

                    break;
                }
            case WorkRequest::WORK_ABORTED:
                break;
            default:
                break;
        }

        state = currentRequest->getState();
        
        Condition *cond = currentRequest->mCondition;

        //Delete object which auto delete flag is true
        
        if(currentRequest->isAutoDeletedObject()){
            currentRequest->cleanup();
            currentRequest->onChecked();
            delete currentRequest;
            currentRequest = NULL;
        }
        else {
            currentRequest->onChecked();
        }

        if(cond != NULL){
            cond->signalAll();
        }
        
        mProcessingLocker->lock();
        mIsProcessing = false;
        mProcessingLocker->unlock();

        return state;
    }
    
    bool QueueThread::workRequest(WorkRequest *request)
    {
        return request->work();
    }

    /****************************************/
    /*!
        @brief	Add new request
        @note

        @param	req Added request
        @return	return true if processing is valid,
                else return false

        @author	Naoto Nakamura
        @date	Sep. 15, 2008
    */
    /****************************************/
    bool QueueThread::addRequest(WorkRequest *req, const bool resume)
    {
        mRequestCondition.lock();
        
        if (mState.load() == THREAD_QUITTING){
            mRequestCondition.unlock();
            return false;
        }
        mRequestContainer->add(req);
        
        mRequestCondition.unlock();

        if(resume) mRequestCondition.signalAll();
        return true;
    }

    /****************************************/
    /*!
        @brief	Erase contained request
        @note	Object is not deleted,
                only erase from the queue

        @param	req Erased request
        @return	return true if processing is valid,
                else return false

        @author Naoto Nakamura
        @date	Sep. 15, 2008
    */
    /****************************************/
    bool QueueThread::eraseRequest(WorkRequest *req)
    {
        mRequestCondition.lock();
        mRequestContainer->erase(req);
        mRequestCondition.unlock();

        return TRUE;
    }

    /****************************************/
    /*!
        @brief	Clear all queue.
        @note	In this method, objects	which
                auto delete flag is true are deleted

        @author	Naoto Nakamura
        @date	Sep. 15, 2008
    */
    /****************************************/
    void QueueThread::clearAllRequest()
    {
        //mRequestCondition.lock();
        
        WorkRequest *req = mRequestContainer->pop();
        while(req != NULL){
            if(req->isAutoDeletedObject()){
                req->cleanup();
                SAFE_DELETE(req);
            }
            req = mRequestContainer->pop();
        }
        mRequestContainer->clear();
        
        //mRequestCondition.unlock();
    }
    
}; //namespace SThread

