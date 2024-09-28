#pragma once

#include "IShadingPipeline.h"
#include "render_pass/DisplayRenderPass.hpp"

#include <glm/glm.hpp>
#include <memory>

namespace MFA
{
    class FlatShadingPipeline : public IShadingPipeline
    {
    public:

        struct Vertex
        {
            glm::vec3 position{};
            glm::vec2 baseColorUV{};
            glm::vec3 normal{};
        };

        struct ViewProjection
        {
            glm::mat4 matrix{};
        };

        struct LightSource
        {
            glm::vec3 dir{};
            float placeholder0{};
            glm::vec3 color{};
            float placeholder1{};
        };

        struct PushConstants
        {
            glm::mat4 model;
        };

        struct Material
        {
            glm::vec4 color {};
            int hasBaseColorTexture {};
            int placeholder0 {};
            int placeholder1 {};
            int placeholder2 {};
        };

        struct Params
        {
            int maxSets = 1000;
            VkCullModeFlags cullModeFlags = VK_CULL_MODE_BACK_BIT;
            VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
        };

        explicit FlatShadingPipeline(
            std::shared_ptr<DisplayRenderPass> displayRenderPass,
            std::shared_ptr<RT::BufferGroup> viewProjectionBuffer,
            std::shared_ptr<RT::SamplerGroup> sampler,
            std::shared_ptr<RT::BufferGroup> lightSourceBuffer,
            Params params = Params {
                .maxSets = 100,
                .cullModeFlags = VK_CULL_MODE_BACK_BIT,
                .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                .polygonMode = VK_POLYGON_MODE_FILL
            }
        );

        ~FlatShadingPipeline();

        [[nodiscard]]
        bool IsBinded(RT::CommandRecordState const& recordState) const;

        void BindPipeline(RT::CommandRecordState& recordState) const;

        void SetPushConstants(RT::CommandRecordState& recordState, PushConstants pushConstants) const;

        [[nodiscard]]
        RT::DescriptorSetGroup CreatePerGeometryDescriptorSetGroup(
            RT::BufferAndMemory const& material,
            RT::GpuTexture const& texture
        ) const;

        void reload() override;

    private:

        void CreatePerPipelineDescriptorSetLayout();

        void CreatePerGeometryDescriptorSetLayout();

        void CreatePipeline();

        void CreatePerPipelineDescriptorSets();

        std::shared_ptr<RT::DescriptorPool> mDescriptorPool{};

    	std::shared_ptr<RT::DescriptorSetLayoutGroup> mPerPipelineDescriptorLayout{};
        std::shared_ptr<RT::DescriptorSetLayoutGroup> mPerGeometryDescriptorLayout{};

        std::shared_ptr<RT::PipelineGroup> mPipeline{};
        std::shared_ptr<RT::BufferGroup> mViewProjBuffer{};
        std::shared_ptr<RT::BufferGroup> mLightSourceBuffer{};
        std::shared_ptr<RT::SamplerGroup> mSampler{};

        RT::DescriptorSetGroup mPerPipelineDescriptorSetGroup{};

        std::shared_ptr<DisplayRenderPass> mDisplayRenderPass{};

        Params _params{};
    };
}
