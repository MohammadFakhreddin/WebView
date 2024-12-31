#pragma once

#include "RenderTypes.hpp"
#include "render_pass/DisplayRenderPass.hpp"
#include "IShadingPipeline.hpp"

#include <glm/glm.hpp>

namespace MFA
{
    class SolidFillPipeline : public IShadingPipeline
    {
    public:

        using Position = glm::vec2;
        using Radius = glm::vec2;
        using Color = glm::vec4;

        struct Vertex
        {
            Position position{};
            Radius borderRadius{};
            Color color{};
        };

        struct Instance
        {
            Position topLeftPos{};
        	Radius topLeftRadius{};
            Color topLeftColor{};

            Position bottomLeftPos{};
            Radius bottomLeftRadius{};
            Color bottomLeftColor{};

            Position topRightPos{};
            Radius topRightRadius{};
            Color topRightColor{};

            Position bottomRightPos{};
            Radius bottomRightRadius{};
            Color bottomRightColor{};

        };

        struct PushConstants
        {
            glm::mat4 model{};
        };

        explicit SolidFillPipeline(std::shared_ptr<DisplayRenderPass> displayRenderPass);

        ~SolidFillPipeline();

        [[nodiscard]]
        bool IsBinded(RT::CommandRecordState const& recordState) const;

        void BindPipeline(RT::CommandRecordState& recordState) const;

        void SetPushConstant(RT::CommandRecordState & recordState, PushConstants const & pushConstant) const;

        void Reload() override;

    private:

        void CreatePipeline();

    private:

        std::shared_ptr<DisplayRenderPass> _displayRenderPass{};
        std::shared_ptr<RT::PipelineGroup> _pipeline{};
        
    };
}
