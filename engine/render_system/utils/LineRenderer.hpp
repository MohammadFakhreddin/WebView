#pragma once

#include "pipeline/LinePipeline.hpp"
#include "BedrockMath.hpp"

namespace MFA
{

    class LineRenderer
    {
    public:

        explicit LineRenderer(std::shared_ptr<MFA::LinePipeline> linePipeline);

        void Draw(
            MFA::RT::CommandRecordState& recordState,
            glm::vec3 const& from,
            glm::vec3 const& to,
            glm::vec4 const& color = { 0.0f, 1.0f, 0.0f, 1.0f }
        ) const;

        std::shared_ptr<MFA::LinePipeline> _linePipeline{};
        std::shared_ptr<MFA::RT::BufferAndMemory> _vertexBuffer{};
        std::shared_ptr<MFA::RT::BufferAndMemory> _indexBuffer{};
        glm::vec3 _startPos{ -0.5f, 0.0f, 0.0f };
        glm::vec3 _endPos{ 0.5f, 0.0f, 0.0f };
        glm::vec3 _direction{};
        glm::vec3 _center{};
        float _length{};
        int _indexCount{};
    };

}