#pragma once

#include "BedrockAssert.hpp"
#include "RenderBackend.hpp"
#include "RenderTypes.hpp"

namespace MFA
{
    struct HostVisibleBufferTracker
    {
    public:

        explicit HostVisibleBufferTracker(std::shared_ptr<RT::BufferGroup> bufferGroup, Alias const & data);

        explicit HostVisibleBufferTracker(std::shared_ptr<RT::BufferGroup> bufferGroup);

        void Update(RT::CommandRecordState const & recordState);

        void SetData(Alias const & data);

        [[nodiscard]]
        uint8_t * Data();

        RT::BufferGroup const & HostVisibleBuffer();

    private:
        
        std::shared_ptr<RT::BufferGroup> mBufferGroup;
        int mDirtyCounter = 0;
        std::unique_ptr<Blob> mData {};
    };

    // Only use it for the data that is frequently updated
    struct LocalBufferTracker
    {
    public:

        explicit LocalBufferTracker(
            std::shared_ptr<RT::BufferGroup> localBuffer,
            std::shared_ptr<RT::BufferGroup> hostVisibleBuffer
        );

        void Update(RT::CommandRecordState const & recordState);

        void SetData(Alias const & data);

        [[nodiscard]]
        uint8_t * Data();

        [[nodiscard]]
        RT::BufferGroup const & HostVisibleBuffer() const;

        [[nodiscard]]
        RT::BufferGroup const & LocalBuffer() const;

    private:

        std::shared_ptr<RT::BufferGroup> mLocalBuffer{};
        std::shared_ptr<RT::BufferGroup> mHostVisibleBuffer{};
        int mDirtyCounter = 0;
        std::unique_ptr<Blob> mData {};
    };
}