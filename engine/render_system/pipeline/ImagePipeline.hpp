#pragma once

#include "render_pass/DisplayRenderPass.hpp"
#include "IShadingPipeline.hpp"

#include <glm/glm.hpp>

namespace MFA
{
    class ImagePipeline : public IShadingPipeline
    {
    public:

        using Position = glm::vec2;
        using Radius = glm::vec2;
        using UV = glm::vec2;

        struct Vertex
        {
            Position position{};
            Radius radius{};
            UV uv{};
        };

        struct Instance
        {
            Position topLeftPos{};
            Radius topLeftRadius{};
            UV topLeftUV{};
            
            Position bottomLeftPos{};
            Radius bottomLeftRadius{};
            UV bottomLeftUV{};
            
            Position topRightPos{};
            Radius topRightRadius{};
            UV topRightUV{};
            
            Position bottomRightPos{};
            Radius bottomRightRadius{};
            UV bottomRightUV{};
        };

        struct PushConstants
        {
            glm::mat4 model{};
        };

        explicit ImagePipeline(
            std::shared_ptr<DisplayRenderPass> displayRenderPass,
            std::shared_ptr<RT::SamplerGroup> sampler
        );

        ~ImagePipeline();

        [[nodiscard]]
        bool IsBinded(RT::CommandRecordState const &recordState) const;

        void BindPipeline(RT::CommandRecordState &recordState) const;

        [[nodiscard]]
        RT::DescriptorSetGroup CreateDescriptorSet(RT::GpuTexture const &texture) const;

        void UpdateDescriptorSet(RT::DescriptorSetGroup &descriptorSetGroup, RT::GpuTexture const &texture) const;

        void FreeDescriptorSet(RT::DescriptorSetGroup &descriptorSetGroup) const;

        void SetPushConstant(RT::CommandRecordState &recordState, PushConstants const &pushConstant) const;

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
