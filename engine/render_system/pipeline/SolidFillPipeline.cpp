#include "SolidFillPipeline.hpp"

#include "RenderBackend.hpp"
#include "LogicalDevice.hpp"
#include "ImportShader.hpp"
#include "BedrockPath.hpp"

#include <utility>

namespace MFA
{

	//=================================================================

	SolidFillPipeline::SolidFillPipeline(std::shared_ptr<DisplayRenderPass> displayRenderPass)
		: _displayRenderPass(std::move(displayRenderPass))
	{
		_descriptorPool = RB::CreateDescriptorPool(LogicalDevice::Instance->GetVkDevice(), 1);
		CreatePipeline();
	}

	//=================================================================

	SolidFillPipeline::~SolidFillPipeline()
	{
		_pipeline = nullptr;
		_descriptorLayout = nullptr;
		_descriptorPool = nullptr;
	}

	//=================================================================

	bool SolidFillPipeline::IsBinded(RT::CommandRecordState const& recordState) const
	{
		if (recordState.pipeline == _pipeline.get())
		{
			return true;
		}
		return false;
	}

	//=================================================================

	void SolidFillPipeline::BindPipeline(RT::CommandRecordState& recordState) const
	{
		if (IsBinded(recordState))
		{
			return;
		}

		RB::BindPipeline(recordState, *_pipeline);
	}

	//=================================================================

	void SolidFillPipeline::CreatePipeline()
	{
		{// Vertex shader
			bool success = Importer::CompileShaderToSPV(
				Path::Instance->Get("engine/shaders/solid_fill_pipeline/SolidFillPipeline.vert.hlsl"),
				Path::Instance->Get("engine/shaders/solid_fill_pipeline/SolidFillPipeline.vert.spv"),
				"vert"
			);
			MFA_ASSERT(success == true);
		}
		auto cpuVertexShader = Importer::ShaderFromSPV(
			Path::Instance->Get("engine/shaders/solid_fill_pipeline/SolidFillPipeline.vert.spv"),
			VK_SHADER_STAGE_VERTEX_BIT,
			"main"
		);
		auto gpuVertexShader = RB::CreateShader(
			LogicalDevice::Instance->GetVkDevice(),
			cpuVertexShader
		);

		{// Fragment shader
			bool success = Importer::CompileShaderToSPV(
				Path::Instance->Get("engine/shaders/solid_fill_pipeline/SolidFillPipeline.frag.hlsl"),
				Path::Instance->Get("engine/shaders/solid_fill_pipeline/SolidFillPipeline.frag.spv"),
				"frag"
			);
			MFA_ASSERT(success == true);
		}
		auto cpuFragmentShader = Importer::ShaderFromSPV(
			Path::Instance->Get("engine/shaders/solid_fill_pipeline/SolidFillPipeline.frag.spv"),
			VK_SHADER_STAGE_FRAGMENT_BIT,
			"main"
		);
		auto gpuFragmentShader = RB::CreateShader(
			LogicalDevice::Instance->GetVkDevice(),
			cpuFragmentShader
		);

		std::vector<RT::GpuShader const*> shaders{ gpuVertexShader.get(), gpuFragmentShader.get() };

		std::vector<VkVertexInputBindingDescription> const bindingDescriptions{
			VkVertexInputBindingDescription {
				.binding = 0,
				.stride = sizeof(Vertex),
				.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
			},
			VkVertexInputBindingDescription {
				.binding = 0,
				.stride = sizeof(Vertex),
				.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE,
			}
		};

		std::vector<VkVertexInputAttributeDescription> inputAttributeDescriptions{};
		// Position
		inputAttributeDescriptions.emplace_back(VkVertexInputAttributeDescription{
			.location = static_cast<uint32_t>(inputAttributeDescriptions.size()),
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(Vertex, position),
		});
		// Color
		inputAttributeDescriptions.emplace_back(VkVertexInputAttributeDescription{
			.location = static_cast<uint32_t>(inputAttributeDescriptions.size()),
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(Vertex, color),
		});
		// InnerPos0
		inputAttributeDescriptions.emplace_back(VkVertexInputAttributeDescription{
			.location = static_cast<uint32_t>(inputAttributeDescriptions.size()),
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(Instance, innerPos0)
		});
		// InnerPos1
		inputAttributeDescriptions.emplace_back(VkVertexInputAttributeDescription{
			.location = static_cast<uint32_t>(inputAttributeDescriptions.size()),
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(Instance, innerPos1)
		});
		// InnerPos2
		inputAttributeDescriptions.emplace_back(VkVertexInputAttributeDescription{
			.location = static_cast<uint32_t>(inputAttributeDescriptions.size()),
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(Instance, innerPos2)
		});
		// InnerPos3
		inputAttributeDescriptions.emplace_back(VkVertexInputAttributeDescription{
			.location = static_cast<uint32_t>(inputAttributeDescriptions.size()),
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(Instance, innerPos3)
		});
		// BorderRadius
		inputAttributeDescriptions.emplace_back(VkVertexInputAttributeDescription{
			.location = static_cast<uint32_t>(inputAttributeDescriptions.size()),
			.binding = 0,
			.format = VK_FORMAT_R32_SFLOAT,
			.offset = offsetof(Instance, borderRadius)
		});

		RB::CreateGraphicPipelineOptions pipelineOptions{};
		pipelineOptions.useStaticViewportAndScissor = false;
		pipelineOptions.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		pipelineOptions.rasterizationSamples = LogicalDevice::Instance->GetMaxSampleCount();            // TODO Find a way to set sample count to 1. We only need MSAA for pbr-pipeline
		pipelineOptions.cullMode = VK_CULL_MODE_BACK_BIT;
		pipelineOptions.colorBlendAttachments.blendEnable = VK_TRUE;
		pipelineOptions.polygonMode = VK_POLYGON_MODE_FILL;
		pipelineOptions.frontFace = VK_FRONT_FACE_CLOCKWISE;
		pipelineOptions.depthStencil.depthTestEnable = false;
		pipelineOptions.depthStencil.depthWriteEnable = false;

		// pipeline layout
		std::vector<VkDescriptorSetLayout> setLayout{ _descriptorLayout->descriptorSetLayout };

		const auto pipelineLayout = RB::CreatePipelineLayout(
			LogicalDevice::Instance->GetVkDevice(),
			setLayout.size(),
			setLayout.data(),
			0,
			nullptr
		);

		auto surfaceCapabilities = LogicalDevice::Instance->GetSurfaceCapabilities();

		_pipeline = RB::CreateGraphicPipeline(
			LogicalDevice::Instance->GetVkDevice(),
			static_cast<uint8_t>(shaders.size()),
			shaders.data(),
			bindingDescriptions.size(),
			bindingDescriptions.data(),
			static_cast<uint8_t>(inputAttributeDescriptions.size()),
			inputAttributeDescriptions.data(),
			surfaceCapabilities.currentExtent,
			_displayRenderPass->GetVkRenderPass(),
			pipelineLayout,
			pipelineOptions
		);
	}

	//=================================================================

}
