#pragma once

#include "render_pass/DisplayRenderPass.hpp"

#include <glm/glm.hpp>

namespace MFA
{
    class TextOverlayPipeline
    {
    public:

        struct Vertex
        {
            glm::vec2 position{};
            glm::vec2 uv{};
            glm::vec3 color{};
        };

        explicit TextOverlayPipeline(
            std::shared_ptr<MFA::DisplayRenderPass> displayRenderPass,
            std::shared_ptr<MFA::RT::SamplerGroup> sampler
        );

        ~TextOverlayPipeline();

        [[nodiscard]]
        bool IsBinded(MFA::RT::CommandRecordState const& recordState) const;

        void BindPipeline(MFA::RT::CommandRecordState& recordState) const;

        MFA::RT::DescriptorSetGroup CreateDescriptorSet(MFA::RT::GpuTexture const & texture);

    private:

        void CreateDescriptorLayout();
        
        void CreatePipeline();
        
        std::shared_ptr<MFA::DisplayRenderPass> _displayRenderPass{};

        std::shared_ptr<MFA::RT::SamplerGroup> _sampler{};

        std::shared_ptr<MFA::RT::DescriptorPool> _descriptorPool{};

        std::shared_ptr<MFA::RT::DescriptorSetLayoutGroup> _descriptorLayout{};

        std::shared_ptr<MFA::RT::PipelineGroup> _pipeline{};

    };
}