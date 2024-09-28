#include "RenderPass.hpp"

#include "BedrockAssert.hpp"
#include "LogicalDevice.hpp"

namespace MFA {


    //-------------------------------------------------------------------------------------------------

    RenderPass::RenderPass()
    {
        mResizeEventId = LogicalDevice::Instance->ResizeEventSignal2.Register([this]()->void
        {
            OnResize();
        });
    }

    //-------------------------------------------------------------------------------------------------

    RenderPass::~RenderPass()
    {
        LogicalDevice::Instance->ResizeEventSignal2.UnRegister(mResizeEventId);
    }

    //-------------------------------------------------------------------------------------------------

    void RenderPass::Begin(RT::CommandRecordState & recordState)
    {
        MFA_ASSERT(mIsRenderPassActive == false);
        mIsRenderPassActive = true;
        recordState.renderPass = this;
    }

    //-------------------------------------------------------------------------------------------------

    void RenderPass::End(RT::CommandRecordState & recordState)
    {
        MFA_ASSERT(mIsRenderPassActive == true);
        recordState.renderPass = nullptr;
        mIsRenderPassActive = false;
    }

    //-------------------------------------------------------------------------------------------------

}
