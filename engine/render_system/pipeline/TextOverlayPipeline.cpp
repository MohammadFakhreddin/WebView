#include "TextOverlayPipeline.hpp"

#include "BedrockPath.hpp"
#include "DescriptorSetSchema.hpp"
#include "ImportShader.hpp"
#include "LogicalDevice.hpp"

using namespace MFA;

//-------------------------------------------------------------------------------------------------

TextOverlayPipeline::TextOverlayPipeline(
	std::shared_ptr<MFA::DisplayRenderPass> displayRenderPass,
	std::shared_ptr<MFA::RT::SamplerGroup> sampler
)
	: _displayRenderPass(std::move(displayRenderPass))
	, _sampler(std::move(sampler))
{
	_descriptorPool = RB::CreateDescriptorPool(
		LogicalDevice::Instance->GetVkDevice(),
		1000
	);
	CreateDescriptorLayout();
	CreatePipeline();
}

//-------------------------------------------------------------------------------------------------

TextOverlayPipeline::~TextOverlayPipeline()
{
	_pipeline = nullptr;
	_descriptorLayout = nullptr;
	_descriptorPool = nullptr;
}

//-------------------------------------------------------------------------------------------------
// TODO: Move to base class
bool TextOverlayPipeline::IsBinded(RT::CommandRecordState const& recordState) const
{
	if (recordState.pipeline == _pipeline.get())
	{
		return true;
	}
	return false;
}

//-------------------------------------------------------------------------------------------------

void TextOverlayPipeline::BindPipeline(RT::CommandRecordState& recordState) const
{
	if (IsBinded(recordState))
	{
		return;
	}

	RB::BindPipeline(recordState, *_pipeline);
}

//-------------------------------------------------------------------------------------------------

void TextOverlayPipeline::CreateDescriptorLayout()
{
	std::vector<VkDescriptorSetLayoutBinding> bindings{};

	// Sampler
	VkDescriptorSetLayoutBinding const combinedImageSampler{
		.binding = static_cast<uint32_t>(bindings.size()),
		.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		.descriptorCount = 1,
		.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
	};
	bindings.emplace_back(combinedImageSampler);

	_descriptorLayout = RB::CreateDescriptorSetLayout(
		LogicalDevice::Instance->GetVkDevice(),
		static_cast<uint8_t>(bindings.size()),
		bindings.data()
	);
}

//-------------------------------------------------------------------------------------------------

RT::DescriptorSetGroup TextOverlayPipeline::CreateDescriptorSet(RT::GpuTexture const & texture)
{
	auto descriptorSetGroup = RB::CreateDescriptorSet(
		LogicalDevice::Instance->GetVkDevice(),
		_descriptorPool->descriptorPool,
		_descriptorLayout->descriptorSetLayout,
		1
	);

	auto const& descriptorSet = descriptorSetGroup.descriptorSets[0];
	MFA_ASSERT(descriptorSet != VK_NULL_HANDLE);

	DescriptorSetSchema schema { descriptorSet };
	
	VkDescriptorImageInfo info {
		.sampler = _sampler->sampler,
		.imageView = texture.imageView->imageView,
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	};
	schema.AddCombinedImageSampler(&info);

	schema.UpdateDescriptorSets();

	return descriptorSetGroup;
}

//-------------------------------------------------------------------------------------------------

void TextOverlayPipeline::SetPushConstant(
    RT::CommandRecordState& recordState,
    PushConstants const& pushConstant
) const
{
    RB::PushConstants(
        recordState,
        _pipeline->pipelineLayout,
        VK_SHADER_STAGE_VERTEX_BIT,
        0,
        Alias{ pushConstant }
    );
}

//-------------------------------------------------------------------------------------------------

void TextOverlayPipeline::CreatePipeline()
{
	// Vertex shader
	{
		bool success = Importer::CompileShaderToSPV(
			Path::Instance()->Get("engine/shaders/text_overlay_pipeline/TextOverlay.vert.hlsl"),
			Path::Instance()->Get("engine/shaders/text_overlay_pipeline/TextOverlay.vert.spv"),
			"vert"
		);
		MFA_ASSERT(success == true);
	}
	auto cpuVertexShader = Importer::ShaderFromSPV(
		Path::Instance()->Get("engine/shaders/text_overlay_pipeline/TextOverlay.vert.spv"),
		VK_SHADER_STAGE_VERTEX_BIT,
		"main"
	);
	auto gpuVertexShader = RB::CreateShader(
		LogicalDevice::Instance->GetVkDevice(),
		cpuVertexShader
	);

	// Fragment shader
	{
		bool success = Importer::CompileShaderToSPV(
			Path::Instance()->Get("engine/shaders/text_overlay_pipeline/TextOverlay.frag.hlsl"),
			Path::Instance()->Get("engine/shaders/text_overlay_pipeline/TextOverlay.frag.spv"),
			"frag"
		);
		MFA_ASSERT(success == true);
	}
	auto cpuFragmentShader = Importer::ShaderFromSPV(
		Path::Instance()->Get("engine/shaders/text_overlay_pipeline/TextOverlay.frag.spv"),
		VK_SHADER_STAGE_FRAGMENT_BIT,
		"main"
	);
	auto gpuFragmentShader = RB::CreateShader(
		LogicalDevice::Instance->GetVkDevice(),
		cpuFragmentShader
	);

	std::vector<RT::GpuShader const*> shaders{ gpuVertexShader.get(), gpuFragmentShader.get() };

	VkVertexInputBindingDescription const bindingDescription{
		.binding = 0,
		.stride = sizeof(Vertex),
		.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
	};

	std::vector<VkVertexInputAttributeDescription> inputAttributeDescriptions{};
	// Position
	inputAttributeDescriptions.emplace_back(VkVertexInputAttributeDescription{
		.location = static_cast<uint32_t>(inputAttributeDescriptions.size()),
		.binding = 0,
		.format = VK_FORMAT_R32G32_SFLOAT,
		.offset = offsetof(Vertex, position),
	});
	// UV
	inputAttributeDescriptions.emplace_back(VkVertexInputAttributeDescription{
		.location = static_cast<uint32_t>(inputAttributeDescriptions.size()),
		.binding = 0,
		.format = VK_FORMAT_R32G32_SFLOAT,
		.offset = offsetof(Vertex, uv),
	});
	// Color
	inputAttributeDescriptions.emplace_back(VkVertexInputAttributeDescription{
		.location = static_cast<uint32_t>(inputAttributeDescriptions.size()),
		.binding = 0,
		.format = VK_FORMAT_R32G32B32_SFLOAT,
		.offset = offsetof(Vertex, color),
	});

	RB::CreateGraphicPipelineOptions pipelineOptions{};
	pipelineOptions.useStaticViewportAndScissor = false;
	pipelineOptions.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
	pipelineOptions.rasterizationSamples = LogicalDevice::Instance->GetMaxSampleCount();
	pipelineOptions.cullMode = VK_CULL_MODE_BACK_BIT;
	pipelineOptions.colorBlendAttachments.blendEnable = VK_TRUE;
	pipelineOptions.polygonMode = VK_POLYGON_MODE_FILL;
	pipelineOptions.frontFace = VK_FRONT_FACE_CLOCKWISE;
	pipelineOptions.depthStencil.depthTestEnable = false;
	pipelineOptions.depthStencil.depthWriteEnable = false;

    std::vector<VkPushConstantRange> pushConstantRanges{};
    {
        pushConstantRanges.emplace_back();
        auto & pushConstant = pushConstantRanges.back();
        pushConstant.size = sizeof(PushConstants);
        pushConstant.offset = 0;
        pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    }
	
	// pipeline layout
	std::vector<VkDescriptorSetLayout> setLayout{_descriptorLayout->descriptorSetLayout};
	
	const auto pipelineLayout = RB::CreatePipelineLayout(
		LogicalDevice::Instance->GetVkDevice(),
		setLayout.size(),
		setLayout.data(),
		pushConstantRanges.size(),
        pushConstantRanges.data()
	);

	auto surfaceCapabilities = LogicalDevice::Instance->GetSurfaceCapabilities();

	_pipeline = RB::CreateGraphicPipeline(
		LogicalDevice::Instance->GetVkDevice(),
		static_cast<uint8_t>(shaders.size()),
		shaders.data(),
		1,
		&bindingDescription,
		static_cast<uint8_t>(inputAttributeDescriptions.size()),
		inputAttributeDescriptions.data(),
		surfaceCapabilities.currentExtent,
		_displayRenderPass->GetVkRenderPass(),
		pipelineLayout,
		pipelineOptions
	);
}

//-------------------------------------------------------------------------------------------------
