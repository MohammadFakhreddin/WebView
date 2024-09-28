#pragma once

#include "pipeline/PointPipeline.hpp"
#include "BedrockMath.hpp"

namespace MFA
{
    class PointRenderer
    {
    public:

        explicit PointRenderer(std::shared_ptr<MFA::PointPipeline> pointPipeline);

        void Draw(
            MFA::RT::CommandRecordState& recordState,
            glm::vec3 const& position,
            glm::vec4 const& color = { 1.0f, 0.0f, 0.0f, 1.0f },
            float pointSize = 10.0f
        ) const;

        std::shared_ptr<MFA::PointPipeline> _pointPipeline{};
        std::shared_ptr<MFA::RT::BufferAndMemory> _vertexBuffer{};
        std::shared_ptr<MFA::RT::BufferAndMemory> _indexBuffer{};
        int _indexCount{};
    };
}
