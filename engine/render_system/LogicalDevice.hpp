#pragma once

#include "RenderBackend.hpp"
#include "BedrockSignal.hpp"

#include <vulkan/vulkan.h>
#include <string>

namespace MFA
{
    class LogicalDevice
    {
    public:
        
        struct InitParams
        {
            int windowWidth = 0;
            int windowHeight = 0;
            bool resizable = true;
            bool fullScreen = false;
            std::string applicationName {};
            // TODO: Maybe expose the sdl flags to support video and audio
        };

        [[nodiscard]]
        static std::unique_ptr<LogicalDevice> Instantiate(InitParams const& params);

        explicit LogicalDevice(InitParams const & params);
        
        ~LogicalDevice();

        void OnResizeEvent(SDL_Event * event);

        void Update();

        RT::CommandRecordState AcquireRecordState(VkSwapchainKHR swapChain);

        [[nodiscard]]
        bool IsValid() const noexcept;

        [[nodiscard]]
        std::string ApplicationName() const noexcept;

        [[nodiscard]]
        bool IsResizable() const noexcept;

        [[nodiscard]]
        bool IsWindowVisible() const noexcept;

        [[nodiscard]]
        int GetWindowWidth() const noexcept;

        [[nodiscard]]
        int GetWindowHeight() const noexcept;

        [[nodiscard]]
        bool IsFullScreen() const noexcept;

        [[nodiscard]]
        VkInstance GetVkInstance() const noexcept;

        [[nodiscard]]
        VkSurfaceKHR GetSurface() const noexcept;

        [[nodiscard]]
        VkPhysicalDevice GetPhysicalDevice() const noexcept;

        [[nodiscard]]
        VkPhysicalDeviceFeatures GetPhysicalDeviceFeatures() const noexcept;

        [[nodiscard]]
        VkSampleCountFlagBits GetMaxSampleCount() const noexcept;

        [[nodiscard]]
        VkPhysicalDeviceProperties GetPhysicalDeviceProperties() const noexcept;

        [[nodiscard]]
        VkSurfaceCapabilitiesKHR GetSurfaceCapabilities() const noexcept;

        [[nodiscard]]
        uint32_t GetSwapChainImageCount() const noexcept;

        [[nodiscard]]
        uint32_t GetMaxFramePerFlight() const noexcept;

        [[nodiscard]]
        uint32_t GetGraphicQueueFamily() const noexcept;

        [[nodiscard]]
        uint32_t GetComputeQueueFamily() const noexcept;

        [[nodiscard]]
        uint32_t GetPresentQueueFamily() const noexcept;

        [[nodiscard]]
        VkDevice GetVkDevice() const noexcept;

        [[nodiscard]]
        VkPhysicalDeviceMemoryProperties GetPhysicalMemoryProperties() const noexcept;

        [[nodiscard]]
        VkQueue GetGraphicQueue() const noexcept;

        [[nodiscard]]
        VkQueue GetComputeQueue() const noexcept;

        [[nodiscard]]
        VkQueue GetPresentQueue() const noexcept;

        [[nodiscard]]
        VkCommandPool GetGraphicCommandPool() const noexcept;

        [[nodiscard]]
        std::vector<VkCommandBuffer> const& GetGraphicCommandBuffer() const noexcept;

        [[nodiscard]]
        std::vector<VkSemaphore> const& GetGraphicSemaphores() const noexcept;

        [[nodiscard]]
        std::vector<VkFence> const& GetGraphicFences() const noexcept;

        [[nodiscard]]
        VkCommandPool GetComputeCommandPool() const noexcept;

        [[nodiscard]]
        std::vector<VkCommandBuffer> const& GetComputeCommandBuffer() const noexcept;

        [[nodiscard]]
        std::vector<VkSemaphore> const & GetComputeSemaphores() const noexcept;

        [[nodiscard]]
        std::vector<VkFence> const& GetComputeFences() const noexcept;

        [[nodiscard]]
        std::vector<VkSemaphore> const & GetPresentSemaphores() const noexcept;

        [[nodiscard]]
        VkFormat GetDepthFormat() const noexcept;

        [[nodiscard]]
        VkSurfaceFormatKHR GetSurfaceFormat() const noexcept;

        [[nodiscard]]
        VkSemaphore GetGraphicSemaphore(RT::CommandRecordState const& recordState) const noexcept;
        
        [[nodiscard]]
        VkSemaphore GetComputeSemaphore(RT::CommandRecordState const& recordState) const noexcept;

        [[nodiscard]]
        VkSemaphore GetPresentSemaphore(RT::CommandRecordState const& recordState) const noexcept;

        [[nodiscard]]
        VkFence GetGraphicFence(RT::CommandRecordState const& recordState) const;

    	[[nodiscard]]
        VkFence GetComputeFence(RT::CommandRecordState const& recordState) const;

        [[nodiscard]]
        VkCommandBuffer GetComputeCommandBuffer(RT::CommandRecordState const& recordState) const;

        [[nodiscard]]
        VkCommandBuffer GetGraphicCommandBuffer(RT::CommandRecordState const& recordState) const;

        void BeginCommandBuffer(
            RT::CommandRecordState & recordState,
            RT::CommandBufferType commandBufferType,
            VkCommandBufferBeginInfo const & beginInfo = {
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
                .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT
            }
        );

        void EndCommandBuffer(RT::CommandRecordState & recordState);

        void SubmitQueues(RT::CommandRecordState & recordState);

        void Present(RT::CommandRecordState const & recordState, VkSwapchainKHR swapChain);

    	void DeviceWaitIdle() const;

        [[nodiscard]]
        SDL_Window* GetWindow() const noexcept;

    private:

        void UpdateSurface();

    public:

        inline static LogicalDevice* Instance = nullptr;

        Signal<> ResizeEventSignal1{};
        Signal<> ResizeEventSignal2{};

        Signal<SDL_Event*> SDL_EventSignal{};

    private:

        bool _isValid = false;
        
        std::string _applicationName {};
        SDL_Window * _window = nullptr;
        bool _resizable = false;
        bool _windowResized = true;
        bool _windowVisible = true;

        int _windowWidth {};
        int _windowHeight {};
        bool _fullScreen {};

        VkInstance _vkInstance {};

        VkSurfaceKHR _surface {};

        VkPhysicalDevice _physicalDevice {};
        VkPhysicalDeviceFeatures _physicalDeviceFeatures{};
        VkSampleCountFlagBits _maxSampleCount{};
        VkPhysicalDeviceProperties _physicalDeviceProperties{};

        VkSurfaceCapabilitiesKHR _surfaceCapabilities {};
    
        uint32_t _swapChainImageCount {};
        uint32_t _maxFramePerFlight {};
        uint32_t _currentFrame{};

        uint32_t _graphicQueueFamily {};
        uint32_t _computeQueueFamily {};
        uint32_t _presentQueueFamily {};

        VkDevice _vkDevice {};
        VkPhysicalDeviceMemoryProperties _physicalMemoryProperties{};

        VkQueue _graphicQueue {};
        VkQueue _computeQueue {};
        VkQueue _presentQueue {};

        VkCommandPool _graphicCommandPool {};
        std::vector<VkCommandBuffer> _graphicCommandBuffer {};
        std::vector<VkSemaphore> _graphicSemaphores {};
        std::vector<VkFence> _graphicFences {};

        VkCommandPool _computeCommandPool {};
        std::vector<VkCommandBuffer> _computeCommandBuffer{};
        std::vector<VkSemaphore> _computeSemaphores {};
        std::vector<VkFence> _computeFences {};
    
        std::vector<VkSemaphore> _presentSemaphores {};

        VkFormat _depthFormat {};
        VkSurfaceFormatKHR _surfaceFormat{};

        VkDebugReportCallbackEXT _vkDebugReportCallbackExt {};
    };

};