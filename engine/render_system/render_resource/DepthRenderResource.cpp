#include "DepthRenderResource.hpp"

#include  "LogicalDevice.hpp"

namespace MFA
{
    //-------------------------------------------------------------------------------------------------

    DepthRenderResource::DepthRenderResource()
        : RenderResource()
    {
        CreateDepthImages();
    }

    //-------------------------------------------------------------------------------------------------

    DepthRenderResource::~DepthRenderResource()
    {
        mDepthImageGroupList.clear();
    }

    //-------------------------------------------------------------------------------------------------

    RT::DepthImageGroup const& DepthRenderResource::GetDepthImage(int const index) const
    {
        return *mDepthImageGroupList[index];
    }

    //-------------------------------------------------------------------------------------------------

    RT::DepthImageGroup const& DepthRenderResource::GetDepthImage(RT::CommandRecordState const& recordState) const
    {
        return GetDepthImage(recordState.imageIndex);
    }

    //-------------------------------------------------------------------------------------------------

    void DepthRenderResource::OnResize()
    {
        mDepthImageGroupList.clear();
        CreateDepthImages();
    }

    //-------------------------------------------------------------------------------------------------

    void DepthRenderResource::CreateDepthImages()
    {
        auto surfaceCapabilities = LogicalDevice::Instance->GetSurfaceCapabilities();
        auto const swapChainExtend = VkExtent2D{
            .width = surfaceCapabilities.currentExtent.width,
            .height = surfaceCapabilities.currentExtent.height
        };

        mDepthImageGroupList.resize(LogicalDevice::Instance->GetSwapChainImageCount());
        for (auto& depthImage : mDepthImageGroupList)
        {
            depthImage = RB::CreateDepthImage(
                LogicalDevice::Instance->GetPhysicalDevice(),
                LogicalDevice::Instance->GetVkDevice(),
                swapChainExtend,
                LogicalDevice::Instance->GetDepthFormat(),
                RB::CreateDepthImageOptions{
                    .usageFlags = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
                    .samplesCount = LogicalDevice::Instance->GetMaxSampleCount()
                }
            );
        }
    }

    //-------------------------------------------------------------------------------------------------

}
