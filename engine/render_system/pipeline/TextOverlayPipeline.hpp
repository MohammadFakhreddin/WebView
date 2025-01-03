#pragma once

#include "render_pass/DisplayRenderPass.hpp"
#include "IShadingPipeline.hpp"

#include <glm/glm.hpp>

namespace MFA
{
    class TextOverlayPipeline : public IShadingPipeline
    {
    public:

        struct Vertex
        {
            glm::vec2 position{};
            glm::vec2 uv{};
            glm::vec3 color{};
        };

        struct PushConstants
        {
            glm::mat4 model{};
        };

        explicit TextOverlayPipeline(
            std::shared_ptr<DisplayRenderPass> displayRenderPass,
            std::shared_ptr<RT::SamplerGroup> sampler
        );

        ~TextOverlayPipeline();

        [[nodiscard]]
        bool IsBinded(RT::CommandRecordState const& recordState) const;

        void BindPipeline(RT::CommandRecordState& recordState) const;

        [[nodiscard]]
        RT::DescriptorSetGroup CreateDescriptorSet(RT::GpuTexture const & texture);

        void SetPushConstant(RT::CommandRecordState & recordState, PushConstants const & pushConstant) const;

        void Reload() override;

    private:

        void CreateDescriptorLayout();
        
        void CreatePipeline();
        
        std::shared_ptr<DisplayRenderPass> _displayRenderPass{};

        std::shared_ptr<RT::SamplerGroup> _sampler{};

        std::shared_ptr<RT::DescriptorPool> _descriptorPool{};

        std::shared_ptr<RT::DescriptorSetLayoutGroup> _descriptorLayout{};

        std::shared_ptr<RT::PipelineGroup> _pipeline{};

    };
}
