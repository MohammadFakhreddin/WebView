#include "LineRenderer.hpp"

#include "LogicalDevice.hpp"
#include "pipeline/LinePipeline.hpp"
#include "BedrockMath.hpp"
#include "BufferTracker.hpp"

namespace MFA
{

	//-------------------------------------------------------------------------------------------------

	LineRenderer::LineRenderer(std::shared_ptr<MFA::LinePipeline> linePipeline)
		: _linePipeline(std::move(linePipeline))
	{
		using namespace MFA;

		{
			auto const vector = _endPos - _startPos;
			_length = glm::length(vector);
			_direction = vector / _length;
			_center = (_endPos + _startPos) * 0.5f;
		}

		std::vector<glm::vec3> vertices{
			_startPos,
			_endPos,
		};
		std::vector<uint16_t> indices{ 0, 1 };

		Alias const verticesAlias{ vertices.data(), vertices.size() };
		Alias const indicesAlias{ indices.data(), indices.size() };

		auto* device = LogicalDevice::Instance;

		auto commandBuffer = RB::BeginSingleTimeCommand(
			device->GetVkDevice(),
			device->GetGraphicCommandPool()
		);
		auto vertexStageBuffer = RB::CreateStageBuffer(
			device->GetVkDevice(),
			device->GetPhysicalDevice(),
			verticesAlias.Len(),
			1
		);
		_vertexBuffer = RB::CreateVertexBuffer(
			device->GetVkDevice(),
			device->GetPhysicalDevice(),
			commandBuffer,
			*vertexStageBuffer->buffers[0],
			verticesAlias
		);
		auto indexStageBuffer = RB::CreateStageBuffer(
			device->GetVkDevice(),
			device->GetPhysicalDevice(),
			indicesAlias.Len(),
			1
		);
		_indexBuffer = RB::CreateIndexBuffer(
			device->GetVkDevice(),
			device->GetPhysicalDevice(),
			commandBuffer,
			*indexStageBuffer->buffers[0],
			indicesAlias
		);
		RB::EndAndSubmitSingleTimeCommand(
			device->GetVkDevice(),
			device->GetGraphicCommandPool(),
			device->GetGraphicQueue(),
			commandBuffer
		);

		_indexCount = indices.size();
	}

	//-------------------------------------------------------------------------------------------------

	void LineRenderer::Draw(
		MFA::RT::CommandRecordState& recordState,
		glm::vec3 const& from0,
		glm::vec3 const& to0,
		glm::vec4 const& color
	) const
	{
		using namespace MFA;

		_linePipeline->BindPipeline(recordState);

		auto from = from0;
		auto to = to0;

		if (to.x < from.x)
		{
			auto temp = from;
			from = to;
			to = temp;
		}

		glm::vec3 const targetVector = to - from;
		float const targetLength = glm::length(targetVector);

		if (targetLength > 0.0f)
		{
			float const scale = targetLength / _length;
			glm::vec3 const targetCenter = (from + to) * 0.5f;

			auto const targetDirection = targetVector / targetLength;

			auto const rotation = Math::FindRotation(_direction, targetDirection);
			auto const rotationMat = glm::toMat4(rotation);
			auto const translateMat = Math::Translate(targetCenter - _center);
			auto const scaleMat = Math::Scale(glm::vec3{ scale, 1.0f, 1.0f });

			glm::mat4 matrix = translateMat
				* rotationMat
				* scaleMat;

			_linePipeline->SetPushConstants(
				recordState,
				LinePipeline::PushConstants{
					.model = matrix,
					.color = color
				}
			);

			RB::BindIndexBuffer(
				recordState,
				*_indexBuffer,
				0,
				VK_INDEX_TYPE_UINT16
			);

			RB::BindVertexBuffer(
				recordState,
				*_vertexBuffer,
				0,
				0
			);

			RB::DrawIndexed(
				recordState,
				_indexCount
			);
		}
	}

	//-------------------------------------------------------------------------------------------------

}