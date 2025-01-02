#pragma once

#include "BufferTracker.hpp"
#include "pipeline/BorderPipeline.hpp"

namespace MFA
{
    class BorderRenderer
    {
    public:

        using Pipeline = BorderPipeline;
        using Position = Pipeline::Position;
		using Radius = Pipeline::Radius;
        using Color = Pipeline::Color;
        using Width = Pipeline::Width;

        explicit BorderRenderer(std::shared_ptr<Pipeline> pipeline);

        [[nodiscard]]
        std::unique_ptr<LocalBufferTracker> AllocateBuffer(
            Position const &topLeftPos,
            Position const &bottomLeftPos,
            Position const &topRightPos,
            Position const &bottomRightPos,

            Color const &topLeftColor,
            Color const &bottomLeftColor,
            Color const &topRightColor,
            Color const &bottomRightColor,

            Radius const &topLeftBorderRadius,
            Radius const &bottomLeftBorderRadius,
            Radius const &topRightBorderRadius,
            Radius const &bottomRightBorderRadius,

            Width leftWidth,
            Width topWidth,
            Width rightWidth,
            Width bottomWidth
        ) const;

        void UpdateBuffer(
            LocalBufferTracker &bufferTracker,
            
            Position const &topLeftPos,
            Position const &bottomLeftPos,
            Position const &topRightPos,
            Position const &bottomRightPos,
            
            Color const &topLeftColor,
            Color const &bottomLeftColor,
            Color const &topRightColor,
            Color const &bottomRightColor,
            
            Radius const &topLeftBorderRadius,
            Radius const &bottomLeftBorderRadius,
            Radius const &topRightBorderRadius,
            Radius const &bottomRightBorderRadius,

            Width leftWidth,
            Width topWidth,
            Width rightWidth,
            Width bottomWidth
        ) const;

        void Draw(
            RT::CommandRecordState & recordState,
            Pipeline::PushConstants const & pushConstants,
            LocalBufferTracker const & vertexBuffer
        ) const;

    private:

        std::shared_ptr<Pipeline> _pipeline;

    };
}
