#include "BorderPipeline.hpp"

#include "RenderBackend.hpp"
#include "LogicalDevice.hpp"
#include "ImportShader.hpp"
#include "BedrockPath.hpp"

#include <utility>


namespace MFA
{

	//=================================================================

	BorderPipeline::BorderPipeline(std::shared_ptr<DisplayRenderPass> displayRenderPass)
		: _displayRenderPass(std::move(displayRenderPass))
	{
		CreatePipeline();
	}

	//=================================================================

	BorderPipeline::~BorderPipeline()
	{
		_pipeline = nullptr;
	}

	//=================================================================

	bool BorderPipeline::IsBinded(RT::CommandRecordState const& recordState) const
	{
		if (recordState.pipeline == _pipeline.get())
		{
			return true;
		}
		return false;
	}

	//=================================================================

	void BorderPipeline::BindPipeline(RT::CommandRecordState& recordState) const
	{
		if (IsBinded(recordState))
		{
			return;
		}

		RB::BindPipeline(recordState, *_pipeline);
	}

    //=================================================================

    void BorderPipeline::SetPushConstant(RT::CommandRecordState &recordState,
                                            PushConstants const &pushConstant) const
    {
        RB::PushConstants(recordState, _pipeline->pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, Alias{pushConstant});
    }

    //=================================================================

    void BorderPipeline::Reload()
	{
	    CreatePipeline();
	}

    //=================================================================

	void BorderPipeline::CreatePipeline()
	{
		{// Vertex shader
			bool success = Importer::CompileShaderToSPV(
				Path::Instance()->Get("engine/shaders/border_pipeline/BorderPipeline.vert.hlsl"),
				Path::Instance()->Get("engine/shaders/border_pipeline/BorderPipeline.vert.spv"),
				"vert"
			);
			MFA_ASSERT(success == true);
		}
		auto cpuVertexShader = Importer::ShaderFromSPV(
			Path::Instance()->Get("engine/shaders/border_pipeline/BorderPipeline.vert.spv"),
			VK_SHADER_STAGE_VERTEX_BIT,
			"main"
		);
		auto gpuVertexShader = RB::CreateShader(
			LogicalDevice::Instance->GetVkDevice(),
			cpuVertexShader
		);

		{// Fragment shader
			bool success = Importer::CompileShaderToSPV(
				Path::Instance()->Get("engine/shaders/border_pipeline/BorderPipeline.frag.hlsl"),
				Path::Instance()->Get("engine/shaders/border_pipeline/BorderPipeline.frag.spv"),
				"frag"
			);
			MFA_ASSERT(success == true);
		}
		auto cpuFragmentShader = Importer::ShaderFromSPV(
			Path::Instance()->Get("engine/shaders/border_pipeline/BorderPipeline.frag.spv"),
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
				.binding = 1,
				.stride = sizeof(Instance),
				.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE,
			}
		};

		std::vector<VkVertexInputAttributeDescription> inputAttributeDescriptions{};

		// Position
		inputAttributeDescriptions.emplace_back(VkVertexInputAttributeDescription{
			.location = static_cast<uint32_t>(inputAttributeDescriptions.size()),
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(Vertex, position),
		});
		// Color
		inputAttributeDescriptions.emplace_back(VkVertexInputAttributeDescription{
			.location = static_cast<uint32_t>(inputAttributeDescriptions.size()),
			.binding = 0,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(Vertex, color),
		});

		// topLeftPos
		inputAttributeDescriptions.emplace_back(VkVertexInputAttributeDescription{
			.location = static_cast<uint32_t>(inputAttributeDescriptions.size()),
			.binding = 1,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(Instance, topLeftPos)
		});
		// topLeftRadius
		inputAttributeDescriptions.emplace_back(VkVertexInputAttributeDescription{
			.location = static_cast<uint32_t>(inputAttributeDescriptions.size()),
			.binding = 1,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(Instance, topLeftRadius)
		});

		// bottomLeftPos
		inputAttributeDescriptions.emplace_back(VkVertexInputAttributeDescription{
			.location = static_cast<uint32_t>(inputAttributeDescriptions.size()),
			.binding = 1,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(Instance, bottomLeftPos)
		});
		// bottomLeftRadius
		inputAttributeDescriptions.emplace_back(VkVertexInputAttributeDescription{
			.location = static_cast<uint32_t>(inputAttributeDescriptions.size()),
			.binding = 1,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(Instance, bottomLeftRadius)
		});

		// topRightPos
		inputAttributeDescriptions.emplace_back(VkVertexInputAttributeDescription{
			.location = static_cast<uint32_t>(inputAttributeDescriptions.size()),
			.binding = 1,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(Instance, topRightPos)
		});
		// topRightRadius
		inputAttributeDescriptions.emplace_back(VkVertexInputAttributeDescription{
			.location = static_cast<uint32_t>(inputAttributeDescriptions.size()),
			.binding = 1,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(Instance, topRightRadius)
		});

		// bottomRightPos
		inputAttributeDescriptions.emplace_back(VkVertexInputAttributeDescription{
			.location = static_cast<uint32_t>(inputAttributeDescriptions.size()),
			.binding = 1,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(Instance, bottomRightPos)
		});
		// bottomRightRadius
		inputAttributeDescriptions.emplace_back(VkVertexInputAttributeDescription{
			.location = static_cast<uint32_t>(inputAttributeDescriptions.size()),
			.binding = 1,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(Instance, bottomRightRadius)
		});
        // leftWidth
        inputAttributeDescriptions.emplace_back(VkVertexInputAttributeDescription{
			.location = static_cast<uint32_t>(inputAttributeDescriptions.size()),
			.binding = 1,
			.format = VK_FORMAT_R32_SFLOAT,
			.offset = offsetof(Instance, leftWidth)
		});
        // topWidth
        inputAttributeDescriptions.emplace_back(VkVertexInputAttributeDescription{
			.location = static_cast<uint32_t>(inputAttributeDescriptions.size()),
			.binding = 1,
			.format = VK_FORMAT_R32_SFLOAT,
			.offset = offsetof(Instance, leftWidth)
		});
        // rightWidth
        inputAttributeDescriptions.emplace_back(VkVertexInputAttributeDescription{
			.location = static_cast<uint32_t>(inputAttributeDescriptions.size()),
			.binding = 1,
			.format = VK_FORMAT_R32_SFLOAT,
			.offset = offsetof(Instance, rightWidth)
		});
        // bottomWidth
        inputAttributeDescriptions.emplace_back(VkVertexInputAttributeDescription{
			.location = static_cast<uint32_t>(inputAttributeDescriptions.size()),
			.binding = 1,
			.format = VK_FORMAT_R32_SFLOAT,
			.offset = offsetof(Instance, bottomWidth)
		});

		RB::CreateGraphicPipelineOptions pipelineOptions{};
		pipelineOptions.useStaticViewportAndScissor = false;
		pipelineOptions.primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		pipelineOptions.rasterizationSamples = LogicalDevice::Instance->GetMaxSampleCount();            // TODO Find a way to set sample count to 1. We only need MSAA for pbr-pipeline
		pipelineOptions.cullMode = VK_CULL_MODE_NONE;
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
        
		const auto pipelineLayout = RB::CreatePipelineLayout(
			LogicalDevice::Instance->GetVkDevice(),
			0,
			nullptr,
            pushConstantRanges.size(),
            pushConstantRanges.data()
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