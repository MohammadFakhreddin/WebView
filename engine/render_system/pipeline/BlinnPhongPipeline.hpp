#pragma once

#include "RenderTypes.hpp"
#include "render_pass/DisplayRenderPass.hpp"

#include <glm/glm.hpp>

namespace shared
{
	class SurfaceMeshRenderer;
}

namespace MFA
{
    class BlinnPhongPipeline
    {
    public:

        struct Vertex
        {
            glm::vec3 position{};
            glm::vec3 normal{};
        };

        struct ViewProjection
        {
            glm::mat4 viewProjection{};
            glm::vec4 cameraPosition{};
        };

        struct PushConstants
        {
            glm::mat4 model;
            // TODO: Move these somewhere else, In a separate file
            glm::vec4 materialColor;
            glm::vec4 lightPosition;
            glm::vec4 lightColor;
        };

        struct Params
        {
            int maxSets = 100;
            VkCullModeFlags cullModeFlags = VK_CULL_MODE_BACK_BIT;
            VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL;
        };

        explicit BlinnPhongPipeline(
            std::shared_ptr<DisplayRenderPass> displayRenderPass,
            std::shared_ptr<RT::BufferGroup> viewProjectionBuffer
        );

        explicit BlinnPhongPipeline(
            std::shared_ptr<DisplayRenderPass> displayRenderPass,
            std::shared_ptr<RT::BufferGroup> viewProjectionBuffer,
            Params params
        );

        ~BlinnPhongPipeline();

        [[nodiscard]]
        bool IsBinded(RT::CommandRecordState const& recordState) const;

        void BindPipeline(RT::CommandRecordState& recordState) const;

        void SetPushConstants(RT::CommandRecordState& recordState, PushConstants pushConstants) const;

    private:

        void CreateDescriptorSetLayout();

        void CreatePipeline(Params const & params);

        void CreateDescriptorSets();

        std::shared_ptr<RT::DescriptorPool> mDescriptorPool{};
        std::shared_ptr<RT::DescriptorSetLayoutGroup> mDescriptorSetLayout{};
        std::shared_ptr<RT::PipelineGroup> mPipeline{};
        std::shared_ptr<RT::BufferGroup> mViewProjBuffer{};

        RT::DescriptorSetGroup mDescriptorSetGroup{};

        std::shared_ptr<DisplayRenderPass> mDisplayRenderPass{};

    };
}
