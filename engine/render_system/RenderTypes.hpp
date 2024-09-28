#pragma once

#include "BedrockPlatforms.hpp"

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

namespace MFA
{
	class RenderPass;

	namespace RenderTypes
    {
        struct ImageGroup
        {
            const VkImage image;
            const VkDeviceMemory memory;

            explicit ImageGroup(
                VkImage image_,
                VkDeviceMemory memory_
            );
            ~ImageGroup();

            ImageGroup(ImageGroup const&) noexcept = delete;
            ImageGroup(ImageGroup&&) noexcept = delete;
            ImageGroup& operator= (ImageGroup const& rhs) noexcept = delete;
            ImageGroup& operator= (ImageGroup&& rhs) noexcept = delete;
        };

        struct ImageViewGroup
        {
            VkImageView const imageView;

            explicit ImageViewGroup(VkImageView imageView_);
            ~ImageViewGroup();

            ImageViewGroup(ImageViewGroup const&) noexcept = delete;
            ImageViewGroup(ImageViewGroup&&) noexcept = delete;
            ImageViewGroup& operator= (ImageViewGroup const& rhs) noexcept = delete;
            ImageViewGroup& operator= (ImageViewGroup&& rhs) noexcept = delete;

        };

        struct DepthImageGroup
        {
            explicit DepthImageGroup(
                std::shared_ptr<ImageGroup> imageGroup_,
                std::shared_ptr<ImageViewGroup> imageView_,
                VkFormat imageFormat_
            );
            ~DepthImageGroup();

            DepthImageGroup(DepthImageGroup const&) noexcept = delete;
            DepthImageGroup(DepthImageGroup&&) noexcept = delete;
            DepthImageGroup& operator= (DepthImageGroup const& rhs) noexcept = delete;
            DepthImageGroup& operator= (DepthImageGroup&& rhs) noexcept = delete;

            std::shared_ptr<ImageGroup> const imageGroup;
            std::shared_ptr<ImageViewGroup> const imageView;
            VkFormat const imageFormat;
        };

        struct DescriptorSetLayoutGroup
        {
            explicit DescriptorSetLayoutGroup(VkDescriptorSetLayout descriptorSetLayout_);
            ~DescriptorSetLayoutGroup();

            DescriptorSetLayoutGroup(DescriptorSetLayoutGroup const &) noexcept = delete;
            DescriptorSetLayoutGroup(DescriptorSetLayoutGroup &&) noexcept = delete;
            DescriptorSetLayoutGroup & operator= (DescriptorSetLayoutGroup const & rhs) noexcept = delete;
            DescriptorSetLayoutGroup & operator= (DescriptorSetLayoutGroup && rhs) noexcept = delete;

            VkDescriptorSetLayout const descriptorSetLayout;

        };
    
        struct PipelineGroup
        {
            VkPipelineLayout const pipelineLayout;
            VkPipeline const pipeline;

            explicit PipelineGroup(
                VkPipelineLayout pipelineLayout_,
                VkPipeline pipeline_
            );
            ~PipelineGroup();

            PipelineGroup(PipelineGroup const&) noexcept = delete;
            PipelineGroup(PipelineGroup&&) noexcept = delete;
            PipelineGroup& operator= (PipelineGroup const& rhs) noexcept = delete;
            PipelineGroup& operator= (PipelineGroup&& rhs) noexcept = delete;

            [[nodiscard]]
            bool IsValid() const noexcept;
        };

        struct SwapChainGroup
        {
            explicit SwapChainGroup(
                VkSwapchainKHR swapChain_,
                VkFormat swapChainFormat_,
                std::vector<VkImage> swapChainImages_,
                std::vector<std::shared_ptr<ImageViewGroup>> swapChainImageViews_
            );
            ~SwapChainGroup();

            SwapChainGroup(SwapChainGroup const&) noexcept = delete;
            SwapChainGroup(SwapChainGroup&&) noexcept = delete;
            SwapChainGroup& operator= (SwapChainGroup const& rhs) noexcept = delete;
            SwapChainGroup& operator= (SwapChainGroup&& rhs) noexcept = delete;

            VkSwapchainKHR const swapChain;
            VkFormat const swapChainFormat;
            std::vector<VkImage> swapChainImages;
            std::vector<std::shared_ptr<ImageViewGroup>> swapChainImageViews;
        };

        enum class CommandBufferType
        {
            Invalid,
            Graphic,
            Compute
        };

        struct CommandRecordState
        {
            uint32_t imageIndex = 0;
            uint32_t frameIndex = 0;
            bool isValid = false;

            CommandBufferType commandBufferType = CommandBufferType::Invalid;   // We could have used queue family index as well
            VkCommandBuffer commandBuffer = nullptr;
            PipelineGroup* pipeline = nullptr;
            RenderPass* renderPass = nullptr;
            VkSwapchainKHR swapChain{};
            std::vector<CommandBufferType> commandBufferHistory {};
        };

        struct ColorImageGroup
        {
            explicit ColorImageGroup(
                std::shared_ptr<ImageGroup> imageGroup_,
                std::shared_ptr<ImageViewGroup> imageView_,
                VkFormat imageFormat_
            );
            ~ColorImageGroup();

            ColorImageGroup(ColorImageGroup const&) noexcept = delete;
            ColorImageGroup(ColorImageGroup&&) noexcept = delete;
            ColorImageGroup& operator= (ColorImageGroup const& rhs) noexcept = delete;
            ColorImageGroup& operator= (ColorImageGroup&& rhs) noexcept = delete;

            std::shared_ptr<ImageGroup> const imageGroup;
            std::shared_ptr<ImageViewGroup> const imageView;
            VkFormat const imageFormat;
        };

        struct RenderPass
        {
            VkRenderPass const vkRenderPass;

            explicit RenderPass(VkRenderPass renderPass_);
            ~RenderPass();

            RenderPass(RenderPass const&) noexcept = delete;
            RenderPass(RenderPass&&) noexcept = delete;
            RenderPass& operator= (RenderPass const& rhs) noexcept = delete;
            RenderPass& operator= (RenderPass&& rhs) noexcept = delete;
        };

        struct FrameBuffer
        {
            explicit FrameBuffer(VkFramebuffer framebuffer_);
            ~FrameBuffer();

            const VkFramebuffer framebuffer;
        };

        struct GpuShader
        {

            explicit GpuShader(
                VkShaderModule const& shaderModule,
                VkShaderStageFlagBits stageFlags,
                std::string entryPointName
            );
            ~GpuShader();

            GpuShader(GpuShader const&) noexcept = delete;
            GpuShader(GpuShader&&) noexcept = delete;
            GpuShader& operator= (GpuShader const& rhs) noexcept = delete;
            GpuShader& operator= (GpuShader&& rhs) noexcept = delete;

            VkShaderModule const shaderModule;
            VkShaderStageFlagBits const stageFlags;
            std::string const entryPointName;

        };

        struct DescriptorPool
        {
            explicit DescriptorPool(VkDescriptorPool descriptorPool_);
            ~DescriptorPool();

            DescriptorPool(DescriptorPool const&) noexcept = delete;
            DescriptorPool(DescriptorPool&&) noexcept = delete;
            DescriptorPool& operator= (DescriptorPool const& rhs) noexcept = delete;
            DescriptorPool& operator= (DescriptorPool&& rhs) noexcept = delete;

            VkDescriptorPool descriptorPool{};
        };

        struct DescriptorSetGroup
        {
            std::vector<VkDescriptorSet> descriptorSets{};

            [[nodiscard]]
            bool IsValid() const noexcept
            {
                return descriptorSets.empty() == false;
            }
        };

        struct BufferAndMemory
        {
            const VkBuffer buffer;
            const VkDeviceMemory memory;
            VkDeviceSize const size;

            explicit BufferAndMemory(
                VkBuffer buffer_,
                VkDeviceMemory memory_,
                VkDeviceSize size_
            );
            ~BufferAndMemory();

            BufferAndMemory(BufferAndMemory const&) noexcept = delete;
            BufferAndMemory(BufferAndMemory&&) noexcept = delete;
            BufferAndMemory& operator= (BufferAndMemory const& rhs) noexcept = delete;
            BufferAndMemory& operator= (BufferAndMemory&& rhs) noexcept = delete;

        };

        struct BufferGroup
        {
            explicit BufferGroup(
                std::vector<std::shared_ptr<BufferAndMemory>> buffers_,
                size_t bufferSize_,
                VkMemoryPropertyFlags memoryPropertyFlags,
                VkBufferUsageFlags bufferUsageFlags
            );
            ~BufferGroup();

            BufferGroup(BufferGroup const&) noexcept = delete;
            BufferGroup(BufferGroup&&) noexcept = delete;
            BufferGroup& operator= (BufferGroup const& rhs) noexcept = delete;
            BufferGroup& operator= (BufferGroup&& rhs) noexcept = delete;

            std::vector<std::shared_ptr<BufferAndMemory>> const buffers;
            size_t const bufferSize;
            VkMemoryPropertyFlags memoryPropertyFlags;
            VkBufferUsageFlags bufferUsageFlags;
        };

        struct SamplerGroup
        {
            VkSampler const sampler;

            explicit SamplerGroup(VkSampler sampler_);
            ~SamplerGroup();

            SamplerGroup(SamplerGroup const&) noexcept = delete;
            SamplerGroup(SamplerGroup&&) noexcept = delete;
            SamplerGroup& operator= (SamplerGroup const& rhs) noexcept = delete;
            SamplerGroup& operator= (SamplerGroup&& rhs) noexcept = delete;

        };

        struct GpuTexture
        {
            explicit GpuTexture();

            explicit GpuTexture(
                std::shared_ptr<ImageGroup> imageGroup,
                std::shared_ptr<ImageViewGroup> imageView
            );
            ~GpuTexture();

            GpuTexture(GpuTexture const&) noexcept = delete;
            GpuTexture(GpuTexture&&) noexcept = delete;
            GpuTexture& operator= (GpuTexture const& rhs) noexcept = delete;
            GpuTexture& operator= (GpuTexture&& rhs) noexcept = delete;

            std::shared_ptr<ImageGroup> const imageGroup{};
            std::shared_ptr<ImageViewGroup> const imageView{};
        };

    };

    namespace RT = RenderTypes;

};
