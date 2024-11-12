#include "SolidFillRenderer.hpp"

#include "LogicalDevice.hpp"

//------------------------------------------------------------------

MFA::SolidFillRenderer::SolidFillRenderer(std::shared_ptr<SolidFillPipeline> pipeline)
	: _pipeline(std::move(pipeline))
{
}

//------------------------------------------------------------------

std::shared_ptr<MFA::LocalBufferTracker> MFA::SolidFillRenderer::AllocateBuffer(
	Position const& pos0,
	Position const& pos1, 
	Position const& pos2, 
	Position const& pos3, 
	
	Color const& color0, 
	Color const& color1,
	Color const& color2, 
	Color const& color3,

	float const borderRadius
)
{
	Pipeline::Instance data{
		.innerPos0 = pos0,
		.color0 = color0,
		.innerPos1 = pos1,
		.color1 = color1,
		.innerPos2 = pos2,
		.color2 = color2,
		.innerPos3 = pos3,
		.color3 = color3,
		.borderRadius = borderRadius
	};

	auto const * device = LogicalDevice::Instance;
	// This should be 1
	int const bufferCount = device->GetMaxFramePerFlight();

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

void MFA::SolidFillRenderer::Draw(RT::CommandRecordState& recordState, LocalBufferTracker const& vertexBuffer) const
{
	_pipeline->BindPipeline(recordState);

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
