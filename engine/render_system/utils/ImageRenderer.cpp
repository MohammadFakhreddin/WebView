#include "ImageRenderer.hpp"

#include "LogicalDevice.hpp"
#include "RenderBackend.hpp"

namespace MFA
{

    //------------------------------------------------------------------------------------------------------------------

    ImageRenderer::ImageRenderer(std::shared_ptr<Pipeline> pipeline)
        : _pipeline(std::move(pipeline))
    {}

    //------------------------------------------------------------------------------------------------------------------

    std::unique_ptr<ImageRenderer::ImageData> ImageRenderer::AllocateImageData(
        RT::GpuTexture const & gpuTexture,
        Extent const & extent
    ) const
    {
        auto const device = LogicalDevice::Instance;

        Pipeline::Vertex vertexData[4]
        {
            Pipeline::Vertex{.position = {extent.x, extent.y}, .uv = {0, 0}},
            Pipeline::Vertex{.position = {extent.x + extent.width, extent.y}, .uv = {1, 0}},
            Pipeline::Vertex{.position = {extent.x, extent.y + extent.height}, .uv = {0, 1}},
            Pipeline::Vertex{.position = {extent.x + extent.width, extent.y + extent.height}, .uv = {1, 1}},
        };

        auto const vertexBuffer = RB::CreateVertexBufferGroup(
            device->GetVkDevice(),
            device->GetPhysicalDevice(),
            sizeof(vertexData),
            (int)device->GetMaxFramePerFlight()
        );

        auto const vertexStageBuffer = RB::CreateStageBuffer(
            device->GetVkDevice(),
            device->GetPhysicalDevice(),
            vertexBuffer->bufferSize,
            vertexBuffer->buffers.size()
        );

        auto imageData = std::make_unique<ImageData>(ImageData{
            .extent = extent,
            .vertexData = LocalBufferTracker(vertexBuffer, vertexStageBuffer, Alias(vertexData)),
            .descriptorSet = _pipeline->CreateDescriptorSet(gpuTexture)
        });

        return imageData;
    }

    //------------------------------------------------------------------------------------------------------------------
    // TODO: I need to use viewport and scissor to render within area.
    void ImageRenderer::Draw(
        RT::CommandRecordState & recordState,
        Pipeline::PushConstants const & pushConstants,
        ImageData const & imageData
    ) const
    {
        _pipeline->BindPipeline(recordState);

        _pipeline->SetPushConstant(recordState, pushConstants);

        RB::AutoBindDescriptorSet(recordState, RB::UpdateFrequency::PerPipeline, imageData.descriptorSet);

        RB::BindVertexBuffer(recordState, *imageData.vertexData->LocalBuffer().buffers[recordState.frameIndex]);

        vkCmdDraw(recordState.commandBuffer, 4, 1, 0, 0);
    }

    //------------------------------------------------------------------------------------------------------------------

}