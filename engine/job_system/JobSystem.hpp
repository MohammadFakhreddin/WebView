#pragma once

#include "ThreadPool.hpp"

#include <future>
#include <omp.h>

namespace MFA
{
    class JobSystem
    {
    public:

        static std::unique_ptr<JobSystem> Instantiate()
        {
            return std::make_unique<JobSystem>();
        }

        explicit JobSystem()
        {
            MFA_ASSERT(Instance == nullptr);

			// 80 percent is the best ratio
			omp_set_num_threads(static_cast<int>(static_cast<float>(std::thread::hardware_concurrency()) * 0.8f));
			MFA_LOG_INFO("Number of available workers are: %d", omp_get_max_threads());

            Instance = this;
        }

        ~JobSystem()
        {
            MFA_ASSERT(Instance != nullptr);
            Instance = nullptr;
        }

        std::future<void> AssignTask(std::function<void()>  task)
        {
            struct Params
            {
                std::promise<void> promise{};
            };
            auto params = std::make_shared<Params>();

            threadPool.AssignTask([task, params]()
                {
                    task();
                    params->promise.set_value();
                }
            );
            return params->promise.get_future();
        }

        template<typename T>
        std::future<T> AssignTask(std::function<T()>  task)
        {
            struct Params
            {
                std::promise<T> promise{};
            };
            auto params = std::make_shared<Params>();

            threadPool.AssignTask([task, params]()
                {
                    params->promise.set_value(task());
                }
            );
            return params->promise.get_future();
        }

        [[nodiscard]]
        auto NumberOfAvailableThreads() const
        {
            return threadPool.NumberOfAvailableThreads();
        }

        [[nodiscard]]
        auto IsMainThread() const
        {
            return threadPool.IsMainThread();
        }

        inline static JobSystem* Instance = nullptr;

    private:

        ThreadPool threadPool{};

    };
}


namespace MFA
{
    using JS = JobSystem;
}
