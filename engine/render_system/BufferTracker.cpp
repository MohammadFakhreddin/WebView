#include "BufferTracker.hpp"

#include "BedrockAssert.hpp"
#include "LogicalDevice.hpp"

namespace MFA
{

    //-----------------------------------------------------------------------------------------------

    HostVisibleBufferTracker::HostVisibleBufferTracker(
        std::shared_ptr<RT::BufferGroup> bufferGroup, 
        Alias const & data
    ) : HostVisibleBufferTracker(std::move(bufferGroup))
    {
        for (auto const& buffer : mBufferGroup->buffers)
        {
            RB::UpdateHostVisibleBuffer(
                LogicalDevice::Instance->GetVkDevice(),
                *buffer,
                Alias(data.Ptr(), data.Len())
            );
        }
    }
    
    //-----------------------------------------------------------------------------------------------
    
    HostVisibleBufferTracker::HostVisibleBufferTracker(std::shared_ptr<RT::BufferGroup> bufferGroup)
        : mBufferGroup(std::move(bufferGroup))
    {
        mData = Memory::AllocSize(mBufferGroup->bufferSize);
        mDirtyCounter = 0;
    }

    //-----------------------------------------------------------------------------------------------
    
    void HostVisibleBufferTracker::Update(RT::CommandRecordState const & recordState)
    {
        if (mDirtyCounter > 0)
        {
            RB::UpdateHostVisibleBuffer(
                LogicalDevice::Instance->GetVkDevice(),
                *mBufferGroup->buffers[recordState.frameIndex],
                Alias(mData->Ptr(), mData->Len())
            );
            --mDirtyCounter;
        }
    }
    
    //-----------------------------------------------------------------------------------------------
    
    void HostVisibleBufferTracker::SetData(Alias const & data)
    {
        mDirtyCounter = mBufferGroup->buffers.size();
        MFA_ASSERT(data.Len() <= mData->Len());
        std::memcpy(mData->Ptr(), data.Ptr(), data.Len());
    }

    //-----------------------------------------------------------------------------------------------
    
    uint8_t * HostVisibleBufferTracker::Data()
    {
        mDirtyCounter = mBufferGroup->buffers.size();
        return mData->Ptr();
    }
    
    //-----------------------------------------------------------------------------------------------
    
    LocalBufferTracker::LocalBufferTracker(
        std::shared_ptr<RT::BufferGroup> localBuffer,
        std::shared_ptr<RT::BufferGroup> hostVisibleBuffer
    )
        : mLocalBuffer(std::move(localBuffer))
        , mHostVisibleBuffer(std::move(hostVisibleBuffer))
    {
        mData = Memory::AllocSize(mLocalBuffer->bufferSize);
    }
    
    //-----------------------------------------------------------------------------------------------
    
    void LocalBufferTracker::Update(RT::CommandRecordState const & recordState)
    {
        if (mDirtyCounter > 0)
        {
            RB::UpdateHostVisibleBuffer(
                LogicalDevice::Instance->GetVkDevice(),
                *mHostVisibleBuffer->buffers[recordState.frameIndex],
                Alias(mData->Ptr(), mData->Len())
            );
            RB::UpdateLocalBuffer(
                recordState.commandBuffer,
                *mLocalBuffer->buffers[recordState.frameIndex],
                *mHostVisibleBuffer->buffers[recordState.frameIndex]
            );
            --mDirtyCounter;
        }
    }

    //-----------------------------------------------------------------------------------------------
    
    void LocalBufferTracker::SetData(Alias const & data)
    {
        mDirtyCounter = mLocalBuffer->buffers.size();
        MFA_ASSERT(data.Len() <= mData->Len());
        std::memcpy(mData->Ptr(), data.Ptr(), data.Len());
    }

    //-----------------------------------------------------------------------------------------------
    
    uint8_t * LocalBufferTracker::Data()
    {
        // Returns the data and resets the counter.
        mDirtyCounter = LogicalDevice::Instance->GetMaxFramePerFlight();
        return mData->Ptr();
    }
    
    //-----------------------------------------------------------------------------------------------
    
    RT::BufferGroup const & LocalBufferTracker::HostVisibleBuffer() const
    {
        return *mHostVisibleBuffer;
    }

    //-----------------------------------------------------------------------------------------------
    
    RT::BufferGroup const & LocalBufferTracker::LocalBuffer() const
    {
        return *mLocalBuffer;
    }
    
    //-----------------------------------------------------------------------------------------------
    
}
