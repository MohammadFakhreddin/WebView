#include "RenderBackend.hpp"

#include "BedrockLog.hpp"
#include "BedrockAssert.hpp"
#include "BedrockString.hpp"

#include <vector>
#include <set>
#include <cstdio>

namespace MFA::RenderBackend
{

    constexpr char EngineName[256] = "MFA";
    constexpr int EngineVersion = 1;
    constexpr char const * ValidationLayer = "VK_LAYER_KHRONOS_validation";

    //-------------------------------------------------------------------------------------------------

    static void VK_Check(VkResult const result);

    //-------------------------------------------------------------------------------------------------

	static void SDL_Check(SDL_bool const result);

    //-------------------------------------------------------------------------------------------------

	static void SDL_CheckForError();

    //-------------------------------------------------------------------------------------------------

	static std::set<std::string> QuerySupportedLayers();

    //-------------------------------------------------------------------------------------------------

	static std::vector<char const *> FilterSupportedLayers(std::vector<char const *> const & layers);

    //-------------------------------------------------------------------------------------------------

	static std::set<std::string> QuerySupportedInstanceExtension();

    //-------------------------------------------------------------------------------------------------

	static std::vector<char const *> FilterSupportedInstanceExtensions(std::vector<char const *> const & extensions);

    //-------------------------------------------------------------------------------------------------

	static std::set<std::string> QuerySupportedDeviceExtensions(VkPhysicalDevice const & physicalDevice);

    //-------------------------------------------------------------------------------------------------

	static std::vector<char const *> FilterSupportedDeviceExtensions(
        VkPhysicalDevice const & physicalDevice,
        std::vector<char const *> const & extensions
    );

    //-------------------------------------------------------------------------------------------------

	static VkExtent2D ChooseSwapChainExtent(
        VkSurfaceCapabilitiesKHR const& surfaceCapabilities,
        int const screenWidth,
        int const screenHeight
    );

    //-------------------------------------------------------------------------------------------------

	static VkPresentModeKHR ChoosePresentMode(
        uint8_t const presentModesCount,
        VkPresentModeKHR const* present_modes
    );

    //-------------------------------------------------------------------------------------------------

	static void CopyBuffer(
        VkCommandBuffer commandBuffer,
        VkBuffer sourceBuffer,
        VkBuffer destinationBuffer,
        VkDeviceSize const size
    );


    //-------------------------------------------------------------------------------------------------

	static void TransferImageLayout(
        VkDevice device,
        VkCommandBuffer commandBuffer,
        VkImage image,
        VkImageLayout oldLayout,
        VkImageLayout newLayout,
        uint32_t levelCount,
        uint32_t layerCount
    );

    //-------------------------------------------------------------------------------------------------

	static void CopyBufferToImage(
        VkDevice device,
        VkCommandBuffer commandBuffer,
        VkBuffer buffer,
        VkImage image,
        AS::Texture const& cpuTexture
    );

    //-------------------------------------------------------------------------------------------------

    static VkFormat ConvertCpuTextureFormatToGpu(AS::Texture::Format const cpuFormat);

    //-------------------------------------------------------------------------------------------------

    static void VK_Check(VkResult const result)
    {
        if (result != VK_SUCCESS)
        {
            // TODO: Display error code enum
            char buffer[100] {};
            auto const length = sprintf(
                buffer,
                "Vulkan command failed with code: %i",
                static_cast<int>(result)
            );
            auto const message = std::string(buffer, length);
            MFA_LOG_ERROR("%s", message.c_str());
        }
    }

    //-------------------------------------------------------------------------------------------------

    static void SDL_Check(SDL_bool const result)
    {
        if (result != SDL_TRUE)
        {
            MFA_CRASH("SDL command failed");
        }
    }

    //-------------------------------------------------------------------------------------------------

    static void SDL_CheckForError()
    {
        std::string error = SDL_GetError();
        if (error.empty() == false) {
            MFA_LOG_ERROR("SDL Error: %s", error.c_str());
        }
    }

    //-------------------------------------------------------------------------------------------------

    static std::set<std::string> QuerySupportedLayers() {
        static std::set<std::string> supportedLayers{};
        static bool layersAreSet = false;

        if (layersAreSet == false)
        {
            uint32_t count;
            vkEnumerateInstanceLayerProperties(&count, nullptr); //get number of extensions
            
            std::vector<VkLayerProperties> layers(count);
            vkEnumerateInstanceLayerProperties(&count, layers.data()); //populate buffer
            
            for (auto const & layer : layers) {
                supportedLayers.insert(layer.layerName);
            }
            layersAreSet = true;
        }
        return supportedLayers;
    }

    //-------------------------------------------------------------------------------------------------

    static std::vector<char const *> FilterSupportedLayers(std::vector<char const *> const & layers) 
    {
        auto const supportedLayers = QuerySupportedLayers();
        std::vector<char const *> result {};
        for (auto const & layer : layers)
        {
            if (supportedLayers.contains(layer)) {
                result.emplace_back(layer);
            } else {
                MFA_LOG_WARN("Layer %s is not supported by this device.", layer);
            }
        }
        return result;
    }

    //-------------------------------------------------------------------------------------------------

    static std::set<std::string> QuerySupportedInstanceExtension() {
        static std::set<std::string> supportedExtensions{};
        static bool extensionsAreSet = false;

        if (extensionsAreSet == false)
        {
            uint32_t count;
            vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr); //get number of extensions

            std::vector<VkExtensionProperties> extensions(count);
            vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data()); //populate buffer

            for (auto const & extension : extensions)
            {
                supportedExtensions.insert(extension.extensionName);
            }
            extensionsAreSet = true;
        }

        return supportedExtensions;
    }

    //-------------------------------------------------------------------------------------------------

    static std::vector<char const *> FilterSupportedInstanceExtensions(std::vector<char const *> const & extensions) 
    {
        auto const supportedExtension = QuerySupportedInstanceExtension();
        std::vector<char const *> result {};
        for (auto const & extension : extensions)
        {
            if (supportedExtension.contains(extension)) {
                result.emplace_back(extension);
            } else {
                MFA_LOG_WARN("Extension %s is not supported by this device.", extension);
            }
        }
        return result;
    }

    //-------------------------------------------------------------------------------------------------

    static std::set<std::string> QuerySupportedDeviceExtensions(VkPhysicalDevice const & physicalDevice)
    {
        static std::set<std::string> supportedExtensions{};
        static bool extensionsAreSet = false;

        if (extensionsAreSet == false)
        {
            uint32_t count;
            vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, nullptr); //get number of extensions

            std::vector<VkExtensionProperties> extensions(count);
            vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &count, extensions.data()); //populate buffer

            for (auto const & extension : extensions)
            {
                supportedExtensions.insert(extension.extensionName);
            }
            extensionsAreSet = true;
        }

        return supportedExtensions;
    }

    //-------------------------------------------------------------------------------------------------

    static std::vector<char const *> FilterSupportedDeviceExtensions(
        VkPhysicalDevice const & physicalDevice,
        std::vector<char const *> const & extensions
    )
    {
        auto const supportedExtension = QuerySupportedDeviceExtensions(physicalDevice);
        std::vector<char const *> result{};
        for (auto const & extension : extensions)
        {
            if (supportedExtension.contains(extension))
            {
                result.emplace_back(extension);
            }
            else
            {
                MFA_LOG_WARN("Extension %s is not supported by this device.", extension);
            }
        }
        return result;
    }

    //-------------------------------------------------------------------------------------------------

    SDL_Window * CreateWindow(
        std::string const & windowName, 
        int const windowWidth, int const windowHeight,
        int const posX, int const posY
    )
    {
        //https://stackoverflow.com/questions/61603693/sdl2-does-not-see-joystick-but-os-does
        SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_AUDIO);
        auto * window = SDL_CreateWindow(
            windowName.c_str(),
            posX,
            posY,
            windowWidth, windowHeight,
            SDL_WINDOW_SHOWN /*| SDL_WINDOW_FULLSCREEN */ | SDL_WINDOW_VULKAN
        );
        SDL_CheckForError();
        return window;
    }

    //-------------------------------------------------------------------------------------------------

    void DestroyWindow(SDL_Window * window)
    {
        MFA_ASSERT(window != nullptr);
        SDL_DestroyWindow(window);
        SDL_CheckForError();
    }
    
    //-------------------------------------------------------------------------------------------------

    VkInstance CreateInstance(char const * applicationName, SDL_Window * window)
    {

        // Filling out application description:
        auto applicationInfo = VkApplicationInfo{
            // sType is mandatory
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            // pNext is mandatory
            .pNext = nullptr,
            // The name of our application
            .pApplicationName = applicationName,
            .applicationVersion = 1,    // TODO Ask this as parameter
            // The name of the engine (e.g: Game engine name)
            .pEngineName = EngineName,
            // The version of the engine
            .engineVersion = EngineVersion,
            // The version of Vulkan we're using for this application
            .apiVersion = VK_API_VERSION_1_1
        };
        std::vector<char const *> instanceExtensions{};

        MFA_ASSERT(window != nullptr);
        {// Filling sdl extensions
            unsigned int sdl_extenstion_count = 0;
            SDL_Check(SDL_Vulkan_GetInstanceExtensions(window, &sdl_extenstion_count, nullptr));
            instanceExtensions.resize(sdl_extenstion_count);
            SDL_Check(SDL_Vulkan_GetInstanceExtensions(
                window,
                &sdl_extenstion_count,
                instanceExtensions.data()
            ));
        }

#ifdef MFA_DEBUG
        instanceExtensions.emplace_back(VK_KHR_EXTERNAL_FENCE_CAPABILITIES_EXTENSION_NAME);
        instanceExtensions.emplace_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        instanceExtensions.emplace_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        instanceExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

        // Filtering instance extensions
        auto supportedExtensions = FilterSupportedInstanceExtensions(instanceExtensions);
#ifdef __PLATFORM_MAC__
        supportedExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
        uint32_t flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#else
        uint32_t flags = 0;
#endif

        std::vector<char const *> enabledLayer {};
#if defined(MFA_DEBUG)
        enabledLayer.emplace_back(ValidationLayer);
#endif

        auto supportedLayers = FilterSupportedLayers(enabledLayer);


        // Filling out instance description:
        auto const instanceInfo = VkInstanceCreateInfo{
            // sType is mandatory
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            // pNext is mandatory
            .pNext = nullptr,
            .flags = flags,
            // The application info structure is then passed through the instance
            .pApplicationInfo = &applicationInfo,
            .enabledLayerCount = static_cast<uint32_t>(supportedLayers.size()),
            .ppEnabledLayerNames = supportedLayers.data(),
            .enabledExtensionCount = static_cast<uint32_t>(supportedExtensions.size()),
            .ppEnabledExtensionNames = supportedExtensions.data()
        };
        VkInstance instance = nullptr;
        VK_Check(vkCreateInstance(&instanceInfo, nullptr, &instance));
        MFA_ASSERT(instance != nullptr);
        return instance;
    }

    //-------------------------------------------------------------------------------------------------

    void DestroyInstance(VkInstance instance)
    {
        MFA_ASSERT(instance != nullptr);
        vkDestroyInstance(instance, nullptr);
    }

    //-------------------------------------------------------------------------------------------------

    VkSampleCountFlagBits ComputeMaxUsableSampleCount(VkPhysicalDeviceProperties deviceProperties)
    {
        auto const counts = deviceProperties.limits.framebufferColorSampleCounts
            & deviceProperties.limits.framebufferDepthSampleCounts;

        if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
        if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
        if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
        if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
        if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
        if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

        return VK_SAMPLE_COUNT_1_BIT;
    }

    //-------------------------------------------------------------------------------------------------

    VkSampleCountFlagBits ComputeMaxUsableSampleCount(VkPhysicalDevice physicalDevice)
    {
        MFA_ASSERT(physicalDevice != VK_NULL_HANDLE);

        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

        return ComputeMaxUsableSampleCount(physicalDeviceProperties);
    }

    //-------------------------------------------------------------------------------------------------

    FindPhysicalDeviceResult FindBestPhysicalDevice(VkInstance instance)
    {
        FindPhysicalDeviceResult result {};

        uint32_t deviceCount = 0;
        //Getting number of physical devices
        VK_Check(vkEnumeratePhysicalDevices(
            instance,
            &deviceCount,
            nullptr
        ));
        MFA_ASSERT(deviceCount > 0);

        std::vector<VkPhysicalDevice> devices(deviceCount);
        const auto phyDevResult = vkEnumeratePhysicalDevices(
            instance,
            &deviceCount,
            devices.data()
        );
        //TODO Search about incomplete
        MFA_ASSERT(phyDevResult == VK_SUCCESS || phyDevResult == VK_INCOMPLETE);

        for (uint32_t i = 0; i < deviceCount; ++i)
        {
            auto device = devices[i];

            VkPhysicalDeviceProperties properties{};

            vkGetPhysicalDeviceProperties(device, &properties);

            VkPhysicalDeviceFeatures features{};

            vkGetPhysicalDeviceFeatures(device, &features);
            
            
            /*MFA_LOG_INFO(
                "Device number %d\nName: %s\nApi version: %d.%d.%d"
                , i
                , properties.deviceName
                , VK_VERSION_MAJOR(properties.apiVersion)
                , VK_VERSION_MINOR(properties.apiVersion)
                , VK_VERSION_PATCH(properties.apiVersion)
            );*/

            auto deviceName = std::string(
                properties.deviceName, 
                strlen(properties.deviceName)
            );
            
            deviceName = String::ToLowerCase(deviceName.c_str());
            
            bool isNvidia = deviceName.find("nvidia") != std::string::npos;
            bool isAmd = deviceName.find("amd") != std::string::npos;
            
            if (i == 0 || isNvidia == true || isAmd == true)
            {
                result.physicalDevice = device;
                result.physicalDeviceFeatures = features;
                result.physicalDeviceProperties = properties;
                if (isNvidia == true || isAmd == true)
                {
                    // We found an ideal device
                    break;
                }
            }
        }

        result.maxSampleCount = ComputeMaxUsableSampleCount(result.physicalDeviceProperties);
        
        MFA_LOG_INFO("Selected device name: %s", result.physicalDeviceProperties.deviceName);

        return result;

    }

    //-------------------------------------------------------------------------------------------------

    VkSurfaceKHR CreateWindowSurface(SDL_Window * window, VkInstance_T * instance)
    {
        VkSurfaceKHR ret{};
        SDL_Check(SDL_Vulkan_CreateSurface(
            window,
            instance,
            &ret
        ));
        return ret;
    }

    //-------------------------------------------------------------------------------------------------

    VkSurfaceCapabilitiesKHR GetSurfaceCapabilities(
        VkPhysicalDevice physicalDevice,
        VkSurfaceKHR windowSurface
    )
    {
        MFA_ASSERT(physicalDevice != VK_NULL_HANDLE);
        MFA_ASSERT(windowSurface != VK_NULL_HANDLE);
        VkSurfaceCapabilitiesKHR surfaceCapabilities;
        VK_Check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, windowSurface, &surfaceCapabilities));
        return surfaceCapabilities;
    }

    //-------------------------------------------------------------------------------------------------

    uint32_t ComputeSwapChainImagesCount(VkSurfaceCapabilitiesKHR surfaceCapabilities)
    {
        uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
        if (surfaceCapabilities.maxImageCount != 0 && imageCount > surfaceCapabilities.maxImageCount)
        {
            imageCount = surfaceCapabilities.maxImageCount;
        }
        return imageCount;
    }

    //-------------------------------------------------------------------------------------------------

    bool CheckSwapChainSupport(VkPhysicalDevice physical_device)
    {
            bool ret = false;
        uint32_t extension_count = 0;
        VK_Check(vkEnumerateDeviceExtensionProperties(
            physical_device,
            nullptr,
            &extension_count,
            nullptr
        ));
        std::vector<VkExtensionProperties> device_extensions(extension_count);
        vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, device_extensions.data());
        for (const auto & extension : device_extensions)
        {
            if (strcmp(extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
            {
                MFA_LOG_INFO("Physical device supports swap chains");
                ret = true;
                break;
            }
        }
        return ret;
    }

    //-------------------------------------------------------------------------------------------------

    FindQueueFamilyResult FindQueueFamilies(
        VkPhysicalDevice physicalDevice,
        VkSurfaceKHR windowSurface
    )
    {

        bool isPresentQueueSet = false;
        uint32_t presentQueueFamily = -1;

        bool isGraphicQueueSet = false;
        uint32_t graphicQueueFamily = -1;

        bool isComputeQueueSet = false;
        uint32_t computeQueueFamily = -1;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
        if (queueFamilyCount == 0)
        {
            MFA_CRASH("physical device has no queue families!");
        }
        // Find queue family with graphics support
        // Note: is a transfer queue necessary to copy vertices to the gpu or can a graphics queue handle that?
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(
            physicalDevice,
            &queueFamilyCount,
            queueFamilies.data()
        );

        MFA_LOG_INFO("physical device has %d queue families.", queueFamilyCount);

        for (uint32_t queueIndex = 0; queueIndex < queueFamilyCount; queueIndex++)
        {
            if (isPresentQueueSet == false)
            {
                VkBool32 presentIsSupported = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, queueIndex, windowSurface, &presentIsSupported);
                if (presentIsSupported) {
                    presentQueueFamily = queueIndex;
                    isPresentQueueSet = true;
                }
            }

            auto & queueFamily = queueFamilies[queueIndex];
            if (queueFamily.queueCount > 0)
            {
                if (isGraphicQueueSet == false && (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT))
                {
                    graphicQueueFamily = queueIndex;
                    isGraphicQueueSet = true;
                }
                if (isComputeQueueSet == false && (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT))
                {
                    computeQueueFamily = queueIndex;
                    isComputeQueueSet = true;
                }
            }

            if (isPresentQueueSet && isGraphicQueueSet && isComputeQueueSet)
            {
                break;
            }
        }

        MFA_REQUIRE(isPresentQueueSet);
        MFA_REQUIRE(isGraphicQueueSet);
        MFA_REQUIRE(isComputeQueueSet);

        return FindQueueFamilyResult {
            .isPresentQueueValid = isPresentQueueSet,
            .presentQueueFamily = presentQueueFamily,

            .isGraphicQueueValid = isGraphicQueueSet,
            .graphicQueueFamily = graphicQueueFamily,

            .isComputeQueueValid = isComputeQueueSet,
            .computeQueueFamily = computeQueueFamily
        };
    }

    //-------------------------------------------------------------------------------------------------

    CreateLogicalDeviceResult CreateLogicalDevice(
        VkPhysicalDevice physicalDevice,
        uint32_t const graphicsQueueFamily,
        uint32_t const presentQueueFamily,
        VkPhysicalDeviceFeatures const & enabledPhysicalDeviceFeatures
    )
    {
        CreateLogicalDeviceResult logicalDevice{};

        MFA_ASSERT(physicalDevice != nullptr);

        // Create one graphics queue and optionally a separate presentation queue
        float const queuePriority = 1.0f;

        VkDeviceQueueCreateInfo queueCreateInfo[2] = {};

        queueCreateInfo[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo[0].queueFamilyIndex = graphicsQueueFamily;
        queueCreateInfo[0].queueCount = 1;
        queueCreateInfo[0].pQueuePriorities = &queuePriority;

        queueCreateInfo[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo[1].queueFamilyIndex = presentQueueFamily;
        queueCreateInfo[1].queueCount = 1;
        queueCreateInfo[1].pQueuePriorities = &queuePriority;

        // Create logical device from physical device
        // Note: there are separate instance and device extensions!
        VkDeviceCreateInfo deviceCreateInfo = {};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pQueueCreateInfos = queueCreateInfo;

        if (graphicsQueueFamily == presentQueueFamily)
        {
            deviceCreateInfo.queueCreateInfoCount = 1;
        }
        else
        {
            deviceCreateInfo.queueCreateInfoCount = 2;
        }

        std::vector<char const *> DebugLayers {};
    #ifdef MFA_DEBUG
        DebugLayers.emplace_back(ValidationLayer);
    #endif

        std::vector<char const *> enabledExtensionNames{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    #if defined(__PLATFORM_MAC__)// TODO We should query instead
        enabledExtensionNames.emplace_back("VK_KHR_portability_subset");
    #elif defined(__ANDROID__)
        enabledExtensionNames.emplace_back("VK_KHR_storage_buffer_storage_class");
    #endif
        enabledExtensionNames.emplace_back(VK_KHR_STORAGE_BUFFER_STORAGE_CLASS_EXTENSION_NAME);
        enabledExtensionNames.emplace_back(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME);
        
        auto filteredExtensionNames = FilterSupportedDeviceExtensions(physicalDevice, enabledExtensionNames);
        
        deviceCreateInfo.ppEnabledExtensionNames = filteredExtensionNames.data();
        deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(filteredExtensionNames.size());
        // Necessary for shader (for some reason)
        deviceCreateInfo.pEnabledFeatures = &enabledPhysicalDeviceFeatures;
        deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(DebugLayers.size());
        deviceCreateInfo.ppEnabledLayerNames = DebugLayers.data();
   

        VK_Check(vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &logicalDevice.device));
        MFA_ASSERT(logicalDevice.device != nullptr);
        MFA_LOG_INFO("Logical device create was successful");
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &logicalDevice.physicalMemoryProperties);

        return logicalDevice;
    }

    //-------------------------------------------------------------------------------------------------

    VkQueue GetQueueByFamilyIndex(
        VkDevice device,
        uint32_t const queueFamilyIndex
    )
    {
        VkQueue graphic_queue = nullptr;
        vkGetDeviceQueue(
            device,
            queueFamilyIndex,
            0,
            &graphic_queue
        );
        return graphic_queue;
    };

    //-------------------------------------------------------------------------------------------------

    VkCommandPool CreateCommandPool(VkDevice device, uint32_t const queue_family_index)
    {
        MFA_ASSERT(device);
        // Create graphics command pool
        VkCommandPoolCreateInfo poolCreateInfo = {};
        poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolCreateInfo.queueFamilyIndex = queue_family_index;
        poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        VkCommandPool command_pool{};
        VK_Check(vkCreateCommandPool(device, &poolCreateInfo, nullptr, &command_pool));

        return command_pool;
    }

    //-------------------------------------------------------------------------------------------------

    std::vector<VkCommandBuffer> CreateCommandBuffers(
        VkDevice device,
        uint32_t const count,
        VkCommandPool commandPool
    )
    {
        std::vector<VkCommandBuffer> commandBuffers(count);

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = count;

        VK_Check(vkAllocateCommandBuffers(
            device,
            &allocInfo,
            commandBuffers.data()
        ));

        MFA_LOG_INFO("Allocated graphics command buffers.");

        // Command buffer data gets recorded each time
        return commandBuffers;
    }

    //-------------------------------------------------------------------------------------------------

    std::vector<VkSemaphore> CreateSemaphores(VkDevice device, uint32_t count)
    {
        VkSemaphoreCreateInfo const semaphoreInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
        };

        std::vector<VkSemaphore> semaphores (count);

        for (auto & semaphore : semaphores)
        {
            VK_Check(vkCreateSemaphore(
                device,
                &semaphoreInfo,
                nullptr,
                &semaphore
            ));
        }

        return semaphores;
    }

    //-------------------------------------------------------------------------------------------------

    std::vector<VkFence> CreateFence(VkDevice device, uint32_t count)
    {
        VkFenceCreateInfo const fenceInfo{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .flags = VK_FENCE_CREATE_SIGNALED_BIT
        };

        std::vector<VkFence> fences (count);

        for (auto & fence : fences)
        {
            VK_Check(vkCreateFence(
                device,
                &fenceInfo,
                nullptr,
                &fence
            ));
        }

        return fences;
    }

    //-------------------------------------------------------------------------------------------------

    VkFormat FindSupportedFormat(
        VkPhysicalDevice physical_device,
        uint8_t const candidates_count,
        VkFormat * candidates,
        VkImageTiling const tiling,
        VkFormatFeatureFlags const features
    )
    {
        for (uint8_t index = 0; index < candidates_count; index++)
        {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physical_device, candidates[index], &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
            {
                return candidates[index];
            }
            if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
            {
                return candidates[index];
            }
        }
        MFA_CRASH("Failed to find supported format!");
    }

    //-------------------------------------------------------------------------------------------------

    VkFormat FindDepthFormat(VkPhysicalDevice physical_device)
    {
        std::vector<VkFormat> candidate_formats{
            VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_D32_SFLOAT_S8_UINT,
            VK_FORMAT_D24_UNORM_S8_UINT
        };
        return FindSupportedFormat(
            physical_device,
            static_cast<uint8_t>(candidate_formats.size()),
            candidate_formats.data(),
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    //-------------------------------------------------------------------------------------------------

    void DeviceWaitIdle(VkDevice device)
    {
        MFA_ASSERT(device != nullptr);
        VK_Check(vkDeviceWaitIdle(device));
    }

    //-------------------------------------------------------------------------------------------------

    VkDebugReportCallbackEXT CreateDebugCallback(
        VkInstance vkInstance,
        PFN_vkDebugReportCallbackEXT const & debugCallback
    )
    {
        MFA_ASSERT(vkInstance != nullptr);
        VkDebugReportCallbackCreateInfoEXT debug_info = {};
        debug_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
        debug_info.pfnCallback = debugCallback;
        debug_info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT |
            VK_DEBUG_REPORT_WARNING_BIT_EXT |
            VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
        auto const debug_report_callback = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(
            vkInstance,
            "vkCreateDebugReportCallbackEXT"
        ));
        VkDebugReportCallbackEXT debug_report_callback_ext;
        VK_Check(debug_report_callback(
            vkInstance,
            &debug_info,
            nullptr,
            &debug_report_callback_ext
        ));
        return debug_report_callback_ext;
    }

    //-------------------------------------------------------------------------------------------------

    void DestroySemaphore(VkDevice device, std::vector<VkSemaphore> const & semaphores)
    {
        for (auto & semaphore : semaphores)
        {
            vkDestroySemaphore(device, semaphore, nullptr);
        }
    }

    //-------------------------------------------------------------------------------------------------

    void DestroyCommandBuffers(
        VkDevice device,
        VkCommandPool commandPool,
        uint32_t const commandBuffersCount,
        VkCommandBuffer * commandBuffers
    )
    {
        MFA_ASSERT(device != nullptr);
        MFA_ASSERT(commandPool != VK_NULL_HANDLE);
        MFA_ASSERT(commandBuffersCount > 0);
        MFA_ASSERT(commandBuffers != nullptr);
        vkFreeCommandBuffers(
            device,
            commandPool,
            commandBuffersCount,
            commandBuffers
        );
    }

    //-------------------------------------------------------------------------------------------------

    void DestroyCommandPool(VkDevice device, VkCommandPool commandPool)
    {
        MFA_ASSERT(device);
        vkDestroyCommandPool(device, commandPool, nullptr);
    }

    //-------------------------------------------------------------------------------------------------

    void DestroyFence(VkDevice device, std::vector<VkFence> const& fences)
    {
        for (auto& fence : fences)
        {
            vkDestroyFence(device, fence, nullptr);
        }
    }

    //-------------------------------------------------------------------------------------------------

    void DestroyLogicalDevice(VkDevice logicalDevice)
    {
        vkDestroyDevice(logicalDevice, nullptr);
    }

    //-------------------------------------------------------------------------------------------------

    void DestroyWindowSurface(VkInstance instance, VkSurfaceKHR surface)
    {
        MFA_ASSERT(instance != nullptr);
        MFA_ASSERT(surface != VK_NULL_HANDLE);
        vkDestroySurfaceKHR(instance, surface, nullptr);
    }

    //-------------------------------------------------------------------------------------------------

    void DestroyDebugReportCallback(
        VkInstance instance,
        VkDebugReportCallbackEXT const & reportCallbackExt
    )
    {
        MFA_ASSERT(instance != nullptr);
        auto const DestroyDebugReportCallback = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(
            instance,
            "vkDestroyDebugReportCallbackEXT"
        ));
        DestroyDebugReportCallback(instance, reportCallbackExt, nullptr);
    }

    //-------------------------------------------------------------------------------------------------

    void GetScreenSize(int& outWidth, int& outHeight)
    {
        SDL_DisplayMode DM;
        SDL_GetDesktopDisplayMode(0, &DM);

        auto const errorMessage = SDL_GetError();
        if (strlen(errorMessage) > 0)
        {
            MFA_LOG_WARN("Error: %s", errorMessage);
        }

    	outWidth = DM.w;
        outHeight = DM.h;
    }

    //-------------------------------------------------------------------------------------------------

	uint32_t FindMemoryType(
        VkPhysicalDevice* physicalDevice,
        uint32_t const typeFilter,
        VkMemoryPropertyFlags const propertyFlags
    )
    {
        MFA_ASSERT(physicalDevice != nullptr);
        VkPhysicalDeviceMemoryProperties memory_properties;
        vkGetPhysicalDeviceMemoryProperties(*physicalDevice, &memory_properties);

        for (uint32_t memory_type_index = 0; memory_type_index < memory_properties.memoryTypeCount; memory_type_index++)
        {
            if ((typeFilter & (1 << memory_type_index)) && (memory_properties.memoryTypes[memory_type_index].propertyFlags & propertyFlags) == propertyFlags)
            {
                return memory_type_index;
            }
        }

        MFA_CRASH("failed to find suitable memory type!");
    }

    //-------------------------------------------------------------------------------------------------

    std::shared_ptr<RT::ImageGroup> CreateImage(
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        uint32_t const width,
        uint32_t const height,
        uint32_t const depth,
        uint8_t const mipLevels,
        uint16_t const sliceCount,
        VkFormat const format,
        VkImageTiling const tiling,
        VkImageUsageFlags const usage,
        VkSampleCountFlagBits const samplesCount,
        VkMemoryPropertyFlags const properties,
        VkImageCreateFlags const imageCreateFlags,
        VkImageType const imageType
    )
    {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = imageType;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = depth;
        imageInfo.mipLevels = mipLevels;
        imageInfo.arrayLayers = sliceCount;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = samplesCount;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.flags = imageCreateFlags;

        VkImage image{};
        VK_Check(vkCreateImage(device, &imageInfo, nullptr, &image));

        VkMemoryRequirements memory_requirements;
        vkGetImageMemoryRequirements(device, image, &memory_requirements);

        VkMemoryAllocateInfo allocate_info{};
        allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocate_info.allocationSize = memory_requirements.size;
        allocate_info.memoryTypeIndex = FindMemoryType(
            &physicalDevice,
            memory_requirements.memoryTypeBits,
            properties
        );

        VkDeviceMemory memory{};
        VK_Check(vkAllocateMemory(device, &allocate_info, nullptr, &memory));
        VK_Check(vkBindImageMemory(device, image, memory, 0));

        return std::make_shared<RT::ImageGroup>(image, memory);
    }

    //-------------------------------------------------------------------------------------------------

	void DestroyImage(
        VkDevice device,
        RT::ImageGroup const& imageGroup
    )
    {
        vkDestroyImage(device, imageGroup.image, nullptr);
        vkFreeMemory(device, imageGroup.memory, nullptr);
    }

    //-------------------------------------------------------------------------------------------------

	std::shared_ptr<RT::ImageViewGroup> CreateImageView(
        VkDevice device, 
        VkImage const& image, 
		VkFormat format,
		VkImageAspectFlags aspectFlags, 
		uint32_t mipmapCount, 
		uint32_t layerCount, 
		VkImageViewType viewType
    )
    {
        MFA_ASSERT(device != nullptr);

        VkImageViewCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = image;
        createInfo.viewType = viewType;
        createInfo.format = format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = aspectFlags;
        // TODO Might need to ask these values from image for mipmaps (Check again later)
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = mipmapCount;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = layerCount;

        VkImageView imageView{};
        VK_Check(vkCreateImageView(device, &createInfo, nullptr, &imageView));

        return std::make_shared<RT::ImageViewGroup>(imageView);
    }

    //-------------------------------------------------------------------------------------------------

    void DestroyImageView(VkDevice device, RT::ImageViewGroup const& imageView)
    {
        vkDestroyImageView(device, imageView.imageView, nullptr);
    }

    //-------------------------------------------------------------------------------------------------

    std::shared_ptr<RT::DepthImageGroup> CreateDepthImage(
        VkPhysicalDevice physicalDevice,
        VkDevice device,
        VkExtent2D const imageExtent,
        VkFormat depthFormat,
        CreateDepthImageOptions const& options
    )
    {
        //RT::DepthImageGroup depthImageGroup{};
        auto imageGroup = CreateImage(
            device,
            physicalDevice,
            imageExtent.width,
            imageExtent.height,
            1,
            1,
            options.layerCount,
            depthFormat,
            VK_IMAGE_TILING_OPTIMAL,
            options.usageFlags,
            options.samplesCount,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            options.imageCreateFlags,
            options.imageType
        );
        MFA_ASSERT(imageGroup->image);
        MFA_ASSERT(imageGroup->memory);
        auto imageView = CreateImageView(
            device,
            imageGroup->image,
            depthFormat,
            VK_IMAGE_ASPECT_DEPTH_BIT,
            1,
            options.layerCount,
            options.viewType
        );
        MFA_ASSERT(imageView);
        return std::make_shared<RT::DepthImageGroup>(
            std::move(imageGroup),
            std::move(imageView),
            depthFormat
            );
    }

    //-------------------------------------------------------------------------------------------------

    CreateGraphicPipelineOptions::CreateGraphicPipelineOptions()
    {
	    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	    depthStencil.depthTestEnable = VK_TRUE;
	    depthStencil.depthWriteEnable = VK_TRUE;                // TODO Instead of using new buffer for occlusion I might be able to use this to disable writting to depth
	    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	    depthStencil.depthBoundsTestEnable = VK_FALSE;
	    depthStencil.stencilTestEnable = VK_FALSE;
	    // TODO Maybe we can ask whether blend is enable or not
	    colorBlendAttachments.blendEnable = VK_TRUE;
	    colorBlendAttachments.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	    colorBlendAttachments.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	    colorBlendAttachments.colorBlendOp = VK_BLEND_OP_ADD;
	    colorBlendAttachments.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	    colorBlendAttachments.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	    colorBlendAttachments.alphaBlendOp = VK_BLEND_OP_ADD;
	    colorBlendAttachments.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    }

    //-------------------------------------------------------------------------------------------------

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
    )
    {
        MFA_ASSERT(shaderStages);
        MFA_ASSERT(renderPass);
        MFA_ASSERT(pipelineLayout);
        // Set up shader stage info
        std::vector<VkPipelineShaderStageCreateInfo> shaderStagesCreateInfos(shaderStagesCount);
        for (uint8_t i = 0; i < shaderStagesCount; i++)
        {
            VkPipelineShaderStageCreateInfo create_info = {};
            create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            create_info.stage = shaderStages[i]->stageFlags;
            create_info.module = shaderStages[i]->shaderModule;
            create_info.pName = shaderStages[i]->entryPointName.c_str();
            shaderStagesCreateInfos[i] = create_info;
        }

        // Describe vertex input
        VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info = {};
        vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_state_create_info.vertexBindingDescriptionCount = vertexBindingDescriptionCount;
        vertex_input_state_create_info.pVertexBindingDescriptions = vertexBindingDescriptionData;
        vertex_input_state_create_info.vertexAttributeDescriptionCount = attributeDescriptionCount;
        vertex_input_state_create_info.pVertexAttributeDescriptions = attributeDescriptionData;

        // Describe input assembly
        VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info = {};
        input_assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly_create_info.topology = options.primitiveTopology;
        input_assembly_create_info.primitiveRestartEnable = VK_FALSE;

        // Note: scissor test is always enabled (although dynamic scissor is possible)
        // Number of viewports must match number of scissors
        VkPipelineViewportStateCreateInfo viewport_create_info = {};
        viewport_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_create_info.viewportCount = 1;
        viewport_create_info.scissorCount = 1;
        // Do not move these objects into if conditions otherwise it will be removed for release mode
        // thus you may face strange validation error about min depth is not set
        VkViewport viewport{};
        VkRect2D scissor{};

        if (true == options.useStaticViewportAndScissor)
        {
            // Describe viewport and scissor
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(swapChainExtent.width);
            viewport.height = static_cast<float>(swapChainExtent.height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;

            scissor.offset.x = 0;
            scissor.offset.y = 0;
            scissor.extent.width = swapChainExtent.width;
            scissor.extent.height = swapChainExtent.height;

            viewport_create_info.pViewports = &viewport;
            viewport_create_info.pScissors = &scissor;
        }

        // Describe rasterization
        // Note: depth bias and using polygon modes other than fill require changes to logical device creation (device features)
        VkPipelineRasterizationStateCreateInfo rasterization_create_info = {};
        rasterization_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterization_create_info.polygonMode = options.polygonMode;
        rasterization_create_info.cullMode = options.cullMode;
        rasterization_create_info.frontFace = options.frontFace;
        rasterization_create_info.depthBiasEnable = VK_FALSE;
        rasterization_create_info.lineWidth = 1.0f;
        rasterization_create_info.depthClampEnable = VK_FALSE;
        rasterization_create_info.rasterizerDiscardEnable = VK_FALSE;

        // Describe multi-sampling
        // Note: using multi-sampling also requires turning on device features
        VkPipelineMultisampleStateCreateInfo multi_sample_state_create_info = {};
        multi_sample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multi_sample_state_create_info.rasterizationSamples = options.rasterizationSamples;
        multi_sample_state_create_info.sampleShadingEnable = VK_TRUE;
        multi_sample_state_create_info.minSampleShading = 0.5f; // It can be in range of 0 to 1. Min fraction for sample shading; closer to one is smooth
        multi_sample_state_create_info.alphaToCoverageEnable = VK_FALSE;
        multi_sample_state_create_info.alphaToOneEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo color_blend_create_info = {};
        color_blend_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blend_create_info.logicOpEnable = VK_FALSE;
        color_blend_create_info.logicOp = VK_LOGIC_OP_COPY;
        color_blend_create_info.attachmentCount = 1;
        color_blend_create_info.pAttachments = &options.colorBlendAttachments;
        color_blend_create_info.blendConstants[0] = 0.0f;
        color_blend_create_info.blendConstants[1] = 0.0f;
        color_blend_create_info.blendConstants[2] = 0.0f;
        color_blend_create_info.blendConstants[3] = 0.0f;

        VkPipelineDynamicStateCreateInfo* dynamicStateCreateInfoRef = nullptr;
        VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
        std::vector<VkDynamicState> dynamicStates{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        if (options.useStaticViewportAndScissor == false)
        {
            if (options.dynamicStateCreateInfo == nullptr)
            {
                dynamicStateCreateInfo = VkPipelineDynamicStateCreateInfo{};
                dynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
                dynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
                dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();
                dynamicStateCreateInfoRef = &dynamicStateCreateInfo;
            }
            else
            {
                dynamicStateCreateInfoRef = options.dynamicStateCreateInfo;
            }
        }

        // Create the graphics pipeline
        VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStagesCreateInfos.size());
        pipelineCreateInfo.pStages = shaderStagesCreateInfos.data();
        pipelineCreateInfo.pVertexInputState = &vertex_input_state_create_info;
        pipelineCreateInfo.pInputAssemblyState = &input_assembly_create_info;
        pipelineCreateInfo.pViewportState = &viewport_create_info;
        pipelineCreateInfo.pRasterizationState = &rasterization_create_info;
        pipelineCreateInfo.pMultisampleState = &multi_sample_state_create_info;
        pipelineCreateInfo.pColorBlendState = &color_blend_create_info;
        pipelineCreateInfo.layout = pipelineLayout;
        pipelineCreateInfo.renderPass = renderPass;
        pipelineCreateInfo.subpass = 0;
        pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineCreateInfo.basePipelineIndex = -1;
        pipelineCreateInfo.pDepthStencilState = &options.depthStencil;
        pipelineCreateInfo.pDynamicState = dynamicStateCreateInfoRef;

        VkPipeline pipeline{};
        VK_Check(vkCreateGraphicsPipelines(
            device,
            VK_NULL_HANDLE,
            1,
            &pipelineCreateInfo,
            nullptr,
            &pipeline
        ));

        auto pipelineGroup = std::make_shared<RT::PipelineGroup>(
            pipelineLayout,
            pipeline
        );
        
        return pipelineGroup;
    }

	//-------------------------------------------------------------------------------------------------

    VkPipelineLayout CreatePipelineLayout(
        VkDevice device,
        uint32_t setLayoutCount,
        const VkDescriptorSetLayout* pSetLayouts,
        uint32_t pushConstantRangeCount,
        const VkPushConstantRange* pPushConstantRanges
    )
    {
        VkPipelineLayoutCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        createInfo.setLayoutCount = setLayoutCount;
        createInfo.pSetLayouts = pSetLayouts; // Array of descriptor set layout, Order matter when more than 1
        createInfo.pushConstantRangeCount = pushConstantRangeCount;
        createInfo.pPushConstantRanges = pPushConstantRanges;

        VkPipelineLayout pipelineLayout{};
        VK_Check(vkCreatePipelineLayout(device, &createInfo, nullptr, &pipelineLayout));

        return pipelineLayout;
    }

    //-------------------------------------------------------------------------------------------------

    std::shared_ptr<RT::PipelineGroup> CreateComputePipeline(
        VkDevice device,
        RT::GpuShader const& shaderStage,
        VkPipelineLayout pipelineLayout
    )
    {
        VkPipelineShaderStageCreateInfo const shaderStageCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = shaderStage.stageFlags,
            .module = shaderStage.shaderModule,
            .pName = shaderStage.entryPointName.c_str()
        };

        VkComputePipelineCreateInfo const pipelineCreateInfo{
            .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
            .stage = shaderStageCreateInfo,
            .layout = pipelineLayout
        };

        VkPipeline pipeline{};
        VK_Check(vkCreateComputePipelines(
            device,
            VK_NULL_HANDLE,
            1,
            &pipelineCreateInfo,
            VK_NULL_HANDLE,
            &pipeline
        ));

        auto pipelineGroup = std::make_shared<RT::PipelineGroup>(
            pipelineLayout,
            pipeline
        );
        
        return pipelineGroup;
    }

    //-------------------------------------------------------------------------------------------------

	void DestroyPipeline(VkDevice device, RT::PipelineGroup& pipelineGroup)
    {
        MFA_ASSERT(device != nullptr);
        MFA_ASSERT(pipelineGroup.IsValid());
        vkDestroyPipeline(device, pipelineGroup.pipeline, nullptr);
        vkDestroyPipelineLayout(device, pipelineGroup.pipelineLayout, nullptr);
    }

	//-------------------------------------------------------------------------------------------------

    void DestroySwapChain(VkDevice device, RT::SwapChainGroup const& swapChainGroup)
    {
        MFA_ASSERT(device);
        MFA_ASSERT(swapChainGroup.swapChain);
        MFA_ASSERT(swapChainGroup.swapChainImageViews.empty() == false);
        MFA_ASSERT(swapChainGroup.swapChainImages.empty() == false);
        MFA_ASSERT(swapChainGroup.swapChainImages.size() == swapChainGroup.swapChainImageViews.size());
        vkDestroySwapchainKHR(device, swapChainGroup.swapChain, nullptr);
    }

    //-------------------------------------------------------------------------------------------------

    [[nodiscard]]
    static VkExtent2D ChooseSwapChainExtent(
        VkSurfaceCapabilitiesKHR const& surfaceCapabilities,
        int const screenWidth,
        int const screenHeight
    )
    {
        if (surfaceCapabilities.currentExtent.width <= 0)
        {
            VkExtent2D const swap_chain_extent = {
                std::min(
                    std::max(
                        static_cast<uint32_t>(screenWidth),
                        surfaceCapabilities.minImageExtent.width
                    ),
                    surfaceCapabilities.maxImageExtent.width
                ),
                std::min(
                    std::max(
                        static_cast<uint32_t>(screenHeight),
                        surfaceCapabilities.minImageExtent.height
                    ),
                    surfaceCapabilities.maxImageExtent.height
                )
            };
            return swap_chain_extent;
        }
        return surfaceCapabilities.currentExtent;
    }

    //-------------------------------------------------------------------------------------------------

    [[nodiscard]]
    static VkPresentModeKHR ChoosePresentMode(
        uint8_t const presentModesCount,
        VkPresentModeKHR const* present_modes
    )
    {
        for (uint8_t index = 0; index < presentModesCount; index++)
        {
            if (present_modes[index] == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                return present_modes[index];
            }
        }
        // If mailbox is unavailable, fall back to FIFO (guaranteed to be available)
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    //-------------------------------------------------------------------------------------------------

    std::shared_ptr<RT::SwapChainGroup> CreateSwapChain(
        VkDevice device, 
        VkPhysicalDevice physicalDevice,
	    VkSurfaceKHR windowSurface, 
        VkSurfaceCapabilitiesKHR surfaceCapabilities, 
        VkSurfaceFormatKHR surfaceFormat,
	    VkSwapchainKHR oldSwapChain
    )
    {
        // Find supported present modes
        uint32_t presentModeCount;
        if (vkGetPhysicalDeviceSurfacePresentModesKHR(
            physicalDevice,
            windowSurface,
            &presentModeCount,
            nullptr
        ) != VK_SUCCESS || presentModeCount == 0)
        {
            MFA_CRASH("Failed to get number of supported presentation modes");
        }

        std::vector<VkPresentModeKHR> present_modes(presentModeCount);
        VK_Check(vkGetPhysicalDeviceSurfacePresentModesKHR(
            physicalDevice,
            windowSurface,
            &presentModeCount,
            present_modes.data())
        );

        // Determine number of images for swap chain
        auto const imageCount = ComputeSwapChainImagesCount(surfaceCapabilities);

        // Select swap chain size
        auto const selectedSwapChainExtent = ChooseSwapChainExtent(
            surfaceCapabilities,
            surfaceCapabilities.currentExtent.width,
            surfaceCapabilities.currentExtent.height
        );

        // Determine transformation to use (preferring no transform)
        /*VkSurfaceTransformFlagBitsKHR surfaceTransform;*/
        //if (surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR) {
            //surfaceTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        //} else if (surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
            //surfaceTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        //} else {
        //surfaceTransform = surfaceCapabilities.currentTransform;
        //}
        VkSurfaceTransformFlagBitsKHR const surfaceTransform = surfaceCapabilities.currentTransform;

        MFA_ASSERT(present_modes.size() <= 255);
        // Choose presentation mode (preferring MAILBOX ~= triple buffering)
        auto const selected_present_mode = ChoosePresentMode(static_cast<uint8_t>(present_modes.size()), present_modes.data());

        // Finally, create the swap chain
        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = windowSurface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = selectedSwapChainExtent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
        createInfo.preTransform = surfaceTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = selected_present_mode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = oldSwapChain;

        VkSwapchainKHR swapChain{};
        VK_Check(vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain));

        MFA_LOG_INFO("Created swap chain");

        //swapChainGroup.swapChainFormat = selected_surface_format.format;
        VkFormat const swapChainFormat = surfaceFormat.format;

        // Store the images used by the swap chain
        // Note: these are the images that swap chain image indices refer to
        // Note: actual number of images may differ from requested number, since it's a lower bound
        uint32_t actualImageCount = 0;
        if (
            vkGetSwapchainImagesKHR(device, swapChain, &actualImageCount, nullptr) != VK_SUCCESS ||
            actualImageCount == 0
            )
        {
            MFA_CRASH("Failed to acquire number of swap chain images");
        }

        std::vector<VkImage> swapChainImages(actualImageCount);
        VK_Check(vkGetSwapchainImagesKHR(
            device,
            swapChain,
            &actualImageCount,
            swapChainImages.data()
        ));

        std::vector<std::shared_ptr<RT::ImageViewGroup>> swapChainImageViews(actualImageCount);
        for (uint32_t imageIndex = 0; imageIndex < actualImageCount; imageIndex++)
        {
            MFA_ASSERT(swapChainImages[imageIndex] != VK_NULL_HANDLE);
            swapChainImageViews[imageIndex] = CreateImageView(
                device,
                swapChainImages[imageIndex],
                swapChainFormat,
                VK_IMAGE_ASPECT_COLOR_BIT,
                1,
                1,
                VK_IMAGE_VIEW_TYPE_2D
            );
            MFA_ASSERT(swapChainImageViews[imageIndex] != VK_NULL_HANDLE);
        }

        MFA_LOG_INFO("Acquired swap chain images");
        return std::make_shared<RT::SwapChainGroup>(
            swapChain,
            swapChainFormat,
            swapChainImages,
            swapChainImageViews
            );
    }

    //-------------------------------------------------------------------------------------------------

    static VkSurfaceFormatKHR ChooseSurfaceFormat(
        uint8_t const availableFormatsCount,
        VkSurfaceFormatKHR const* availableFormats
    )
    {
        // We can either choose any format
        if (availableFormatsCount == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
        {
            return { VK_FORMAT_R8G8B8A8_UNORM, VK_COLORSPACE_SRGB_NONLINEAR_KHR };
        }

        // Or go with the standard format - if available
        for (uint8_t index = 0; index < availableFormatsCount; index++)
        {
            if (availableFormats[index].format == VK_FORMAT_R8G8B8A8_UNORM)
            {
                return availableFormats[index];
            }
        }

        // Or fall back to the first available one
        return availableFormats[0];
    }

    //-------------------------------------------------------------------------------------------------

    VkSurfaceFormatKHR ChooseSurfaceFormat(
        VkPhysicalDevice physicalDevice,
        VkSurfaceKHR windowSurface,
        VkSurfaceCapabilitiesKHR surfaceCapabilities
    )
    {
        // Find supported surface formats
        uint32_t formatCount;
        if (
            vkGetPhysicalDeviceSurfaceFormatsKHR(
                physicalDevice,
                windowSurface,
                &formatCount,
                nullptr
            ) != VK_SUCCESS ||
            formatCount == 0
            )
        {
            MFA_CRASH("Failed to get number of supported surface formats");
        }

        std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
        VK_Check(vkGetPhysicalDeviceSurfaceFormatsKHR(
            physicalDevice,
            windowSurface,
            &formatCount,
            surfaceFormats.data()
        ));

        // Find supported present modes
        uint32_t presentModeCount;
        if (vkGetPhysicalDeviceSurfacePresentModesKHR(
            physicalDevice,
            windowSurface,
            &presentModeCount,
            nullptr
        ) != VK_SUCCESS || presentModeCount == 0)
        {
            MFA_CRASH("Failed to get number of supported presentation modes");
        }

        std::vector<VkPresentModeKHR> present_modes(presentModeCount);
        VK_Check(vkGetPhysicalDeviceSurfacePresentModesKHR(
            physicalDevice,
            windowSurface,
            &presentModeCount,
            present_modes.data())
        );

        // Determine number of images for swap chain
        auto const imageCount = ComputeSwapChainImagesCount(surfaceCapabilities);

        MFA_LOG_INFO(
            "Surface extend width: %d, height: %d"
            , surfaceCapabilities.currentExtent.width
            , surfaceCapabilities.currentExtent.height
        );

        MFA_LOG_INFO("Using %d images for swap chain.", imageCount);
        MFA_ASSERT(surfaceFormats.size() <= 255);
        // Select a surface format
        auto const selectedSurfaceFormat = ChooseSurfaceFormat(
            static_cast<uint8_t>(surfaceFormats.size()),
            surfaceFormats.data()
        );

        return selectedSurfaceFormat;
    }

    //-------------------------------------------------------------------------------------------------

    std::shared_ptr<RT::ColorImageGroup> CreateColorImage(
        VkPhysicalDevice physicalDevice, 
        VkDevice device,
	    VkExtent2D const& imageExtent, 
		VkFormat const imageFormat, 
		CreateColorImageOptions const& options
    )
    {
	    auto imageGroup = CreateImage(
		    device,
		    physicalDevice,
		    imageExtent.width,
		    imageExtent.height,
		    1,
		    1,
		    options.layerCount,
		    imageFormat,
		    VK_IMAGE_TILING_OPTIMAL,
		    options.usageFlags,
		    options.samplesCount,
		    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		    options.imageCreateFlags,
		    options.imageType
	    );
	    MFA_ASSERT(imageGroup->image);
	    MFA_ASSERT(imageGroup->memory);
	    auto imageView = CreateImageView(
		    device,
		    imageGroup->image,
		    imageFormat,
		    VK_IMAGE_ASPECT_COLOR_BIT,
		    1,
		    options.layerCount,
		    options.viewType
	    );
	    MFA_ASSERT(imageView);
	    return std::make_shared<RT::ColorImageGroup>(
		    std::move(imageGroup),
		    std::move(imageView),
		    imageFormat
	    );
    }

    //-------------------------------------------------------------------------------------------------

    VkRenderPass CreateRenderPass(
        VkDevice device, 
        VkAttachmentDescription* attachments, 
        uint32_t attachmentsCount,
	    VkSubpassDescription* subPasses, 
        uint32_t subPassesCount, 
        VkSubpassDependency* dependencies,
	    uint32_t dependenciesCount
    )
    {
        VkRenderPassCreateInfo const createInfo{
		    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		    .attachmentCount = attachmentsCount,
		    .pAttachments = attachments,
		    .subpassCount = subPassesCount,
		    .pSubpasses = subPasses,
		    .dependencyCount = dependenciesCount,
		    .pDependencies = dependencies,
        };

        VkRenderPass renderPass{};
        VK_Check(vkCreateRenderPass(device, &createInfo, nullptr, &renderPass));

        return renderPass;
    }

    //-------------------------------------------------------------------------------------------------

    void DestroyRenderPass(VkDevice device, VkRenderPass renderPass)
    {
        MFA_ASSERT(device != nullptr);
        MFA_ASSERT(renderPass != VK_NULL_HANDLE);
        vkDestroyRenderPass(device, renderPass, nullptr);
    }

    //-------------------------------------------------------------------------------------------------

    void BeginRenderPass(
        VkCommandBuffer commandBuffer,
        VkRenderPass renderPass,
        VkFramebuffer frameBuffer,
        VkExtent2D const& extent2D,
        uint32_t const clearValuesCount,
        VkClearValue const* clearValues
    )
    {
        VkRenderPassBeginInfo renderPassBeginInfo = {};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = renderPass;
        renderPassBeginInfo.framebuffer = frameBuffer;
        renderPassBeginInfo.renderArea.offset.x = 0;
        renderPassBeginInfo.renderArea.offset.y = 0;
        renderPassBeginInfo.renderArea.extent.width = extent2D.width;
        renderPassBeginInfo.renderArea.extent.height = extent2D.height;
        renderPassBeginInfo.clearValueCount = clearValuesCount;
        renderPassBeginInfo.pClearValues = clearValues;

        vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    //-------------------------------------------------------------------------------------------------

    void EndRenderPass(VkCommandBuffer commandBuffer)
    {
        vkCmdEndRenderPass(commandBuffer);
    }

    VkFramebuffer CreateFrameBuffers(VkDevice device, VkRenderPass renderPass, VkImageView const* attachments,
	    uint32_t attachmentsCount, VkExtent2D const swapChainExtent, uint32_t layersCount)
    {
	    MFA_ASSERT(device != nullptr);
	    MFA_ASSERT(renderPass != VK_NULL_HANDLE);
	    MFA_ASSERT(attachments != nullptr);
	    MFA_ASSERT(attachmentsCount > 0);

	    VkFramebuffer frameBuffer{};

	    // Note: FrameBuffer is basically a specific choice of attachments for a render pass
	    // That means all attachments must have the same dimensions, interesting restriction

	    VkFramebufferCreateInfo createInfo = {};
	    createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	    createInfo.renderPass = renderPass;
	    createInfo.attachmentCount = attachmentsCount;
	    createInfo.pAttachments = attachments;
	    createInfo.width = swapChainExtent.width;
	    createInfo.height = swapChainExtent.height;
	    createInfo.layers = layersCount;

	    VK_Check(vkCreateFramebuffer(device, &createInfo, nullptr, &frameBuffer));

	    return frameBuffer;
    }

    //-------------------------------------------------------------------------------------------------

    void DestroyFrameBuffers(VkDevice device, uint32_t const frameBuffersCount, VkFramebuffer* frameBuffers)
    {
	    for (uint32_t index = 0; index < frameBuffersCount; index++)
	    {
		    vkDestroyFramebuffer(device, frameBuffers[index], nullptr);
	    }
    }

    //-------------------------------------------------------------------------------------------------

    void DestroyFrameBuffer(VkDevice device, VkFramebuffer frameBuffer)
    {
	    vkDestroyFramebuffer(device, frameBuffer, nullptr);
    }

	//-------------------------------------------------------------------------------------------------

    void AssignViewportAndScissorToCommandBuffer(VkExtent2D const& extent2D, VkCommandBuffer commandBuffer)
    {
	    MFA_ASSERT(commandBuffer != nullptr);

	    VkViewport const viewport{
		    .x = 0.0f,
		    .y = 0.0f,
		    .width = static_cast<float>(extent2D.width),
		    .height = static_cast<float>(extent2D.height),
		    .minDepth = 0.0f,
		    .maxDepth = 1.0f,
	    };

	    VkRect2D scissor{
		    .offset {
			    .x = 0,
			    .y = 0
		    },
		    .extent {
			    .width = extent2D.width,
			    .height = extent2D.height,
		    }
	    };

	    SetViewport(commandBuffer, viewport);
	    SetScissor(commandBuffer, scissor);
    }

    //-------------------------------------------------------------------------------------------------

    void SetScissor(VkCommandBuffer commandBuffer, VkRect2D const& scissor)
    {
	    MFA_ASSERT(commandBuffer != nullptr);
	    vkCmdSetScissor(
		    commandBuffer,
		    0,
		    1,
		    &scissor
	    );
    }

	//-------------------------------------------------------------------------------------------------

    void SetViewport(VkCommandBuffer commandBuffer, VkViewport const& viewport)
    {
	    MFA_ASSERT(commandBuffer != nullptr);
	    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    }
    
    //-------------------------------------------------------------------------------------------------

    VkResult AcquireNextImage(
        VkDevice device, 
        VkSemaphore imageAvailabilitySemaphore,
        VkSwapchainKHR const& swapChain,
        uint32_t& outImageIndex
    )
    {
	    return vkAcquireNextImageKHR(
		    device,
		    swapChain,
		    UINT64_MAX,
		    imageAvailabilitySemaphore,
		    VK_NULL_HANDLE,
		    &outImageIndex
	    );

    }

    //-------------------------------------------------------------------------------------------------

    void ResetFences(VkDevice device, std::vector<VkFence> const& fences)
    {
	    VK_Check(vkResetFences(device, static_cast<uint32_t>(fences.size()), fences.data()));
    }

    //-------------------------------------------------------------------------------------------------

    void WaitForFence(VkDevice device, std::vector<VkFence> const& fences)
    {
	    VK_Check(vkWaitForFences(
		    device,
            static_cast<uint32_t>(fences.size()),
            fences.data(),
		    VK_TRUE,
		    UINT64_MAX
	    ));
    }

    //-------------------------------------------------------------------------------------------------

    void BeginCommandBuffer(
        VkCommandBuffer commandBuffer,
        VkCommandBufferBeginInfo const & beginInfo
    )
    {
        VK_Check(vkBeginCommandBuffer(commandBuffer, &beginInfo));
    }

    //-------------------------------------------------------------------------------------------------

    void ExecuteCommandBuffer(
        VkCommandBuffer primaryCommandBuffer,
        uint32_t subCommandBuffersCount,
        const VkCommandBuffer * subCommandBuffers
    )
    {
        vkCmdExecuteCommands(
            primaryCommandBuffer,
            subCommandBuffersCount,
            subCommandBuffers
        );
    }

    //-------------------------------------------------------------------------------------------------

    void PipelineBarrier(
        VkCommandBuffer commandBuffer,
        VkPipelineStageFlags sourceStageMask,
        VkPipelineStageFlags destinationStateMask,
        uint32_t imageMemoryBarrierCount,
        VkImageMemoryBarrier const * imageMemoryBarriers
    )
    {
        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStageMask,
            destinationStateMask,
            0,
            0, nullptr,
            0, nullptr,
            imageMemoryBarrierCount, imageMemoryBarriers
        );
    }

    //-------------------------------------------------------------------------------------------------

    void PipelineBarrier(
        VkCommandBuffer commandBuffer,
        VkPipelineStageFlags sourceStageMask,
        VkPipelineStageFlags destinationStateMask,
        uint32_t barrierCount,
        VkBufferMemoryBarrier const * bufferMemoryBarrier
    )
    {
        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStageMask,
            destinationStateMask,
            0,
            0,
            nullptr,
            barrierCount,
            bufferMemoryBarrier,
            0,
            nullptr
        );
    }

    //-------------------------------------------------------------------------------------------------

    void EndCommandBuffer(VkCommandBuffer commandBuffer)
    {
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        {
            MFA_CRASH("Failed to record command buffer");
        }
    }


    //-------------------------------------------------------------------------------------------------

    void SubmitQueues(
        VkQueue queue,
        uint32_t submitCount,
        const VkSubmitInfo * submitInfos,
        VkFence fence
    )
    {
        VK_Check(vkQueueSubmit(queue, submitCount, submitInfos, fence));
    }

    //-------------------------------------------------------------------------------------------------

    [[nodiscard]]
    VkCommandBuffer BeginSingleTimeCommand(VkDevice device, VkCommandPool const & commandPool)
    {
        MFA_ASSERT(device != nullptr);

        VkCommandBufferAllocateInfo allocate_info{};
        allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocate_info.commandPool = commandPool;
        allocate_info.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device, &allocate_info, &commandBuffer);

        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &begin_info);

        return commandBuffer;
    }

    //-------------------------------------------------------------------------------------------------

    void EndAndSubmitSingleTimeCommand(
        VkDevice device,
        VkCommandPool const & commandPool,
        VkQueue const & queue,
        VkCommandBuffer const & commandBuffer
    )
    {
        VK_Check(vkEndCommandBuffer(commandBuffer));

        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &commandBuffer;
        VK_Check(vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE));
        VK_Check(vkQueueWaitIdle(queue));

        vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
    }

    //-------------------------------------------------------------------------------------------------

    std::shared_ptr<RT::GpuShader> CreateShader(VkDevice device, std::shared_ptr<AS::Shader> const& cpuShader)
    {
	    MFA_ASSERT(device != nullptr);
	    std::shared_ptr<RT::GpuShader> gpuShader = nullptr;
	    MFA_ASSERT(cpuShader != nullptr);

        auto const& shaderCode = cpuShader->compiledShaderCode;

	    VkShaderModule shaderModule{};

	    VkShaderModuleCreateInfo const createInfo{
		    .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		    .codeSize = shaderCode->Len(),
		    .pCode = shaderCode->As<uint32_t>(),
	    };
	    VK_Check(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule));
	    MFA_LOG_INFO("Creating shader module was successful");
	    gpuShader = std::make_shared<RT::GpuShader>(
		    shaderModule,
		    cpuShader->stage,
		    cpuShader->entryPoint
	    );
	    return gpuShader;
    }

    //-------------------------------------------------------------------------------------------------

    void DestroyShader(VkDevice device, RT::GpuShader const& gpuShader)
    {
	    MFA_ASSERT(device != nullptr);
	    vkDestroyShaderModule(device, gpuShader.shaderModule, nullptr);
    }

    //-------------------------------------------------------------------------------------------------
    
    std::shared_ptr<RT::DescriptorSetLayoutGroup> CreateDescriptorSetLayout(
        VkDevice device,
        uint8_t const bindings_count,
        VkDescriptorSetLayoutBinding * bindings
    )
    {
        VkDescriptorSetLayoutCreateInfo descriptorLayoutCreateInfo = {};
        descriptorLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorLayoutCreateInfo.bindingCount = static_cast<uint32_t>(bindings_count);
        descriptorLayoutCreateInfo.pBindings = bindings;
        VkDescriptorSetLayout descriptorSetLayout{};
        VK_Check(vkCreateDescriptorSetLayout(
            device,
            &descriptorLayoutCreateInfo,
            nullptr,
            &descriptorSetLayout
        ));
        return std::make_shared<RT::DescriptorSetLayoutGroup>(descriptorSetLayout);
    }
    
    //-------------------------------------------------------------------------------------------------
    
    void DestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout)
    {
        vkDestroyDescriptorSetLayout(
            device,
            descriptorSetLayout,
            nullptr
        );
    }

    //-------------------------------------------------------------------------------------------------

    std::shared_ptr<RT::DescriptorPool> CreateDescriptorPool(
        VkDevice device,
        uint32_t const maxSets
    )
    {
        std::vector<VkDescriptorPoolSize> poolSizes{};
        // TODO Check if both of these variables must have same value as maxSets // Maybe I have to ask each of separately
        {// Uniform buffers
            VkDescriptorPoolSize poolSize;
            poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            poolSize.descriptorCount = maxSets;
            poolSizes.emplace_back(poolSize);
        }
        {// Combined image sampler
            VkDescriptorPoolSize poolSize;
            poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            poolSize.descriptorCount = maxSets;
            poolSizes.emplace_back(poolSize);
        }
        {// Sampled image
            VkDescriptorPoolSize poolSize;
            poolSize.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            poolSize.descriptorCount = maxSets;
            poolSizes.emplace_back(poolSize);
        }
        {// Sampler
            VkDescriptorPoolSize poolSize;
            poolSize.type = VK_DESCRIPTOR_TYPE_SAMPLER;
            poolSize.descriptorCount = maxSets;
            poolSizes.emplace_back(poolSize);
        }
        {// Storage buffer
            VkDescriptorPoolSize poolSize;
            poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            poolSize.descriptorCount = maxSets;
            poolSizes.emplace_back(poolSize);
        }
        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = maxSets;

        VkDescriptorPool descriptorPool{};

        VK_Check(vkCreateDescriptorPool(
            device,
            &poolInfo,
            nullptr,
            &descriptorPool
        ));

        return std::make_shared<RT::DescriptorPool>(descriptorPool);
    }

    //-------------------------------------------------------------------------------------------------

    void DestroyDescriptorPool(
        VkDevice device,
        VkDescriptorPool pool
    )
    {
        MFA_ASSERT(device != nullptr);
        MFA_ASSERT(pool != VK_NULL_HANDLE);
        vkDestroyDescriptorPool(device, pool, nullptr);
    }

    //-------------------------------------------------------------------------------------------------

    void BindPipeline(RT::CommandRecordState& recordState, RT::PipelineGroup& pipeline)
    {
        MFA_ASSERT(recordState.isValid);
        recordState.pipeline = &pipeline;

        VkPipelineBindPoint bindPoint{};

        switch (recordState.commandBufferType)
        {
        case RenderTypes::CommandBufferType::Compute:
            bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
            break;
        case RenderTypes::CommandBufferType::Graphic:
            bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            break;
        default:
            MFA_ASSERT(false);
        }

        // We can bind command buffer to multiple pipeline
        vkCmdBindPipeline(
            recordState.commandBuffer,
            bindPoint,
            pipeline.pipeline
        );
    }

    //-------------------------------------------------------------------------------------------------

    void AutoBindDescriptorSet(
        RT::CommandRecordState const& recordState, 
        UpdateFrequency frequency,
        RT::DescriptorSetGroup const& descriptorGroup
	)
    {
        AutoBindDescriptorSet(
            recordState,
            frequency,
            descriptorGroup.descriptorSets[recordState.frameIndex]
        );
    }

    //-------------------------------------------------------------------------------------------------

    void AutoBindDescriptorSet(
        RT::CommandRecordState const& recordState, 
        UpdateFrequency frequency,
	    VkDescriptorSet descriptorSet
    )
    {
        MFA_ASSERT(recordState.isValid);
        MFA_ASSERT(recordState.commandBuffer != nullptr);
        MFA_ASSERT(recordState.pipeline != nullptr);

        VkPipelineBindPoint bindPoint;
        switch (recordState.commandBufferType)
        {
        case RT::CommandBufferType::Compute:
            bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
            break;
        case RT::CommandBufferType::Graphic:
            bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            break;
        default:
            MFA_ASSERT(false);
            break;
        }

        BindDescriptorSet(
            recordState.commandBuffer,
            bindPoint,
            recordState.pipeline->pipelineLayout,
            frequency,
            descriptorSet
        );
    }

    //-------------------------------------------------------------------------------------------------

    void BindDescriptorSet(
        VkCommandBuffer commandBuffer,
        VkPipelineBindPoint bindPoint,
        VkPipelineLayout pipelineLayout,
        UpdateFrequency frequency,
        VkDescriptorSet descriptorSet
    )
    {
        vkCmdBindDescriptorSets(
            commandBuffer,
            bindPoint,
            pipelineLayout,
            static_cast<uint32_t>(frequency),
            1,
            &descriptorSet,
            0,
            nullptr
        );
    }

    //-------------------------------------------------------------------------------------------------

    RT::DescriptorSetGroup CreateDescriptorSet(
        VkDevice device, 
        VkDescriptorPool descriptorPool,
	    VkDescriptorSetLayout descriptorSetLayout, 
        uint32_t const descriptorSetCount, 
        uint8_t const schemasCount,
	    VkWriteDescriptorSet* schemas
    )
    {
	    MFA_ASSERT(device != nullptr);
	    MFA_ASSERT(descriptorPool != VK_NULL_HANDLE);
	    MFA_ASSERT(descriptorSetLayout != VK_NULL_HANDLE);

	    std::vector<VkDescriptorSetLayout> layouts(descriptorSetCount, descriptorSetLayout);
	    // There needs to be one descriptor set per binding point in the shader
	    VkDescriptorSetAllocateInfo allocInfo = {};
	    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	    allocInfo.descriptorPool = descriptorPool;
	    allocInfo.descriptorSetCount = descriptorSetCount;
	    allocInfo.pSetLayouts = layouts.data();

	    std::vector<VkDescriptorSet> descriptorSets{};
	    descriptorSets.resize(allocInfo.descriptorSetCount);
	    // Descriptor sets gets destroyed automatically when descriptor pool is destroyed
	    VK_Check(vkAllocateDescriptorSets(
		    device,
		    &allocInfo,
		    descriptorSets.data()
	    ));
	    MFA_LOG_INFO("Create descriptor set is successful");

	    if (schemasCount > 0 && schemas != nullptr)
	    {
		    MFA_ASSERT(descriptorSets.size() < 256);
		    UpdateDescriptorSets(
			    device,
			    static_cast<uint8_t>(descriptorSets.size()),
			    descriptorSets.data(),
			    schemasCount,
			    schemas
		    );
	    }

	    return RT::DescriptorSetGroup{ .descriptorSets = descriptorSets };
    }

    //-------------------------------------------------------------------------------------------------

    void UpdateDescriptorSets(
        VkDevice const device, 
        uint8_t const descriptorSetCount, 
		VkDescriptorSet* descriptorSets,
	    uint8_t const schemasCount, 
        VkWriteDescriptorSet* schemas
    )
    {
	    for (auto descriptor_set_index = 0; descriptor_set_index < descriptorSetCount; descriptor_set_index++)
	    {
		    std::vector<VkWriteDescriptorSet> descriptor_writes(schemasCount);
		    for (uint8_t schema_index = 0; schema_index < schemasCount; schema_index++)
		    {
			    descriptor_writes[schema_index] = schemas[schema_index];
			    descriptor_writes[schema_index].dstSet = descriptorSets[descriptor_set_index];
		    }

		    vkUpdateDescriptorSets(
			    device,
			    static_cast<uint32_t>(descriptor_writes.size()),
			    descriptor_writes.data(),
			    0,
			    nullptr
		    );
	    }
    }

    //-------------------------------------------------------------------------------------------------

    void UpdateDescriptorSets(VkDevice device, uint32_t descriptorWritesCount, VkWriteDescriptorSet* descriptorWrites)
    {
	    MFA_ASSERT(device != nullptr);
	    MFA_ASSERT(descriptorWritesCount > 0);
	    MFA_ASSERT(descriptorWrites != nullptr);

	    vkUpdateDescriptorSets(
		    device,
		    descriptorWritesCount,
		    descriptorWrites,
		    0,
		    nullptr
	    );
    }

    //-------------------------------------------------------------------------------------------------

    void DestroyBuffer(
        VkDevice device,
        RT::BufferAndMemory const& bufferGroup
    )
    {
	    MFA_ASSERT(device != nullptr);
        MFA_ASSERT(bufferGroup.memory != VK_NULL_HANDLE);
        MFA_ASSERT(bufferGroup.buffer != VK_NULL_HANDLE);
        vkFreeMemory(device, bufferGroup.memory, nullptr);
        vkDestroyBuffer(device, bufferGroup.buffer, nullptr);
    }

    //-------------------------------------------------------------------------------------------------

    void BindVertexBuffer(
        RT::CommandRecordState const& recordState,
        RT::BufferAndMemory const& vertexBuffer,
        uint32_t const firstBinding,
        VkDeviceSize const offset
    )
    {
        MFA_ASSERT(recordState.isValid);
        vkCmdBindVertexBuffers(
            recordState.commandBuffer,
            firstBinding,
            1,
            &vertexBuffer.buffer,
            &offset
        );
    }

    //-------------------------------------------------------------------------------------------------

    void BindIndexBuffer(
        RT::CommandRecordState const& recordState,
        RT::BufferAndMemory const& indexBuffer,
        VkDeviceSize const offset,
        VkIndexType const indexType
    )
    {
        MFA_ASSERT(recordState.isValid);
        vkCmdBindIndexBuffer(
            recordState.commandBuffer,
            indexBuffer.buffer,
            offset,
            indexType
        );
    }

    //-------------------------------------------------------------------------------------------------

    void DrawIndexed(
        RT::CommandRecordState const& recordState,
        uint32_t const indicesCount,
        uint32_t const instanceCount,
        uint32_t const firstIndex,
        uint32_t const vertexOffset,
        uint32_t const firstInstance
    )
    {
        MFA_ASSERT(recordState.isValid);
        vkCmdDrawIndexed(
            recordState.commandBuffer,
            indicesCount,
            instanceCount,
            firstIndex,
            vertexOffset,
            firstInstance
        );
    }

    //-------------------------------------------------------------------------------------------------

    std::shared_ptr<RT::BufferGroup> CreateBufferGroup(
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        VkDeviceSize const bufferSize, 
        uint32_t const count,
	    VkBufferUsageFlags bufferUsageFlagBits, 
        VkMemoryPropertyFlags memoryPropertyFlags
    )
    {
	    std::vector<std::shared_ptr<RT::BufferAndMemory>> buffers(count);
	    for (auto& buffer : buffers)
	    {
		    buffer = CreateBuffer(
                device,
                physicalDevice,
			    bufferSize,
			    bufferUsageFlagBits,
			    memoryPropertyFlags
		    );
	    }

	    auto bufferGroup = std::make_shared<RT::BufferGroup>(
		    std::move(buffers),
		    bufferSize,
            bufferUsageFlagBits,
            memoryPropertyFlags
	    );

	    return bufferGroup;
    }

    //-------------------------------------------------------------------------------------------------

    std::shared_ptr<RT::BufferGroup> CreateLocalUniformBuffer(
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        size_t bufferSize,
        uint32_t count
    )
    {
	    return CreateBufferGroup(
            device,
            physicalDevice,
		    bufferSize,
		    count,
		    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	    );
    }

	//-------------------------------------------------------------------------------------------------

    std::shared_ptr<RT::BufferGroup> CreateHostVisibleUniformBuffer(
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        size_t bufferSize,
        uint32_t count
    )
    {
	    return CreateBufferGroup(
            device,
            physicalDevice,
		    bufferSize,
		    count,
		    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
	    );
    }

    //-------------------------------------------------------------------------------------------------

    std::shared_ptr<RT::BufferGroup> CreateLocalStorageBuffer(
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        size_t const bufferSize, 
        uint32_t const count
    )
    {
	    return CreateBufferGroup(
            device,
            physicalDevice,
		    bufferSize,
		    count,
		    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	    );
    }

    //-------------------------------------------------------------------------------------------------

    std::shared_ptr<RT::BufferGroup> CreateHostVisibleStorageBuffer(
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        VkDeviceSize const bufferSize,
        uint32_t const count
    )
    {
	    return CreateBufferGroup(
            device,
            physicalDevice,
		    bufferSize,
		    count,
		    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
	    );
    }

    //-------------------------------------------------------------------------------------------------

    std::shared_ptr<RT::BufferGroup> CreateStageBuffer(
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        VkDeviceSize const bufferSize, 
        uint32_t const count
    )
    {
	    return CreateBufferGroup(
            device,
            physicalDevice,
		    bufferSize,
		    count,
		    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	    );
    }

    //-------------------------------------------------------------------------------------------------

    static void CopyBuffer(
        VkCommandBuffer commandBuffer,
        VkBuffer sourceBuffer,
        VkBuffer destinationBuffer,
        VkDeviceSize const size
    )
    {
        VkBufferCopy const copyRegion{
            .size = size
        };

        vkCmdCopyBuffer(
            commandBuffer,
            sourceBuffer,
            destinationBuffer,
            1,
            &copyRegion
        );
    }

    //-------------------------------------------------------------------------------------------------

    void CopyDataToHostVisibleBuffer(
        VkDevice device,
        VkDeviceMemory bufferMemory,
        BaseBlob const & dataBlob
    )
    {
        MFA_ASSERT(dataBlob.IsValid() == true);
        void* tempBufferData = nullptr;
        MapHostVisibleMemory(
            device,
            bufferMemory,
            0,
            dataBlob.Len(),
            &tempBufferData
        );
        std::memcpy(tempBufferData, dataBlob.Ptr(), dataBlob.Len());
        UnMapHostVisibleMemory(device, bufferMemory);
    }

    //-------------------------------------------------------------------------------------------------

    void UpdateHostVisibleBuffer(
        VkDevice device,
        RT::BufferAndMemory const& buffer, 
        BaseBlob const& data
    )
    {
        //assert(buffer.size == data.Len());
        CopyDataToHostVisibleBuffer(device, buffer.memory, data);
    }

    //-------------------------------------------------------------------------------------------------

    void UpdateLocalBuffer(
        VkCommandBuffer commandBuffer, 
        RT::BufferAndMemory const& buffer,
	    RT::BufferAndMemory const& stageBuffer
    )
    {
        // MFA_ASSERT(buffer.size == stageBuffer.size);
        CopyBuffer(
            commandBuffer,
            stageBuffer.buffer,
            buffer.buffer,
            buffer.size
        );
    }

    //-------------------------------------------------------------------------------------------------

    std::shared_ptr<RT::BufferAndMemory> CreateVertexBuffer(
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        VkCommandBuffer commandBuffer,
        RT::BufferAndMemory const& stageBuffer,
        BaseBlob const& verticesBlob
    )
    {

	    VkDeviceSize const bufferSize = verticesBlob.Len();

	    auto vertexBuffer = CreateVertexBuffer(device, physicalDevice, bufferSize);

	    UpdateHostVisibleBuffer(device, stageBuffer, verticesBlob);

	    UpdateLocalBuffer(commandBuffer, *vertexBuffer, stageBuffer);

	    return vertexBuffer;
    }

    //-------------------------------------------------------------------------------------------------

    std::shared_ptr<RT::BufferAndMemory> CreateVertexBuffer(
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        VkDeviceSize const bufferSize
    )
    {
	    return RB::CreateBuffer(
		    device,
		    physicalDevice,
		    bufferSize,
		    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	    );
    }
    
	//-------------------------------------------------------------------------------------------------

    std::shared_ptr<RT::BufferGroup> CreateVertexBufferGroup(
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        VkDeviceSize const bufferSize,
        int const bufferCount
    )
    {
        return RB::CreateBufferGroup(
            device, 
            physicalDevice, 
            bufferSize,
            bufferCount,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );
    }

	//-------------------------------------------------------------------------------------------------

    std::shared_ptr<RT::BufferAndMemory> CreateIndexBuffer(
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        VkCommandBuffer commandBuffer,
	    RT::BufferAndMemory const& stageBuffer, 
        BaseBlob const& indicesBlob
    )
    {
	    auto const bufferSize = indicesBlob.Len();

	    auto indexBuffer = CreateIndexBuffer(
            device,
            physicalDevice,
            bufferSize
        );

	    UpdateHostVisibleBuffer(device, stageBuffer, indicesBlob);

	    UpdateLocalBuffer(commandBuffer, *indexBuffer, stageBuffer);

	    return indexBuffer;
    }

    //-------------------------------------------------------------------------------------------------

    std::shared_ptr<RT::BufferAndMemory> CreateIndexBuffer(
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        VkDeviceSize bufferSize
    )
    {
	    return CreateBuffer(
		    device,
		    physicalDevice,
		    bufferSize,
		    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	    );
    }

    //-------------------------------------------------------------------------------------------------

    std::shared_ptr<RT::BufferAndMemory> CreateBuffer(
        VkDevice device, 
        VkPhysicalDevice physicalDevice,
	    VkDeviceSize const size, 
        VkBufferUsageFlags const usage, 
        VkMemoryPropertyFlags const properties
    )
    {
	    MFA_ASSERT(device != nullptr);
	    MFA_ASSERT(physicalDevice != nullptr);

	    VkBufferCreateInfo buffer_info{};
	    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	    buffer_info.size = size;
	    buffer_info.usage = usage;
	    buffer_info.sharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE;

	    VkBuffer buffer{};
	    VK_Check(vkCreateBuffer(device, &buffer_info, nullptr, &buffer));
	    MFA_ASSERT(buffer != VK_NULL_HANDLE);
	    VkMemoryRequirements memory_requirements{};
	    vkGetBufferMemoryRequirements(device, buffer, &memory_requirements);

	    VkMemoryAllocateInfo memoryAllocationInfo{};
	    memoryAllocationInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	    memoryAllocationInfo.allocationSize = memory_requirements.size;
	    memoryAllocationInfo.memoryTypeIndex = FindMemoryType(
		    &physicalDevice,
		    memory_requirements.memoryTypeBits,
		    properties
	    );

	    VkDeviceMemory memory{};
	    VK_Check(vkAllocateMemory(device, &memoryAllocationInfo, nullptr, &memory));
	    VK_Check(vkBindBufferMemory(device, buffer, memory, 0));

	    return std::make_shared<RT::BufferAndMemory>(buffer, memory, size);
    }

	//-------------------------------------------------------------------------------------------------

    void MapHostVisibleMemory(
        VkDevice device, 
        VkDeviceMemory bufferMemory, 
        size_t const offset, 
        size_t const size,
	    void** outBufferData
    )
    {
	    MFA_ASSERT(*outBufferData == nullptr);
	    VK_Check(vkMapMemory(device, bufferMemory, offset, size, 0, outBufferData));
	    MFA_ASSERT(*outBufferData != nullptr);
    }

	//-------------------------------------------------------------------------------------------------

    void UnMapHostVisibleMemory(VkDevice device, VkDeviceMemory bufferMemory)
    {
	    vkUnmapMemory(device, bufferMemory);
    }

    //-------------------------------------------------------------------------------------------------

    void PushConstants(
        RT::CommandRecordState& recordState,
        VkPipelineLayout pipeline_layout,
        VkShaderStageFlags const shader_stage,
        uint32_t const offset,
        BaseBlob const & data
    )
    {
        vkCmdPushConstants(
            recordState.commandBuffer,
            pipeline_layout,
            shader_stage,
            offset,
            static_cast<uint32_t>(data.Len()),
            data.Ptr()
        );
    }

    //-------------------------------------------------------------------------------------------------

    std::shared_ptr<RT::SamplerGroup> CreateSampler(
        VkDevice device, 
        CreateSamplerParams const& params
    )
    {
        MFA_ASSERT(device != nullptr);
        VkSampler sampler{};

        VkSamplerCreateInfo sampler_info{};
        sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler_info.magFilter = params.magFilter;
        sampler_info.minFilter = params.minFilter;
        sampler_info.addressModeU = params.addressModeU;
        sampler_info.addressModeV = params.addressModeV;
        sampler_info.addressModeW = params.addressModeW;
        sampler_info.anisotropyEnable = params.anisotropyEnabled;
        sampler_info.maxAnisotropy = params.maxAnisotropy;
        sampler_info.borderColor = params.borderColor;
        sampler_info.unnormalizedCoordinates = params.unnormalizedCoordinates;
        /*
         *TODO Issue for mac:
        "Message code: 0\nMessage: Validation Error: [ VUID-VkDescriptorImageInfo-mutableComparisonSamplers-04450 ] Object 0: handle = 0x62200004c918, type = VK_OBJECT_TYPE_DEVICE; | MessageID = 0xf467460 | vkUpdateDescriptorSets() (portability error): sampler comparison not available. The Vulkan spec states: If the [VK_KHR_portability_subset] extension is enabled, and VkPhysicalDevicePortabilitySubsetFeaturesKHR::mutableComparisonSamplers is VK_FALSE, then sampler must have been created with VkSamplerCreateInfo::compareEnable set to VK_FALSE. (https://vulkan.lunarg.com/doc/view/1.2.182.0/mac/1.2-extensions/vkspec.html#VUID-VkDescriptorImageInfo-mutableComparisonSamplers-04450)\nLocation: 256275552\n"
         **/
#ifdef __PLATFORM_MAC__
        sampler_info.compareEnable = VK_FALSE;
#else
        sampler_info.compareEnable = params.compareEnable;
#endif
        sampler_info.compareOp = params.compareOp;
        sampler_info.minLod = params.minLod;
        sampler_info.maxLod = params.maxLod;
        sampler_info.mipmapMode = params.mipmapMode;
        sampler_info.mipLodBias = params.mipLodBias;

        VK_Check(vkCreateSampler(device, &sampler_info, nullptr, &sampler));

        return std::make_shared<RT::SamplerGroup>(sampler);
    }

    //-------------------------------------------------------------------------------------------------

    void DestroySampler(VkDevice device, RT::SamplerGroup const& sampler)
    {
        MFA_ASSERT(device != nullptr);
        MFA_ASSERT(sampler.sampler != VK_NULL_HANDLE);
        vkDestroySampler(device, sampler.sampler, nullptr);
    }

    //-------------------------------------------------------------------------------------------------

    void TransferImageLayout(
        VkDevice device,
        VkCommandBuffer commandBuffer,
        VkImage image,
        VkImageLayout const oldLayout,
        VkImageLayout const newLayout,
        uint32_t const levelCount,
        uint32_t const layerCount
    )
    {
        MFA_ASSERT(device != nullptr);
        MFA_ASSERT(commandBuffer != VK_NULL_HANDLE);
        MFA_ASSERT(image != VK_NULL_HANDLE);

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = levelCount;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = layerCount;

        VkPipelineStageFlags source_stage;
        VkPipelineStageFlags destination_stage;

        if (
            oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
            newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
            )
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (
            oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
            newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            )
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else
        {
            MFA_CRASH("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(
            commandBuffer,
            source_stage, destination_stage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
    }

    //-------------------------------------------------------------------------------------------------

    static void CopyBufferToImage(
        VkDevice device,
        VkCommandBuffer commandBuffer,
        VkBuffer buffer,
        VkImage image,
        AS::Texture const& cpuTexture
    )
    {
        MFA_ASSERT(device != nullptr);
        MFA_ASSERT(commandBuffer != VK_NULL_HANDLE);
        MFA_ASSERT(buffer != VK_NULL_HANDLE);
        MFA_ASSERT(image != VK_NULL_HANDLE);
        MFA_ASSERT(cpuTexture.isValid());
        MFA_ASSERT(
            cpuTexture.GetMipmap(cpuTexture.GetMipCount() - 1).offset +
            cpuTexture.GetMipmap(cpuTexture.GetMipCount() - 1).size == cpuTexture.GetBuffer()->Len()
        );

        auto const mipCount = cpuTexture.GetMipCount();
        auto const slices = cpuTexture.GetSlices();
        auto const regionCount = mipCount * slices;
        auto const regionsBlob = Memory::AllocSize(regionCount * sizeof(VkBufferImageCopy));
        auto* regionsArray = regionsBlob->As<VkBufferImageCopy>();
        for (uint8_t sliceIndex = 0; sliceIndex < slices; sliceIndex++)
        {
            for (uint8_t mipLevel = 0; mipLevel < mipCount; mipLevel++)
            {
                auto const& mipInfo = cpuTexture.GetMipmap(mipLevel);
                auto& region = regionsArray[mipLevel];
                region.imageExtent.width = mipInfo.dimension.width;
                region.imageExtent.height = mipInfo.dimension.height;
                region.imageExtent.depth = mipInfo.dimension.depth;
                region.imageOffset.x = 0;
                region.imageOffset.y = 0;
                region.imageOffset.z = 0;
                region.bufferOffset = static_cast<uint32_t>(cpuTexture.mipOffsetInBytes(mipLevel, sliceIndex));
                region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                region.imageSubresource.mipLevel = mipLevel;
                region.imageSubresource.baseArrayLayer = sliceIndex;
                region.imageSubresource.layerCount = 1;
                region.bufferRowLength = 0;
                region.bufferImageHeight = 0;
            }
        }

        vkCmdCopyBufferToImage(
            commandBuffer,
            buffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            regionCount,
            regionsArray
        );
    }

    //-------------------------------------------------------------------------------------------------

    static VkFormat ConvertCpuTextureFormatToGpu(AS::Texture::Format const cpuFormat)
    {
        using Format = AS::Texture::Format;
        switch (cpuFormat)
        {
        case Format::UNCOMPRESSED_UNORM_R8_SRGB:
            return VkFormat::VK_FORMAT_R8_SRGB;
        case Format::UNCOMPRESSED_UNORM_R8_LINEAR:
            return VkFormat::VK_FORMAT_R8_UNORM;
        case Format::UNCOMPRESSED_UNORM_R8G8_LINEAR:
            return VkFormat::VK_FORMAT_R8G8_UNORM;
        case Format::UNCOMPRESSED_UNORM_R8G8B8A8_SRGB:
            return VkFormat::VK_FORMAT_R8G8B8A8_SRGB;
        case Format::UNCOMPRESSED_UNORM_R8G8B8A8_LINEAR:
            return VkFormat::VK_FORMAT_R8G8B8A8_UNORM;
        case Format::BC4_SNorm_Linear_R:
            return VkFormat::VK_FORMAT_BC4_SNORM_BLOCK;
        case Format::BC4_UNorm_Linear_R:
            return VkFormat::VK_FORMAT_BC4_UNORM_BLOCK;
        case Format::BC5_SNorm_Linear_RG:
            return VkFormat::VK_FORMAT_BC5_SNORM_BLOCK;
        case Format::BC5_UNorm_Linear_RG:
            return VkFormat::VK_FORMAT_BC5_UNORM_BLOCK;
        case Format::BC6H_SFloat_Linear_RGB:
            return VkFormat::VK_FORMAT_BC6H_SFLOAT_BLOCK;
        case Format::BC6H_UFloat_Linear_RGB:
            return VkFormat::VK_FORMAT_BC6H_UFLOAT_BLOCK;
        case Format::BC7_UNorm_Linear_RGB:
        case Format::BC7_UNorm_Linear_RGBA:
            return VkFormat::VK_FORMAT_BC7_UNORM_BLOCK;
        case Format::BC7_UNorm_sRGB_RGB:
        case Format::BC7_UNorm_sRGB_RGBA:
            return VkFormat::VK_FORMAT_BC7_SRGB_BLOCK;
        default:
            MFA_LOG_WARN("Format not found");
        }
        return {};
    }

    //-------------------------------------------------------------------------------------------------

    CreateTextureResult CreateTexture(
        AS::Texture const& cpuTexture,
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        VkCommandBuffer commandBuffer
    )
    {
        MFA_ASSERT(device != nullptr);
        MFA_ASSERT(physicalDevice != nullptr);
        MFA_ASSERT(commandBuffer != VK_NULL_HANDLE);
        MFA_ASSERT(cpuTexture.isValid());

        if (cpuTexture.isValid())
        {
            auto const format = cpuTexture.GetFormat();
            auto const mipCount = cpuTexture.GetMipCount();
            auto const sliceCount = cpuTexture.GetSlices();
            auto const& largestMipmapInfo = cpuTexture.GetMipmap(0);
            auto const buffer = cpuTexture.GetBuffer();
            MFA_ASSERT(buffer != nullptr && buffer->IsValid() == true);
            // Create upload buffer
            auto const uploadBufferGroup = CreateBuffer(    // TODO: We can cache this buffer
                device,
                physicalDevice,
                buffer->Len(),
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            );

            // Map texture data to buffer
            CopyDataToHostVisibleBuffer(device, uploadBufferGroup->memory, *buffer);

            auto const vulkan_format = ConvertCpuTextureFormatToGpu(format);

            auto imageGroup = CreateImage(
                device,
                physicalDevice,
                largestMipmapInfo.dimension.width,
                largestMipmapInfo.dimension.height,
                static_cast<uint32_t>(largestMipmapInfo.dimension.depth),
                mipCount,
                sliceCount,
                vulkan_format,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_SAMPLE_COUNT_1_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
            );

            TransferImageLayout(
                device,
                commandBuffer,
                imageGroup->image,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                mipCount,
                sliceCount
            );

            CopyBufferToImage(
                device,
                commandBuffer,
                uploadBufferGroup->buffer,
                imageGroup->image,
                cpuTexture
            );

            TransferImageLayout(
                device,
                commandBuffer,
                imageGroup->image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                mipCount,
                sliceCount
            );

            auto imageView = CreateImageView(
                device,
                imageGroup->image,
                vulkan_format,
                VK_IMAGE_ASPECT_COLOR_BIT,
                mipCount,
                sliceCount,
                VK_IMAGE_VIEW_TYPE_2D
            );

            std::shared_ptr<RT::GpuTexture> gpuTexture = std::make_shared<RT::GpuTexture>(
                imageGroup,
                imageView
            );
            return { gpuTexture, uploadBufferGroup };
        }
        return {nullptr, nullptr};
    }

    //-------------------------------------------------------------------------------------------------

    void DestroyTexture(VkDevice device, RT::GpuTexture& gpuTexture)
    {
        MFA_ASSERT(device != nullptr);
        MFA_ASSERT(device != nullptr);
        DestroyImage(
            device,
            *gpuTexture.imageGroup
        );
        DestroyImageView(
            device,
            *gpuTexture.imageView
        );
    }

    //-------------------------------------------------------------------------------------------------

};
