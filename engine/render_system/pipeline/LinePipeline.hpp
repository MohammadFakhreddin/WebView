#pragma once

#include "RenderTypes.hpp"
#include "render_pass/DisplayRenderPass.hpp"

#include <glm/glm.hpp>

namespace MFA
{
    class LinePipeline 
    {
    public:

        struct Vertex
        {
            glm::vec3 position{};
        };

        struct PushConstants
        {
            glm::mat4 model;
            glm::vec4 color;
        };

        explicit LinePipeline(
            std::shared_ptr<DisplayRenderPass> displayRenderPass,
            int maxSets
        );

        ~LinePipeline();

        [[nodiscard]]
        bool IsBinded(RT::CommandRecordState const & recordState) const;

        void BindPipeline(RT::CommandRecordState & recordState) const;

        void SetPushConstants(RT::CommandRecordState& recordState, PushConstants pushConstants) const;

    private:

        void CreateDescriptorSetLayout();

        void CreatePipeline();

        std::shared_ptr<RT::DescriptorPool> mDescriptorPool {};
        std::shared_ptr<RT::DescriptorSetLayoutGroup> mDescriptorSetLayout{};
        std::shared_ptr<RT::PipelineGroup> mPipeline{};
        
        std::shared_ptr<DisplayRenderPass> mDisplayRenderPass {};
        
    };
}
