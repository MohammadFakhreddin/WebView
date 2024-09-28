#pragma once

#include "BedrockSignalTypes.hpp"

#include <functional>
#include <vector>

#include "BedrockAssert.hpp"

// TODO Make this file thread safe by having a queue of tasks
namespace MFA
{
    template<typename ... ArgsT>
    class Signal
    {
    public:

        using Listener = std::function<void(ArgsT ... args)>;

        struct Slot
        {
            SignalId id;
            Listener listener;
        };

        template <typename Instance>
        SignalId Register(Instance * obj, void (Instance:: * memFunc)(ArgsT...))
        {
            // Member function wrapper.
            auto wrapperLambda = [obj, memFunc](ArgsT... args)
            {
                (obj->*memFunc)(std::forward<ArgsT>(args)...);
            };

            return Register(std::move(wrapperLambda));
        }

        SignalId Register(const Listener & listener)
        {
            MFA_ASSERT(listener != nullptr);

            mSlots.emplace_back(Slot{ mNextId, listener });
            ++mNextId;
            MFA_ASSERT(mNextId != SignalIdInvalid);
            auto const id = mSlots.back().id;
            
            return id;
        }

        bool UnRegister(SignalId listenerId)
        {
            if (listenerId != SignalIdInvalid)
            {
                for (int i = static_cast<int>(mSlots.size() - 1); i >= 0; --i)
                {
                    if (mSlots[i].id == listenerId)
                    {
                        mSlots[i] = mSlots.back();
                        mSlots.pop_back();
                        return true;
                    }
                }
            }
            return false;
        }

        void Emit(ArgsT ... args)
        {
            std::vector<Listener> listeners {};
            {
                for (auto & slot : mSlots)
                {
                    MFA_ASSERT(slot.listener != nullptr);
                    listeners.emplace_back(slot.listener);
                }
            }

            for (auto & listener : listeners)
            {
                listener(std::forward<ArgsT>(args)...);
            }
        }

        [[nodiscard]]
        bool IsEmpty()
        {
            return mSlots.empty();
        }

    private:

        std::vector<Slot> mSlots{};

        SignalId mNextId = 0;

    };
};
