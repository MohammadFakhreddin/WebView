#include "PointPipeline.hpp"

#include "RenderBackend.hpp"
#include "BedrockAssert.hpp"
#include "BedrockPath.hpp"
#include "DescriptorSetSchema.hpp"
#include "LogicalDevice.hpp"
#include "ImportShader.hpp"

namespace MFA
{
    //-------------------------------------------------------------------------------------------------

    PointPipeline::PointPipeline(
        std::shared_ptr<DisplayRenderPass> displayRenderPass,
        std::shared_ptr<RT::BufferGroup> viewProjectionBuffer,
        int const maxSets
    )
    {
        mDisplayRenderPass = std::move(displayRenderPass);

        mViewProjBuffer = std::move(viewProjectionBuffer);

        mDescriptorPool = RB::CreateDescriptorPool(
            LogicalDevice::Instance->GetVkDevice(),
            maxSets
        );
        CreateDescriptorSetLayout();
        CreatePipeline();
        CreateDescriptorSets();
    }

    //-------------------------------------------------------------------------------------------------

    PointPipeline::~PointPipeline()
    {
        mPipeline = nullptr;
        mDescriptorSetLayout = nullptr;
        mDescriptorPool = nullptr;
    }

    //-------------------------------------------------------------------------------------------------

    bool PointPipeline::IsBinded(RT::CommandRecordState const& recordState) const
    {
        if (recordState.pipeline == mPipeline.get())
        {
            return true;
        }
        return false;
    }

    //-------------------------------------------------------------------------------------------------

    void PointPipeline::BindPipeline(RT::CommandRecordState& recordState) const
    {
        if (IsBinded(recordState))
        {
            return;
        }

        RB::BindPipeline(recordState, *mPipeline);
        RB::AutoBindDescriptorSet(recordState, RB::UpdateFrequency::PerPipeline, mDescriptorSetGroup);
    }

    //-------------------------------------------------------------------------------------------------

    void PointPipeline::SetPushConstants(
        RT::CommandRecordState& recordState,
        PushConstants pushConstants
    ) const
    {
        RB::PushConstants(
            recordState,
            mPipeline->pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            Alias(pushConstants)
        );
    }

    //-------------------------------------------------------------------------------------------------

    void PointPipeline::CreateDescriptorSetLayout()
    {
        std::vector<VkDescriptorSetLayoutBinding> bindings{};

        // ModelViewProjection
        VkDescriptorSetLayoutBinding layoutBinding{
            .binding = static_cast<uint32_t>(bindings.size()),
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT
        };
        bindings.emplace_back(layoutBinding);

        MFA_ASSERT(mDescriptorSetLayout == VK_NULL_HANDLE);
        mDescriptorSetLayout = RB::CreateDescriptorSetLayout(
            LogicalDevice::Instance->GetVkDevice(),
            static_cast<uint8_t>(bindings.size()),
            bindings.data()
        );
    }

    //-------------------------------------------------------------------------------------------------

    void PointPipeline::CreatePipeline()
    {
        // Vertex shader
        auto cpuVertexShader = Importer::ShaderFromSPV(
            Path::Instance->Get("engine/shaders/point_pipeline/PointPipeline.vert.spv"),
            VK_SHADER_STAGE_VERTEX_BIT,
            "main"
        );
        auto gpuVertexShader = RB::CreateShader(
            LogicalDevice::Instance->GetVkDevice(),
            cpuVertexShader
        );

        // Fragment shader
        auto cpuFragmentShader = Importer::ShaderFromSPV(
            Path::Instance->Get("engine/shaders/point_pipeline/PointPipeline.frag.spv"),
            VK_SHADER_STAGE_FRAGMENT_BIT,
            "main"
        );
        auto gpuFragmentShader = RB::CreateShader(
            LogicalDevice::Instance->GetVkDevice(),
            cpuFragmentShader
        );

        std::vector<RT::GpuShader const*> shaders{ gpuVertexShader.get(), gpuFragmentShader.get() };

        VkVertexInputBindingDescription const bindingDescription{
            .binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        };

        std::vector<VkVertexInputAttributeDescription> inputAttributeDescriptions{};
        // Position
        inputAttributeDescriptions.emplace_back(VkVertexInputAttributeDescription{
            .location = static_cast<uint32_t>(inputAttributeDescriptions.size()),
            .binding = 0,
            .format = VK_FORMAT_R32G32B32_SFLOAT,
            .offset = offsetof(Vertex, position),
            });

        RB::CreateGraphicPipelineOptions pipelineOptions{};
        pipelineOptions.useStaticViewportAndScissor = false;
        pipelineOptions.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        // TODO I think we should submit each pipeline . Each one should have independent depth buffer 
        pipelineOptions.rasterizationSamples = LogicalDevice::Instance->GetMaxSampleCount();            // TODO Find a way to set sample count to 1. We only need MSAA for pbr-pipeline
        pipelineOptions.cullMode = VK_CULL_MODE_NONE;
        pipelineOptions.colorBlendAttachments.blendEnable = VK_FALSE;

        // pipeline layout
        std::vector<VkPushConstantRange> const pushConstantRanges{
            VkPushConstantRange {
                .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
                .offset = 0,
                .size = sizeof(PushConstants),
            }
        };
        const auto pipelineLayout = RB::CreatePipelineLayout(
            LogicalDevice::Instance->GetVkDevice(),
            1,
            &mDescriptorSetLayout->descriptorSetLayout,
            static_cast<uint32_t>(pushConstantRanges.size()),
            pushConstantRanges.data()
        );

        auto surfaceCapabilities = LogicalDevice::Instance->GetSurfaceCapabilities();

        mPipeline = RB::CreateGraphicPipeline(
            LogicalDevice::Instance->GetVkDevice(),
            static_cast<uint8_t>(shaders.size()),
            shaders.data(),
            1,
            &bindingDescription,
            static_cast<uint8_t>(inputAttributeDescriptions.size()),
            inputAttributeDescriptions.data(),
            surfaceCapabilities.currentExtent,
            mDisplayRenderPass->GetVkRenderPass(),
            pipelineLayout,
            pipelineOptions
        );
    }

    //-------------------------------------------------------------------------------------------------

    void PointPipeline::CreateDescriptorSets()
    {
        auto const maxFramesPerFlight = LogicalDevice::Instance->GetMaxFramePerFlight();
        mDescriptorSetGroup = RB::CreateDescriptorSet(
            LogicalDevice::Instance->GetVkDevice(),
            mDescriptorPool->descriptorPool,
            mDescriptorSetLayout->descriptorSetLayout,
            maxFramesPerFlight
        );

        for (uint32_t frameIndex = 0; frameIndex < maxFramesPerFlight; ++frameIndex)
        {

            auto const& descriptorSet = mDescriptorSetGroup.descriptorSets[frameIndex];
            MFA_ASSERT(descriptorSet != VK_NULL_HANDLE);

            DescriptorSetSchema descriptorSetSchema{ descriptorSet };

            /////////////////////////////////////////////////////////////////
            // Vertex shader
            /////////////////////////////////////////////////////////////////

            // ViewProjectionTransform
            VkDescriptorBufferInfo bufferInfo{
                .buffer = mViewProjBuffer->buffers[frameIndex]->buffer,
                .offset = 0,
                .range = mViewProjBuffer->bufferSize,
            };
            descriptorSetSchema.AddUniformBuffer(&bufferInfo);

            descriptorSetSchema.UpdateDescriptorSets();
        }
    }

    //-------------------------------------------------------------------------------------------------

}