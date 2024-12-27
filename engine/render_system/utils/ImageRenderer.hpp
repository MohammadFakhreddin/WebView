#pragma once

#include "BufferTracker.hpp"
#include "pipeline/ImagePipeline.hpp"

namespace MFA
{
    class ImageRenderer
    {
    public:

        using Pipeline = ImagePipeline;

        struct Extent
        {
            int x;
            int y;
            int width;
            int height;
        };

        struct ImageData
        {
            Extent extent;
            std::optional<LocalBufferTracker> vertexData = std::nullopt;
            RT::DescriptorSetGroup descriptorSet;
        };

        explicit ImageRenderer(std::shared_ptr<Pipeline> pipeline);

        [[nodiscard]]
        std::unique_ptr<ImageData> AllocateImageData(RT::GpuTexture const & gpuTexture, Extent const & extent) const;

        void Draw(
            RT::CommandRecordState & recordState,
            Pipeline::PushConstants const & pushConstants,
            ImageData const & imageData
        ) const;

        std::shared_ptr<Pipeline> _pipeline;

    };
}
