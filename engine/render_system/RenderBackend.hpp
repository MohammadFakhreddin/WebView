#pragma once


#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <cmath>

#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_vulkan.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL.h>
#undef main


#include "BedrockPlatforms.hpp"
#include "RenderTypes.hpp"
#include "AssetShader.hpp"
#include "AssetTexture.hpp"

namespace MFA::RenderBackend
{
    [[nodiscard]]
    SDL_Window * CreateWindow(
        std::string const& windowName,
        int windowWidth, int windowHeight,
        int posX, int posY
    );

    void DestroyWindow(SDL_Window * window);

    [[nodiscard]]
    VkInstance CreateInstance(char const * applicationName, SDL_Window * window);

    void DestroyInstance(VkInstance instance);
    
    struct FindPhysicalDeviceResult
    {
        VkPhysicalDevice physicalDevice = nullptr;
        VkPhysicalDeviceFeatures physicalDeviceFeatures{};
        VkSampleCountFlagBits maxSampleCount{};
        VkPhysicalDeviceProperties physicalDeviceProperties{};
    };
    [[nodiscard]]
    FindPhysicalDeviceResult FindBestPhysicalDevice(VkInstance instance);

    [[nodiscard]]
    VkSurfaceKHR CreateWindowSurface(SDL_Window * window, VkInstance_T * instance);

    [[nodiscard]]
    VkSurfaceCapabilitiesKHR GetSurfaceCapabilities(
        VkPhysicalDevice physicalDevice,
        VkSurfaceKHR windowSurface
    );

    [[nodiscard]]
    uint32_t ComputeSwapChainImagesCount(VkSurfaceCapabilitiesKHR surfaceCapabilities);

    [[nodiscard]]
    bool CheckSwapChainSupport(VkPhysicalDevice physical_device);

    struct FindQueueFamilyResult
    {
        bool const isPresentQueueValid = false;
        uint32_t const presentQueueFamily = -1;

        bool const isGraphicQueueValid = false;
        uint32_t const graphicQueueFamily = -1;

        bool const isComputeQueueValid = false;
        uint32_t const computeQueueFamily = -1;
    };

    [[nodiscard]]
    FindQueueFamilyResult FindQueueFamilies(
        VkPhysicalDevice physicalDevice,
        VkSurfaceKHR windowSurface
    );


    struct CreateLogicalDeviceResult
    {
        VkDevice device{};
        VkPhysicalDeviceMemoryProperties physicalMemoryProperties{};
    };

    [[nodiscard]]
    CreateLogicalDeviceResult CreateLogicalDevice(
        VkPhysicalDevice physicalDevice,
        uint32_t graphicsQueueFamily,
        uint32_t presentQueueFamily,
        VkPhysicalDeviceFeatures const & enabledPhysicalDeviceFeatures
    );

    [[nodiscard]]
    VkQueue GetQueueByFamilyIndex(
        VkDevice device,
        uint32_t const queueFamilyIndex
    );

    [[nodiscard]]
    VkCommandPool CreateCommandPool(VkDevice device, uint32_t const queue_family_index);

    [[nodiscard]]
    std::vector<VkCommandBuffer> CreateCommandBuffers(
        VkDevice device,
        uint32_t const count,
        VkCommandPool commandPool
    );

    [[nodiscard]]
    std::vector<VkSemaphore> CreateSemaphores(VkDevice device, uint32_t count);

    [[nodiscard]]
    std::vector<VkFence> CreateFence(VkDevice device, uint32_t count);

    [[nodiscard]]
    VkFormat FindSupportedFormat(
        VkPhysicalDevice physical_device,
        uint8_t const candidates_count,
        VkFormat * candidates,
        VkImageTiling const tiling,
        VkFormatFeatureFlags const features
    );

    [[nodiscard]]
    VkFormat FindDepthFormat(VkPhysicalDevice physical_device);

    void DeviceWaitIdle(VkDevice device);

    VkDebugReportCallbackEXT CreateDebugCallback(
        VkInstance vkInstance,
        PFN_vkDebugReportCallbackEXT const & debugCallback
    );

    void DestroySemaphore(VkDevice device, std::vector<VkSemaphore> const & semaphores);

    void DestroyCommandBuffers(
        VkDevice device,
        VkCommandPool commandPool,
        uint32_t const commandBuffersCount,
        VkCommandBuffer * commandBuffers
    );

    void DestroyCommandPool(VkDevice device, VkCommandPool commandPool);

    void DestroyFence(VkDevice device, std::vector<VkFence> const & fences);

    void DestroyLogicalDevice(VkDevice logicalDevice);

    void DestroyWindowSurface(VkInstance instance, VkSurfaceKHR surface);

    void DestroyDebugReportCallback(
        VkInstance instance,
        VkDebugReportCallbackEXT const & reportCallbackExt
    );

    void GetScreenSize(int& outWidth, int& outHeight);

    std::shared_ptr<RT::ImageGroup> CreateImage(
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        uint32_t width,
        uint32_t height,
        uint32_t depth,
        uint8_t mipLevels,
        uint16_t sliceCount,
        VkFormat format,
        VkImageTiling tiling,
        VkImageUsageFlags usage,
        VkSampleCountFlagBits samplesCount,
        VkMemoryPropertyFlags properties,
        VkImageCreateFlags imageCreateFlags = 0,
        VkImageType imageType = VK_IMAGE_TYPE_2D
    );

    void DestroyImage(
        VkDevice device,
        RT::ImageGroup const& imageGroup
    );

    [[nodiscard]]
    std::shared_ptr<RT::ImageViewGroup> CreateImageView(
        VkDevice device,
        VkImage const& image,
        VkFormat format,
        VkImageAspectFlags aspectFlags,
        uint32_t mipmapCount,
        uint32_t layerCount,
        VkImageViewType viewType
    );

    void DestroyImageView(
        VkDevice device,
        RT::ImageViewGroup const& imageView
    );

    struct CreateDepthImageOptions
    {
        uint16_t layerCount = 1;
        VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;
        VkImageCreateFlags imageCreateFlags = 0;
        VkSampleCountFlagBits samplesCount = VK_SAMPLE_COUNT_1_BIT;
        VkImageType imageType = VK_IMAGE_TYPE_2D;
    };

    std::shared_ptr<RT::DepthImageGroup> CreateDepthImage(
        VkPhysicalDevice physicalDevice,
        VkDevice device,
        VkExtent2D imageExtent,
        VkFormat depthFormat,
        CreateDepthImageOptions const& options
    );

    VkPipelineLayout CreatePipelineLayout(
        VkDevice device,
        uint32_t setLayoutCount,
        const VkDescriptorSetLayout* pSetLayouts,
        uint32_t pushConstantRangeCount,
        const VkPushConstantRange* pPushConstantRanges
    );

    struct CreateGraphicPipelineOptions
    {
        VkFrontFace frontFace = VK_FRONT_FACE_CLOCKWISE;
        VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;                   // TODO We need no cull for certain objects
        VkPipelineDynamicStateCreateInfo* dynamicStateCreateInfo = nullptr;
        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        VkPipelineColorBlendAttachmentState colorBlendAttachments{};
        bool useStaticViewportAndScissor = false;           // Use of dynamic viewport and scissor is recommended
        VkPrimitiveTopology primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        VkSampleCountFlagBits rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;

        // Default params
        explicit CreateGraphicPipelineOptions();
    };

    std::shared_ptr<RT::PipelineGroup> CreateGraphicPipeline(
        VkDevice device,
        uint8_t shaderStagesCount,
        RT::GpuShader const** shaderStages,
        uint32_t vertexBindingDescriptionCount,
        VkVertexInputBindingDescription const* vertexBindingDescriptionData,
        uint32_t attributeDescriptionCount,
        VkVertexInputAttributeDescription* attributeDescriptionData,
        VkExtent2D swapChainExtent,
        VkRenderPass renderPass,
        VkPipelineLayout pipelineLayout,
        CreateGraphicPipelineOptions const& options
    );

    std::shared_ptr<RT::PipelineGroup> CreateComputePipeline(
        VkDevice device,
        RT::GpuShader const& shaderStage,
        VkPipelineLayout pipelineLayout
    );

    void DestroyPipeline(VkDevice device, RT::PipelineGroup& pipelineGroup);

    void DestroySwapChain(VkDevice device, RT::SwapChainGroup const& swapChainGroup);

    std::shared_ptr<RT::SwapChainGroup> CreateSwapChain(
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        VkSurfaceKHR windowSurface,
        VkSurfaceCapabilitiesKHR surfaceCapabilities,
        VkSurfaceFormatKHR surfaceFormat,
        VkSwapchainKHR oldSwapChain
    );

    VkSurfaceFormatKHR ChooseSurfaceFormat(
        VkPhysicalDevice physicalDevice,
        VkSurfaceKHR windowSurface,
        VkSurfaceCapabilitiesKHR surfaceCapabilities
    );

    struct CreateColorImageOptions
    {
        uint16_t layerCount = 1;
        VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;
        VkImageCreateFlags imageCreateFlags = 0;
        VkSampleCountFlagBits samplesCount = VK_SAMPLE_COUNT_1_BIT;
        VkImageType imageType = VK_IMAGE_TYPE_2D;
    };

    std::shared_ptr<RT::ColorImageGroup> CreateColorImage(
        VkPhysicalDevice physicalDevice,
        VkDevice device,
        VkExtent2D const& imageExtent,
        VkFormat const imageFormat,
        CreateColorImageOptions const& options
    );

    [[nodiscard]]
    VkRenderPass CreateRenderPass(
        VkDevice device,
        VkAttachmentDescription* attachments,
        uint32_t attachmentsCount,
        VkSubpassDescription* subPasses,
        uint32_t subPassesCount,
        VkSubpassDependency* dependencies,
        uint32_t dependenciesCount
    );

    void DestroyRenderPass(VkDevice device, VkRenderPass renderPass);

    void BeginRenderPass(
        VkCommandBuffer commandBuffer,
        VkRenderPass renderPass,
        VkFramebuffer frameBuffer,
        VkExtent2D const& extent2D,
        uint32_t clearValuesCount,
        VkClearValue const* clearValues
    );

    void EndRenderPass(VkCommandBuffer commandBuffer);

    VkFramebuffer CreateFrameBuffers(
        VkDevice device,
        VkRenderPass renderPass,
        VkImageView const* attachments,
        uint32_t attachmentsCount,
        VkExtent2D const swapChainExtent,
        uint32_t layersCount
    );
    
    void DestroyFrameBuffers(
        VkDevice device,
        uint32_t const frameBuffersCount,
        VkFramebuffer* frameBuffers
    );
    
    void DestroyFrameBuffer(VkDevice device, VkFramebuffer frameBuffer);

    void AssignViewportAndScissorToCommandBuffer(
        VkExtent2D const& extent2D,
        VkCommandBuffer commandBuffer
    );

	void SetScissor(VkCommandBuffer commandBuffer, VkRect2D const& scissor);

    void SetViewport(VkCommandBuffer commandBuffer, VkViewport const& viewport);

    VkResult AcquireNextImage(
        VkDevice device,
        VkSemaphore imageAvailabilitySemaphore,
        VkSwapchainKHR const& swapChain,
        uint32_t& outImageIndex
    );

    void ResetFences(VkDevice device, std::vector<VkFence> const & fences);

    void WaitForFence(VkDevice device, std::vector<VkFence> const & fences);

    void BeginCommandBuffer(
        VkCommandBuffer commandBuffer,
        VkCommandBufferBeginInfo const & beginInfo
    );

    void ExecuteCommandBuffer(
        VkCommandBuffer primaryCommandBuffer,
        uint32_t subCommandBuffersCount,
        const VkCommandBuffer * subCommandBuffers
    );

    void PipelineBarrier(
        VkCommandBuffer commandBuffer,
        VkPipelineStageFlags sourceStageMask,
        VkPipelineStageFlags destinationStateMask,
        uint32_t imageMemoryBarrierCount,
        VkImageMemoryBarrier const * imageMemoryBarriers
    );

    void PipelineBarrier(
        VkCommandBuffer commandBuffer,
        VkPipelineStageFlags sourceStageMask,
        VkPipelineStageFlags destinationStateMask,
        uint32_t barrierCount,
        VkBufferMemoryBarrier const * bufferMemoryBarrier
    );

    void EndCommandBuffer(VkCommandBuffer commandBuffer);

    void SubmitQueues(
        VkQueue queue,
        uint32_t submitCount,
        const VkSubmitInfo * submitInfos,
        VkFence fence
    );

    [[nodiscard]]
    VkCommandBuffer BeginSingleTimeCommand(VkDevice device, VkCommandPool const & commandPool);

    void EndAndSubmitSingleTimeCommand(
        VkDevice device,
        VkCommandPool const & commandPool,
        VkQueue const & queue,
        VkCommandBuffer const & commandBuffer
    );

    std::shared_ptr<RT::GpuShader> CreateShader(
        VkDevice device,
        std::shared_ptr<AS::Shader> const& cpuShader
    );

    void DestroyShader(VkDevice device, RT::GpuShader const& gpuShader);

    std::shared_ptr<RT::DescriptorSetLayoutGroup> CreateDescriptorSetLayout(
        VkDevice device,
        uint8_t const bindings_count,
        VkDescriptorSetLayoutBinding * bindings
    );

    void DestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout);

    // TODO: We need to ask for pool sizes instead
    std::shared_ptr<RT::DescriptorPool> CreateDescriptorPool(
        VkDevice device,
        uint32_t const maxSets
    );

    void DestroyDescriptorPool(
        VkDevice device,
        VkDescriptorPool pool
    );

    void BindPipeline(
        RT::CommandRecordState& recordState,
        RT::PipelineGroup& pipeline
    );

    enum class UpdateFrequency : uint32_t
    {
        PerPipeline = 0,
        PerGeometry = 1,
        PerInstance = 2
    };

    void AutoBindDescriptorSet(
        RT::CommandRecordState const& recordState,
        UpdateFrequency frequency,
        RT::DescriptorSetGroup const& descriptorGroup
    );

    void AutoBindDescriptorSet(
        RT::CommandRecordState const& recordState,
        UpdateFrequency frequency,
        VkDescriptorSet descriptorSet
    );

    void BindDescriptorSet(
        VkCommandBuffer commandBuffer,
        VkPipelineBindPoint bindPoint,
        VkPipelineLayout pipelineLayout,
        UpdateFrequency frequency,
        VkDescriptorSet descriptorSet
    );

    RT::DescriptorSetGroup CreateDescriptorSet(
        VkDevice device,
        VkDescriptorPool descriptorPool,
        VkDescriptorSetLayout descriptorSetLayout,
        uint32_t descriptorSetCount,
        uint8_t schemasCount = 0,
        VkWriteDescriptorSet* schemas = nullptr
    );

    void UpdateDescriptorSets(
        VkDevice device,
        uint8_t descriptorSetCount,
        VkDescriptorSet* descriptorSets,
        uint8_t schemasCount,
        VkWriteDescriptorSet* schemas
    );

    void UpdateDescriptorSets(
        VkDevice device,
        uint32_t descriptorWritesCount,
        VkWriteDescriptorSet* descriptorWrites
    );

    //  We should destroy it when it is not bind anymore
    void DestroyBuffer(
        VkDevice device,
        RT::BufferAndMemory const& bufferGroup
    );

    void BindVertexBuffer(
        RT::CommandRecordState const& recordState,
        RT::BufferAndMemory const& vertexBuffer,
        uint32_t firstBinding = 0,
        VkDeviceSize offset = 0
    );

    void BindIndexBuffer(
        RT::CommandRecordState const& recordState,
        RT::BufferAndMemory const& indexBuffer,
        VkDeviceSize offset,
        VkIndexType indexType
    );

    void DrawIndexed(
        RT::CommandRecordState const& recordState,
        uint32_t indicesCount,
        uint32_t instanceCount = 1,
        uint32_t firstIndex = 0,
        uint32_t vertexOffset = 0,
        uint32_t firstInstance = 0
    );

    std::shared_ptr<RT::BufferGroup> CreateBufferGroup(
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        VkDeviceSize bufferSize,
        uint32_t count,
        VkBufferUsageFlags bufferUsageFlagBits,
        VkMemoryPropertyFlags memoryPropertyFlags
    );

    std::shared_ptr<RT::BufferGroup> CreateLocalUniformBuffer(
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        size_t bufferSize,
        uint32_t count
    );

    std::shared_ptr<RT::BufferGroup> CreateHostVisibleUniformBuffer(
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        size_t bufferSize, 
        uint32_t count
    );

    std::shared_ptr<RT::BufferGroup> CreateLocalStorageBuffer(
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        size_t bufferSize,
        uint32_t count
    );

    std::shared_ptr<RT::BufferGroup> CreateHostVisibleStorageBuffer(
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        VkDeviceSize bufferSize,
        uint32_t count
    );
    
    std::shared_ptr<RT::BufferGroup> CreateStageBuffer(
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        VkDeviceSize bufferSize,
        uint32_t count
    );

    void UpdateHostVisibleBuffer(
        VkDevice device,
        RT::BufferAndMemory const& buffer,
        BaseBlob const& data
    );

    void UpdateLocalBuffer(
        VkCommandBuffer commandBuffer,
        RT::BufferAndMemory const& buffer,
        RT::BufferAndMemory const& stageBuffer
    );
    
    std::shared_ptr<RT::BufferAndMemory> CreateVertexBuffer(
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        VkCommandBuffer commandBuffer,
        RT::BufferAndMemory const& stageBuffer,
        BaseBlob const& verticesBlob
    );
    
    std::shared_ptr<RT::BufferAndMemory> CreateVertexBuffer(
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        VkDeviceSize bufferSize
    );
    
    std::shared_ptr<RT::BufferGroup> CreateVertexBufferGroup(
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        VkDeviceSize const bufferSize,
        int const bufferCount
    );

    std::shared_ptr<RT::BufferAndMemory> CreateIndexBuffer(
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        VkCommandBuffer commandBuffer,
        RT::BufferAndMemory const& stageBuffer,
        BaseBlob const& indicesBlob
    );
    
    std::shared_ptr<RT::BufferAndMemory> CreateIndexBuffer(
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        VkDeviceSize bufferSize
    );

    std::shared_ptr<RT::BufferAndMemory> CreateBuffer(
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties
    );
    
    void MapHostVisibleMemory(
        VkDevice device,
        VkDeviceMemory bufferMemory,
        size_t const offset,
        size_t const size,
        void** outBufferData
    );
    
    void UnMapHostVisibleMemory(VkDevice device, VkDeviceMemory bufferMemory);
    
    void CopyDataToHostVisibleBuffer(
        VkDevice device,
        VkDeviceMemory bufferMemory,
        BaseBlob const & dataBlob
    );

    void PushConstants(
        RT::CommandRecordState& recordState,
        VkPipelineLayout pipeline_layout,
        VkShaderStageFlags shader_stage,
        uint32_t offset,
        BaseBlob const & data
    );


    struct CreateSamplerParams
    {
        VkFilter minFilter = VK_FILTER_LINEAR;
        VkFilter magFilter = VK_FILTER_LINEAR;
        VkSamplerAddressMode addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        VkSamplerAddressMode addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        VkSamplerAddressMode addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        bool anisotropyEnabled = true;
        float maxAnisotropy = 16.0f;
        VkBorderColor borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
        bool unnormalizedCoordinates = false;
        bool compareEnable = true;                                          // It is disabled for mac.
        VkCompareOp compareOp = VK_COMPARE_OP_ALWAYS;
        float minLod = 0;  // Level of detail
        float maxLod = 1;
        VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        float mipLodBias = 0.0f;
    };

    [[nodiscard]]
    std::shared_ptr<RT::SamplerGroup> CreateSampler(
        VkDevice device,
        CreateSamplerParams const& params
    );

    void DestroySampler(VkDevice device, RT::SamplerGroup const& sampler);

    using CreateTextureResult = std::tuple<std::shared_ptr<RT::GpuTexture>, std::shared_ptr<RT::BufferAndMemory>>;
    [[nodiscard]]
    CreateTextureResult CreateTexture(
        AS::Texture const& cpuTexture,
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        VkCommandBuffer commandBuffer
    );

    void DestroyTexture(VkDevice device, RT::GpuTexture& gpuTexture);

};

namespace MFA
{
    namespace RB = RenderBackend;
};

