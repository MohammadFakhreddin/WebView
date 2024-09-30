#pragma once

#include "RenderTypes.hpp"

#include <glm/glm.hpp>

#include "render_pass/DisplayRenderPass.hpp"

namespace MFA
{
    class SolidFillPipeline
    {
    public:

        struct Vertex
        {
            glm::vec3 position{};
            glm::vec3 color{};
        };

        struct Instance
        {
            glm::vec3 innerPos0{};
            glm::vec3 innerPos1{};
            glm::vec3 innerPos2{};
            glm::vec3 innerPos3{};
            float borderRadius{};
        };

        struct Boundary
        {
            glm::vec3 pos0{};
            float placeholder0{};

            glm::vec3 pos1{};
            float placeholder1{};

            glm::vec3 pos2{};
            float placeholder2{};

            glm::vec3 pos3{};
            float placeholder3{};

            float borderRadius{};
            glm::vec3 placeholder4{};
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
        std::shared_ptr<RT::DescriptorPool> _descriptorPool{};
        std::shared_ptr<RT::DescriptorSetLayoutGroup> _descriptorLayout{};

    };
}
