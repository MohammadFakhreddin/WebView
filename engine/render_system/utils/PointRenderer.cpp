#include "PointRenderer.hpp"

#include "LogicalDevice.hpp"
#include "pipeline/PointPipeline.hpp"
#include "BedrockMath.hpp"
#include "BufferTracker.hpp"

namespace MFA
{

	//-------------------------------------------------------------------------------------------------

	PointRenderer::PointRenderer(std::shared_ptr<MFA::PointPipeline> pointPipeline)
		: _pointPipeline(std::move(pointPipeline))
	{
		using namespace MFA;

		std::vector<glm::vec3> vertices{ {0.0f, 0.0f, 0.0f} };
		std::vector<uint16_t> indices{ 0 };

		Alias const verticesAlias{ vertices.data(), vertices.size() };
		Alias const indicesAlias{ indices.data(), indices.size() };

		auto* device = LogicalDevice::Instance;

		auto commandBuffer = RB::BeginSingleTimeCommand(
			device->GetVkDevice(),
			device->GetGraphicCommandPool()
		);
		auto const vertexStageBuffer = RB::CreateStageBuffer(
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
		auto const indexStageBuffer = RB::CreateStageBuffer(
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

	void PointRenderer::Draw(
		MFA::RT::CommandRecordState& recordState,
		glm::vec3 const& position,
		glm::vec4 const& color,
		float pointSize
	) const
	{
		using namespace MFA;

		_pointPipeline->BindPipeline(recordState);

		glm::mat4 matrix = Math::Translate(position);

		_pointPipeline->SetPushConstants(
			recordState,
			PointPipeline::PushConstants{
				.model = matrix,
				.color = color,
				.pointSize = pointSize
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

	//-------------------------------------------------------------------------------------------------

}