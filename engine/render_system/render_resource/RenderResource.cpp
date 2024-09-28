#include "RenderResource.hpp"

#include "LogicalDevice.hpp"

namespace MFA
{

    //-------------------------------------------------------------------------------------------------

    RenderResource::RenderResource()
    {
        assert(LogicalDevice::Instance != nullptr);
        mResizeEventId = LogicalDevice::Instance->ResizeEventSignal1.Register([this]()->void
        {
            OnResize();
        });
    }

    //-------------------------------------------------------------------------------------------------

    RenderResource::~RenderResource()
    {
        LogicalDevice::Instance->ResizeEventSignal1.UnRegister(mResizeEventId);
    }

    //-------------------------------------------------------------------------------------------------

}
