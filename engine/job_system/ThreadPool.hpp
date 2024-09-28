#pragma once

#include "ThreadSafeQueue.hpp"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <vector>

namespace MFA
{

    class ThreadPool
    {
    public:

        using Task = std::function<void()>;
        
        explicit ThreadPool();
        // We can have a threadPool with custom number of threads

        ~ThreadPool();

        ThreadPool(ThreadPool const &) noexcept = delete;
        ThreadPool(ThreadPool &&) noexcept = delete;
        ThreadPool & operator = (ThreadPool const &) noexcept = delete;
        ThreadPool & operator = (ThreadPool &&) noexcept = delete;

        [[nodiscard]]
        bool IsMainThread() const;

        void AssignTask(Task const & task);

        [[nodiscard]]
        int NumberOfAvailableThreads() const;
        
        class ThreadObject
        {
        public:

            explicit ThreadObject(int threadNumber, ThreadPool & parent);

            ~ThreadObject() = default;

            ThreadObject(ThreadObject const &) noexcept = delete;
            ThreadObject(ThreadObject &&) noexcept = delete;
            ThreadObject & operator = (ThreadObject const &) noexcept = delete;
            ThreadObject & operator = (ThreadObject &&) noexcept = delete;

            void Join() const;

            [[nodiscard]]
            bool IsFree();

            void Notify();

            [[nodiscard]]
            int GetThreadNumber() const;

            bool AwakeCondition(int idx);

        private:

            void mainLoop();
            
            ThreadPool & mParent;

            int mThreadNumber;

            std::condition_variable mCondition;

            std::unique_ptr<std::thread> mThread;

            std::atomic<bool> mIsBusy = false;

        };

        bool AllThreadsAreIdle() const;

        std::vector<std::string> Exceptions();

    private:
        
        std::vector<std::unique_ptr<ThreadObject>> mThreadObjects;

        bool mIsAlive = true;

        int mNumberOfThreads = 0;

        ThreadSafeQueue<std::string> mExceptions{};

        std::vector<ThreadSafeQueue<Task>> mTasks{};
        int mNextTaskIdx {};

        std::thread::id mMainThreadId{};

    };

}
