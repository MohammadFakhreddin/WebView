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

        Position const &topLeftPos,
        Position const &bottomLeftPos,
        Position const &topRightPos,
        Position const &bottomRightPos,

        Radius const &topLeftBorderRadius,
        Radius const &bottomLeftBorderRadius,
        Radius const &topRightBorderRadius,
        Radius const &bottomRightBorderRadius
    ) const
    {
        auto const device = LogicalDevice::Instance;

        Pipeline::Vertex vertexData[4]
        {
            Pipeline::Vertex{.position = topLeftPos, .radius = topLeftBorderRadius,.uv = {0, 0}},
            Pipeline::Vertex{.position = bottomLeftPos, .radius = bottomLeftBorderRadius, .uv = {0, 1}},
            Pipeline::Vertex{.position = topRightPos, .radius = topRightBorderRadius, .uv = {1, 0}},
            Pipeline::Vertex{.position = bottomRightPos, .radius = bottomRightBorderRadius, .uv = {1, 1}},
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
            .vertexData = LocalBufferTracker(vertexBuffer, vertexStageBuffer, Alias(vertexData)),
            .descriptorSet = _pipeline->CreateDescriptorSet(gpuTexture)
        });

        return imageData;
    }

    //------------------------------------------------------------------------------------------------------------------
    
    void ImageRenderer::UpdateImageData(
        ImageData &imageData,

        RT::GpuTexture const &gpuTexture,

        Position const &topLeftPos,
        Position const &bottomLeftPos,
        Position const &topRightPos,
        Position const &bottomRightPos,

        Radius const &topLeftBorderRadius,
        Radius const &bottomLeftBorderRadius,
        Radius const &topRightBorderRadius,
        Radius const &bottomRightBorderRadius
    ) const
    {
        MFA_ASSERT(imageData.vertexData.has_value() == true);
        auto * rawData = imageData.vertexData->Data();

        Pipeline::Vertex * vertexData = reinterpret_cast<Pipeline::Vertex *>(rawData);
        vertexData[0].position = topLeftPos;
        vertexData[0].radius = topLeftBorderRadius;
        vertexData[1].position = bottomLeftPos;
        vertexData[1].radius = bottomLeftBorderRadius;
        vertexData[2].position = topRightPos;
        vertexData[2].radius = topRightBorderRadius;
        vertexData[3].position = bottomRightPos;
        vertexData[3].radius = bottomRightBorderRadius;

        // TODO: We need to wrap descriptor sets as well to be freeable
        _pipeline->UpdateDescriptorSet(imageData.descriptorSet, gpuTexture);
    }

    //------------------------------------------------------------------------------------------------------------------

    void ImageRenderer::FreeImageData(ImageData &imageData)
    {
        imageData.vertexData.reset();
        _pipeline->FreeDescriptorSet(imageData.descriptorSet);
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

    	auto const &localBuffers = imageData.vertexData->LocalBuffer().buffers;
        auto const &localBuffer = localBuffers[recordState.frameIndex % localBuffers.size()];
        VkBuffer buffers[2]{localBuffer->buffer, localBuffer->buffer};
        VkDeviceSize bindingOffsets[2]{0, 0};

        vkCmdBindVertexBuffers(recordState.commandBuffer, 0, 2, buffers, bindingOffsets);

        vkCmdDraw(recordState.commandBuffer, 4, 1, 0, 0);
    }

    //------------------------------------------------------------------------------------------------------------------

}
