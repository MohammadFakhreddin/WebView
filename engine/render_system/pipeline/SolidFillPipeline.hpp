#pragma once

#include "RenderTypes.hpp"

#include "render_pass/DisplayRenderPass.hpp"

#include <glm/glm.hpp>

namespace MFA
{
    class SolidFillPipeline
    {
    public:

        using Position = glm::vec2;
        using Color = glm::vec3;

        struct Vertex
        {
            Position position{};
            Color color{};
        };

        struct Instance
        {
            Position innerPos0{};
            Color color0{};

            Position innerPos1{};
            Color color1{};

            Position innerPos2{};
            Color color2{};

            Position innerPos3{};
            Color color3{};

        	float borderRadius{};
        };

        explicit SolidFillPipeline(std::shared_ptr<DisplayRenderPass> displayRenderPass);

        ~SolidFillPipeline();

        [[nodiscard]]
        bool IsBinded(RT::CommandRecordState const& recordState) const;

        void BindPipeline(RT::CommandRecordState& recordState) const;

    private:

        void CreatePipeline();

    	std::shared_ptr<DisplayRenderPass> _displayRenderPass{};
        std::shared_ptr<RT::PipelineGroup> _pipeline{};
        
    };
}
