#include "SolidFillRenderer.hpp"

#include "LogicalDevice.hpp"

//------------------------------------------------------------------

MFA::SolidFillRenderer::SolidFillRenderer(std::shared_ptr<SolidFillPipeline> pipeline)
	: _pipeline(std::move(pipeline))
{
}

//------------------------------------------------------------------

std::shared_ptr<MFA::LocalBufferTracker> MFA::SolidFillRenderer::AllocateBuffer(
	Position const& topLeftPos,
	Position const& bottomLeftPos,
	Position const& topRightPos,
	Position const& bottomRightPos,
	
	Color const& topLeftColor,
	Color const& bottomLeftColor,
	Color const& topRightColor,
	Color const& bottomRightColor,

	Radius const& topLeftBorderRadius,
	Radius const& bottomLeftBorderRadius,
	Radius const& topRightBorderRadius,
	Radius const& bottomRightBorderRadius
)
{
	Pipeline::Instance data{
		.topLeftPos = topLeftPos,
		.topLeftRadius = topLeftBorderRadius,
		.topLeftColor = topLeftColor,

		.bottomLeftPos = bottomLeftPos,
		.bottomLeftRadius = bottomLeftBorderRadius,
		.bottomLeftColor = bottomLeftColor,

		.topRightPos = topRightPos,
		.topRightRadius = topRightBorderRadius,
		.topRightColor = topRightColor,

		.bottomRightPos = bottomRightPos,
		.bottomRightRadius = bottomRightBorderRadius,
		.bottomRightColor = bottomRightColor,
	};

	auto const * device = LogicalDevice::Instance;
	// This should be 1
	int const bufferCount = 1;

	auto vertexBuffer = RB::CreateBufferGroup(
		device->GetVkDevice(), 
		device->GetPhysicalDevice(), 
		sizeof(Pipeline::Instance), 
		bufferCount,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	auto stageBuffer = RB::CreateStageBuffer(
		device->GetVkDevice(), 
		device->GetPhysicalDevice(), 
		sizeof(Pipeline::Instance), 
		bufferCount
	);

	auto bufferTracker = std::make_shared<LocalBufferTracker>(vertexBuffer, stageBuffer);
	bufferTracker->SetData(Alias(data));

	return bufferTracker;
}

//------------------------------------------------------------------

void MFA::SolidFillRenderer::Draw(
    RT::CommandRecordState& recordState,
    Pipeline::PushConstants const & pushConstants,
    LocalBufferTracker const& vertexBuffer
) const
{
    // Note: We could have achieve the same thing with viewport and scissor and a fixed vertex buffer.
	_pipeline->BindPipeline(recordState);

    _pipeline->SetPushConstant(recordState, pushConstants);

	auto const & localBuffers = vertexBuffer.LocalBuffer().buffers;
	auto const & localBuffer = localBuffers[recordState.frameIndex % localBuffers.size()];
	VkBuffer buffers[2]{ localBuffer->buffer, localBuffer->buffer };
	VkDeviceSize bindingOffsets[2]{ 0,0 };

	vkCmdBindVertexBuffers(
		recordState.commandBuffer,
		0,
		2,
		buffers,
		bindingOffsets
	);

	vkCmdDraw(recordState.commandBuffer, 4, 1, 0, 0);
}

//------------------------------------------------------------------
