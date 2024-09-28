#include "ThreadPool.hpp"

namespace MFA
{

    //-------------------------------------------------------------------------------------------------

    ThreadPool::ThreadPool()
    {
        mMainThreadId = std::this_thread::get_id();
        mNumberOfThreads = static_cast<int>(std::thread::hardware_concurrency() * 0.75f);
        MFA_LOG_INFO("Job system is running on %d threads. Available threads are: %d", mNumberOfThreads, static_cast<int>(std::thread::hardware_concurrency()));
        if (mNumberOfThreads < 2)
        {
            mIsAlive = false;
        }
        else
        {
            mIsAlive = true;
            mTasks = std::vector<ThreadSafeQueue<Task>>(mNumberOfThreads);
        	for (int threadIndex = 0; threadIndex < mNumberOfThreads; threadIndex++)
            {
                mThreadObjects.emplace_back(std::make_unique<ThreadObject>(threadIndex, *this));
            }
        }

    }

    //-------------------------------------------------------------------------------------------------

    bool ThreadPool::IsMainThread() const
    {
        return std::this_thread::get_id() == mMainThreadId;
    }

    //-------------------------------------------------------------------------------------------------

    ThreadPool::~ThreadPool()
    {
        mIsAlive = false;
        for (auto const & thread : mThreadObjects)
        {
            thread->Notify();
        }
        for (auto const & thread : mThreadObjects)
        {
            thread->Join();
        }
    }

    //-------------------------------------------------------------------------------------------------

    void ThreadPool::AssignTask(Task const & task)
    {
        assert(task != nullptr);

        if (mIsAlive == true)
        {
            mTasks[mNextTaskIdx].Push(task);
            mThreadObjects[mNextTaskIdx]->Notify();
            mNextTaskIdx = (mNextTaskIdx + 1) % mThreadObjects.size();
        }
        else
        {
            task();
        }
    }

    //-------------------------------------------------------------------------------------------------

    ThreadPool::ThreadObject::ThreadObject(int const threadNumber, ThreadPool & parent)
        :
        mParent(parent),
        mThreadNumber(threadNumber)
    {
        mThread = std::make_unique<std::thread>([this]()-> void
        {
            mainLoop();
        });
    }

    //-------------------------------------------------------------------------------------------------

    int ThreadPool::NumberOfAvailableThreads() const
    {
        return mNumberOfThreads;
    }

    //-------------------------------------------------------------------------------------------------

    bool ThreadPool::ThreadObject::AwakeCondition(int idx)
    {
        return mParent.mTasks[idx].IsEmpty() == false || mParent.mIsAlive == false;
    }

    //-------------------------------------------------------------------------------------------------

    void ThreadPool::ThreadObject::Join() const
    {
        mThread->join();
    }

    //-------------------------------------------------------------------------------------------------

    bool ThreadPool::ThreadObject::IsFree()
    {
        auto const isFree = mIsBusy == false;
        if (isFree == false)
        {
            mCondition.notify_one();
        }
        return isFree;
    }

    //-------------------------------------------------------------------------------------------------

    void ThreadPool::ThreadObject::Notify()
    {
        mCondition.notify_one();
    }

    //-------------------------------------------------------------------------------------------------

    int ThreadPool::ThreadObject::GetThreadNumber() const
    {
        return mThreadNumber;
    }

    //-------------------------------------------------------------------------------------------------

    void ThreadPool::ThreadObject::mainLoop()
    {
        std::mutex mutex{};
        std::unique_lock<std::mutex> mLock{ mutex };
        while (mParent.mIsAlive)
        {
            mCondition.wait(mLock, [this]()->bool
                {
                    return AwakeCondition(mThreadNumber);
                }
            );
            mIsBusy = true;
            while (mParent.mIsAlive)
            {
                Task currentTask;
                bool isEmpty = false;
                while (mParent.mTasks[mThreadNumber].TryToPop(currentTask, isEmpty) == false);
                if (isEmpty == true)
                {
	                break;
                }
                try
                {
                    if (currentTask != nullptr)
                    {
                        currentTask();
                    }
                }
                catch (std::exception const & exception)
                {
                    while (mParent.mExceptions.TryToPush(exception.what()) == false);
                }
            }
            mIsBusy = false;
        }
    }

    //-------------------------------------------------------------------------------------------------

    bool ThreadPool::AllThreadsAreIdle() const
    {
        for (auto const & threadObject : mThreadObjects)
        {
            if (threadObject->IsFree() == false)
            {
                return false;
            }
        }
        return true;
    }

    //-------------------------------------------------------------------------------------------------

    std::vector<std::string> ThreadPool::Exceptions()
    {
        std::vector<std::string> exceptions{};

        while (true)
        {
            std::string exception;
            bool isEmpty = false;
            if (mExceptions.TryToPop(exception, isEmpty))
            {
                if (isEmpty == false)
                {
                    exceptions.emplace_back(exception);
                }
                else
                {
	                break;
                }
            }
        }
        return exceptions;
    }

    //-------------------------------------------------------------------------------------------------

}
