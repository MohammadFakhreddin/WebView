#include "DisplayRenderPass.hpp"

#include "LogicalDevice.hpp"

namespace MFA
{

    //-------------------------------------------------------------------------------------------------

    DisplayRenderPass::DisplayRenderPass(
        std::shared_ptr<SwapChainRenderResource> swapChain,
        std::shared_ptr<DepthRenderResource> depth,
        std::shared_ptr<MSSAA_RenderResource> msaa
    )
        : RenderPass()
        , mSwapChain(std::move(swapChain))
        , mDepth(std::move(depth))
        , mMSAA(std::move(msaa))
    {
        CreatePresentToDrawBarrier();
        CreateRenderPass();
        CreateFrameBuffers();
    }

    //-------------------------------------------------------------------------------------------------

    DisplayRenderPass::~DisplayRenderPass()
    {
        // This cleanup is optional. It is not important unless when order matters
        mFrameBuffers.clear();
        mRenderPass.reset();
    }

    //-------------------------------------------------------------------------------------------------

    VkRenderPass DisplayRenderPass::GetVkRenderPass()
    {
        return mRenderPass->vkRenderPass;
    }

    //-------------------------------------------------------------------------------------------------

    void DisplayRenderPass::Begin(RT::CommandRecordState & recordState, glm::vec4 backgroundColor)
    {
        ClearDepthBufferIfNeeded(recordState);

        RenderPass::Begin(recordState);

        UsePresentToDrawBarrier(recordState);

        auto surfaceCapabilities = LogicalDevice::Instance->GetSurfaceCapabilities();
        auto const swapChainExtend = VkExtent2D{
            .width = surfaceCapabilities.currentExtent.width,
            .height = surfaceCapabilities.currentExtent.height
        };

        RB::AssignViewportAndScissorToCommandBuffer(swapChainExtend, recordState.commandBuffer);

        std::vector<VkClearValue> clearValues(3);
        clearValues[0].color = VkClearColorValue{ .float32 = {backgroundColor.x, backgroundColor.y, backgroundColor.z, backgroundColor.w } };
        clearValues[1].color = VkClearColorValue{ .float32 = {backgroundColor.x, backgroundColor.y, backgroundColor.z, backgroundColor.w } };
        clearValues[2].depthStencil = { .depth = 1.0f, .stencil = 0 };

        RB::BeginRenderPass(
            recordState.commandBuffer,
            mRenderPass->vkRenderPass,
            GetFrameBuffer(recordState),
            swapChainExtend,
            static_cast<uint32_t>(clearValues.size()),
            clearValues.data()
        );
    }

    //-------------------------------------------------------------------------------------------------

    void DisplayRenderPass::End(RT::CommandRecordState & recordState)
    {
        RenderPass::End(recordState);

        if (LogicalDevice::Instance->IsWindowVisible() == false)
        {
            return;
        }

        RB::EndRenderPass(recordState.commandBuffer);

        auto const presentQueueFamily = LogicalDevice::Instance->GetPresentQueueFamily();
        auto const graphicQueueFamily = LogicalDevice::Instance->GetGraphicQueueFamily();

        // If present and graphics queue families differ, then another barrier is required
        if (presentQueueFamily != graphicQueueFamily)
        {
            // TODO Check that WTF is this ?
            VkImageSubresourceRange const subResourceRange{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            };

            VkImageMemoryBarrier const drawToPresentBarrier {
                .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
                .oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                .srcQueueFamilyIndex = graphicQueueFamily,
                .dstQueueFamilyIndex = presentQueueFamily,
                .image = mSwapChain->GetSwapChainImage(recordState),
                .subresourceRange = subResourceRange
            };

            RB::PipelineBarrier(
                recordState.commandBuffer,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                1,
                &drawToPresentBarrier
            );
        }
    }

    //-------------------------------------------------------------------------------------------------

    void DisplayRenderPass::NotifyDepthImageLayoutIsSet()
    {
        mIsDepthImageUndefined = false;
    }

    //-------------------------------------------------------------------------------------------------

    // Static approach for initial layout of depth buffers
    /*void DisplayRenderPass::UseDepthImageLayoutAsUndefined(bool const setDepthImageLayoutAsUndefined)
    {
        if (mIsDepthImageInitialLayoutUndefined == setDepthImageLayoutAsUndefined)
        {
            return;
        }
        mIsDepthImageInitialLayoutUndefined = setDepthImageLayoutAsUndefined;

        RF::DeviceWaitIdle();

        auto surfaceCapabilities = RF::GetSurfaceCapabilities();
        auto const swapChainExtend = VkExtent2D{
            .width = surfaceCapabilities.currentExtent.width,
            .height = surfaceCapabilities.currentExtent.height
        };

        RF::DestroyRenderPass(mVkDisplayRenderPass);

        createDisplayRenderPass();

        RF::DestroyFrameBuffers(
            static_cast<uint32_t>(mDisplayFrameBuffers.size()),
            mDisplayFrameBuffers.data()
        );

        createDisplayFrameBuffers(swapChainExtend);
    }*/



    //-------------------------------------------------------------------------------------------------

    VkFramebuffer DisplayRenderPass::GetFrameBuffer(RT::CommandRecordState const & recordState) const
    {
        return mFrameBuffers[recordState.imageIndex]->framebuffer;
    }

    //-------------------------------------------------------------------------------------------------

    VkFramebuffer DisplayRenderPass::GetFrameBuffer(uint32_t const imageIndex) const
    {
        return mFrameBuffers[imageIndex]->framebuffer;
    }

    //-------------------------------------------------------------------------------------------------

    void DisplayRenderPass::CreateFrameBuffers()
    {
        auto surfaceCapabilities = LogicalDevice::Instance->GetSurfaceCapabilities();
        auto const swapChainExtent = VkExtent2D{
            .width = surfaceCapabilities.currentExtent.width,
            .height = surfaceCapabilities.currentExtent.height
        };

        mFrameBuffers.clear();
        mFrameBuffers.resize(LogicalDevice::Instance->GetSwapChainImageCount());
        for (int i = 0; i < static_cast<int>(mFrameBuffers.size()); ++i)
        {
            std::vector<VkImageView> const attachments{
                mMSAA->GetImageGroup(i).imageView->imageView,
                mSwapChain->GetSwapChainImages().swapChainImageViews[i]->imageView,
                mDepth->GetDepthImage(i).imageView->imageView
            };
            mFrameBuffers[i] = std::make_shared<RT::FrameBuffer>(RB::CreateFrameBuffers(
                LogicalDevice::Instance->GetVkDevice(),
                mRenderPass->vkRenderPass,
                attachments.data(),
                static_cast<uint32_t>(attachments.size()),
                swapChainExtent,
                1
            ));
        }
    }

    //-------------------------------------------------------------------------------------------------

    void DisplayRenderPass::CreateRenderPass()
    {
        auto surfaceFormat = LogicalDevice::Instance->GetSurfaceFormat();
        auto depthFormat = LogicalDevice::Instance->GetDepthFormat();
        auto sampleCount = LogicalDevice::Instance->GetMaxSampleCount();

    	// Multi-sampled attachment that we render to
        VkAttachmentDescription const msaaAttachment{
            .format = surfaceFormat.format,
            .samples = sampleCount,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        };

        VkAttachmentDescription const swapChainAttachment{
            .format = surfaceFormat.format,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        };

        VkAttachmentDescription const depthAttachment{
            .format = depthFormat,
            .samples = sampleCount,
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        };

        // Note: hardware will automatically transition attachment to the specified layout
        // Note: index refers to attachment descriptions array
        VkAttachmentReference msaaAttachmentReference{
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        };

        VkAttachmentReference swapChainAttachmentReference{
            .attachment = 1,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        };

        VkAttachmentReference depthAttachmentRef{
            .attachment = 2,
            .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        };

        // Note: this is a description of how the attachments of the render pass will be used in this sub pass
        // e.g. if they will be read in shaders and/or drawn to
        std::vector<VkSubpassDescription> subPassDescription{
            VkSubpassDescription {
                .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                .colorAttachmentCount = 1,
                .pColorAttachments = &msaaAttachmentReference,
                .pResolveAttachments = &swapChainAttachmentReference,
                .pDepthStencilAttachment = &depthAttachmentRef,
            }
        };

        std::vector<VkAttachmentDescription> attachments = { msaaAttachment, swapChainAttachment, depthAttachment };

        mRenderPass = std::make_shared<RT::RenderPass>(RB::CreateRenderPass(
            LogicalDevice::Instance->GetVkDevice(),
            attachments.data(),
            static_cast<uint32_t>(attachments.size()),
            subPassDescription.data(),
            static_cast<uint32_t>(subPassDescription.size()),
            nullptr,
            0
        ));
    }

    //-------------------------------------------------------------------------------------------------

    void DisplayRenderPass::CreatePresentToDrawBarrier()
    {
        // If present queue family and graphics queue family are different, then a barrier is necessary
        // The barrier is also needed initially to transition the image to the present layout
        VkImageMemoryBarrier presentToDrawBarrier = {};
        presentToDrawBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        presentToDrawBarrier.srcAccessMask = 0;
        presentToDrawBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        presentToDrawBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        presentToDrawBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        auto const presentQueueFamily = LogicalDevice::Instance->GetPresentQueueFamily();
        auto const graphicQueueFamily = LogicalDevice::Instance->GetGraphicQueueFamily();

        if (presentQueueFamily != graphicQueueFamily)
        {
            presentToDrawBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            presentToDrawBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        }
        else
        {
            presentToDrawBarrier.srcQueueFamilyIndex = presentQueueFamily;
            presentToDrawBarrier.dstQueueFamilyIndex = graphicQueueFamily;
        }

        VkImageSubresourceRange const subResourceRange{
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        };

        presentToDrawBarrier.subresourceRange = subResourceRange;

        mPresentToDrawBarrier = presentToDrawBarrier;
    }

    //-------------------------------------------------------------------------------------------------

    static void changeDepthImageLayout(
        VkCommandBuffer commandBuffer,
        RT::DepthImageGroup const & depthImage,
        VkImageLayout initialLayout,
        VkImageLayout finalLayout,
        VkImageSubresourceRange const & subResourceRange
    )
    {
        auto const imageBarrier = VkImageMemoryBarrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = 0,
            .dstAccessMask = 0,
            .oldLayout = initialLayout,
            .newLayout = finalLayout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = depthImage.imageGroup->image,
            .subresourceRange = subResourceRange
        };

        std::vector<VkImageMemoryBarrier> const barriers {imageBarrier};
        RB::PipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            static_cast<uint32_t>(barriers.size()),
            barriers.data()
        );
    }

    //-------------------------------------------------------------------------------------------------

    void DisplayRenderPass::ClearDepthBufferIfNeeded(RT::CommandRecordState const & recordState)
    {
        if (mIsDepthImageUndefined)
        {
            auto const & depthImage = mDepth->GetDepthImage(recordState);

            VkImageSubresourceRange const subResourceRange{
                .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            };

            changeDepthImageLayout(
                recordState.commandBuffer,
                depthImage,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_GENERAL,
                subResourceRange
            );

            VkClearDepthStencilValue const depthStencil = { .depth = 1.0f, .stencil = 0 };

            // TODO Move to RF or RB
            vkCmdClearDepthStencilImage(
                recordState.commandBuffer,
                depthImage.imageGroup->image,
                VK_IMAGE_LAYOUT_GENERAL,
                &depthStencil,
                1,
                &subResourceRange
            );

            changeDepthImageLayout(
                recordState.commandBuffer,
                depthImage,
                VK_IMAGE_LAYOUT_GENERAL,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                subResourceRange
            );
        }

        mIsDepthImageUndefined = true;
    }

    //-------------------------------------------------------------------------------------------------

    void DisplayRenderPass::UsePresentToDrawBarrier(RT::CommandRecordState const & recordState)
    {
        mPresentToDrawBarrier.image = mSwapChain->GetSwapChainImage(recordState);

        std::vector<VkImageMemoryBarrier> const barriers {mPresentToDrawBarrier};

        RB::PipelineBarrier(
            recordState.commandBuffer,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            static_cast<uint32_t>(barriers.size()),
            barriers.data()
        );
    }

    //-------------------------------------------------------------------------------------------------

    void DisplayRenderPass::OnResize()
    {
        CreateFrameBuffers();
    }

    //-------------------------------------------------------------------------------------------------

}
