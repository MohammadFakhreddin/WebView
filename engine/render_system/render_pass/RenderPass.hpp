#pragma once

#include "BedrockPlatforms.hpp"
#include "RenderTypes.hpp"

#include <vulkan/vulkan.h>

namespace MFA {

    // Render pass has fixed resources. Change in resource causes change in frame-buffer
    class RenderPass {

    public:

        explicit RenderPass();

        virtual ~RenderPass();

        virtual VkRenderPass GetVkRenderPass() = 0;

        virtual void Begin(RT::CommandRecordState & recordState);

        virtual void End(RT::CommandRecordState & recordState);

        virtual void OnResize() = 0;

    protected:

        [[nodiscard]]
        bool getIsRenderPassActive() const {
            return mIsRenderPassActive;
        }

    private:

        bool mIsRenderPassActive = false;

        int mResizeEventId = -1;

    };

}
