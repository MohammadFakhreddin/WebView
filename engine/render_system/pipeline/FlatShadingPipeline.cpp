#include "FlatShadingPipeline.hpp"

#include "BedrockPath.hpp"
#include "DescriptorSetSchema.hpp"
#include "ImportShader.hpp"
#include "LogicalDevice.hpp"

namespace MFA
{

	//-------------------------------------------------------------------------------------------------

	FlatShadingPipeline::FlatShadingPipeline(
		std::shared_ptr<DisplayRenderPass> displayRenderPass,
		std::shared_ptr<RT::BufferGroup> viewProjectionBuffer,
		std::shared_ptr<RT::SamplerGroup> sampler,
        std::shared_ptr<RT::BufferGroup> lightSourceBuffer,
		Params params
	)
		: _params(std::move(params))
	{
		mDisplayRenderPass = std::move(displayRenderPass);

		mViewProjBuffer = std::move(viewProjectionBuffer);

		mSampler = std::move(sampler);

        mLightSourceBuffer = std::move(lightSourceBuffer);

		mDescriptorPool = RB::CreateDescriptorPool(
			LogicalDevice::Instance->GetVkDevice(),
			_params.maxSets
		);

		CreatePerPipelineDescriptorSetLayout();
		CreatePerGeometryDescriptorSetLayout();
		CreatePipeline();
		CreatePerPipelineDescriptorSets();
	}

	//-------------------------------------------------------------------------------------------------

	FlatShadingPipeline::~FlatShadingPipeline()
	{
		mPipeline = nullptr;
		mPerPipelineDescriptorLayout = nullptr;
		mPerGeometryDescriptorLayout = nullptr;
		mDescriptorPool = nullptr;
	}

	//-------------------------------------------------------------------------------------------------

	bool FlatShadingPipeline::IsBinded(RT::CommandRecordState const& recordState) const
	{
		if (recordState.pipeline == mPipeline.get())
		{
			return true;
		}
		return false;
	}

	//-------------------------------------------------------------------------------------------------

	void FlatShadingPipeline::BindPipeline(RT::CommandRecordState& recordState) const
	{
		if (IsBinded(recordState))
		{
			return;
		}

		RB::BindPipeline(recordState, *mPipeline);
		RB::AutoBindDescriptorSet(recordState, RB::UpdateFrequency::PerPipeline, mPerPipelineDescriptorSetGroup);
	}

	//-------------------------------------------------------------------------------------------------

	void FlatShadingPipeline::SetPushConstants(RT::CommandRecordState& recordState, PushConstants pushConstants) const
	{
		RB::PushConstants(
			recordState,
			mPipeline->pipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			Alias(pushConstants)
		);
	}

	//-------------------------------------------------------------------------------------------------

	RT::DescriptorSetGroup FlatShadingPipeline::CreatePerGeometryDescriptorSetGroup(
		RT::BufferAndMemory const & material,
		RT::GpuTexture const & texture
	) const
	{
		auto perGeometryDescriptorSet = RB::CreateDescriptorSet(
			LogicalDevice::Instance->GetVkDevice(),
			mDescriptorPool->descriptorPool,
			mPerGeometryDescriptorLayout->descriptorSetLayout,
			1
		);

		auto const& descriptorSet = perGeometryDescriptorSet.descriptorSets[0];
		MFA_ASSERT(descriptorSet != VK_NULL_HANDLE);

		DescriptorSetSchema descriptorSetSchema{ descriptorSet };

		// Material
		VkDescriptorBufferInfo const bufferInfo{
			.buffer = material.buffer,
			.offset = 0,
			.range = material.size,
		};
		descriptorSetSchema.AddUniformBuffer(&bufferInfo);

		// BaseColor
		VkDescriptorImageInfo const texturesSamplerInfo{
			.sampler = VK_NULL_HANDLE,
			.imageView = texture.imageView->imageView,
			.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		};

		descriptorSetSchema.AddImage(&texturesSamplerInfo, 1);

		descriptorSetSchema.UpdateDescriptorSets();

		return perGeometryDescriptorSet;
	}

	//-------------------------------------------------------------------------------------------------

	void FlatShadingPipeline::CreatePerPipelineDescriptorSetLayout()
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings{};

		// ViewProjection
		VkDescriptorSetLayoutBinding modelViewProjectionBinding{
			.binding = static_cast<uint32_t>(bindings.size()),
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_VERTEX_BIT
		};
		bindings.emplace_back(modelViewProjectionBinding);

        // Light source
        VkDescriptorSetLayoutBinding const lightSourceBinding{
            .binding = static_cast<uint32_t>(bindings.size()),
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
        };
        bindings.emplace_back(lightSourceBinding);

		// Sampler
		VkDescriptorSetLayoutBinding const samplerLayoutBinding{
			.binding = static_cast<uint32_t>(bindings.size()),
			.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
		};
		bindings.emplace_back(samplerLayoutBinding);

		mPerPipelineDescriptorLayout = RB::CreateDescriptorSetLayout(
			LogicalDevice::Instance->GetVkDevice(),
			static_cast<uint8_t>(bindings.size()),
			bindings.data()
		);
	}

	//-------------------------------------------------------------------------------------------------

	void FlatShadingPipeline::CreatePerGeometryDescriptorSetLayout()
	{
		std::vector<VkDescriptorSetLayoutBinding> bindings{};

		// Material
		bindings.emplace_back(VkDescriptorSetLayoutBinding{
			.binding = static_cast<uint32_t>(bindings.size()),
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
		});

		// Base color
		bindings.emplace_back(VkDescriptorSetLayoutBinding{
			.binding = static_cast<uint32_t>(bindings.size()),
			.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
		});

		mPerGeometryDescriptorLayout = RB::CreateDescriptorSetLayout(
			LogicalDevice::Instance->GetVkDevice(),
			static_cast<uint8_t>(bindings.size()),
			bindings.data()
		);

	}

	//-------------------------------------------------------------------------------------------------

	void FlatShadingPipeline::CreatePipeline()
	{
		// Vertex shader
		{
			bool success = Importer::CompileShaderToSPV(
				Path::Instance->Get("engine/shaders/flat_shading_pipeline/FlatShadingPipeline.vert.hlsl"),
				Path::Instance->Get("engine/shaders/flat_shading_pipeline/FlatShadingPipeline.vert.spv"),
				"vert"
			);
			MFA_ASSERT(success == true);
		}
		auto cpuVertexShader = Importer::ShaderFromSPV(
			Path::Instance->Get("engine/shaders/flat_shading_pipeline/FlatShadingPipeline.vert.spv"),
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
				Path::Instance->Get("engine/shaders/flat_shading_pipeline/FlatShadingPipeline.frag.hlsl"),
				Path::Instance->Get("engine/shaders/flat_shading_pipeline/FlatShadingPipeline.frag.spv"),
				"frag"
			);
			MFA_ASSERT(success == true);
		}
		auto cpuFragmentShader = Importer::ShaderFromSPV(
			Path::Instance->Get("engine/shaders/flat_shading_pipeline/FlatShadingPipeline.frag.spv"),
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
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(Vertex, position),
		});
		// UV
		inputAttributeDescriptions.emplace_back(VkVertexInputAttributeDescription{
			.location = static_cast<uint32_t>(inputAttributeDescriptions.size()),
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(Vertex, baseColorUV),
		});
		// Normal
		inputAttributeDescriptions.emplace_back(VkVertexInputAttributeDescription{
			.location = static_cast<uint32_t>(inputAttributeDescriptions.size()),
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(Vertex, normal),
		});


		RB::CreateGraphicPipelineOptions pipelineOptions{};
		pipelineOptions.useStaticViewportAndScissor = false;
		pipelineOptions.primitiveTopology = _params.topology;
		// TODO I think we should submit each pipeline . Each one should have independent depth buffer 
		pipelineOptions.rasterizationSamples = LogicalDevice::Instance->GetMaxSampleCount();            // TODO Find a way to set sample count to 1. We only need MSAA for pbr-pipeline
		pipelineOptions.cullMode = _params.cullModeFlags;
		pipelineOptions.colorBlendAttachments.blendEnable = VK_TRUE;
		pipelineOptions.polygonMode = _params.polygonMode;

		// pipeline layout
		std::vector<VkPushConstantRange> const pushConstantRanges{
			VkPushConstantRange {
				.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
				.offset = 0,
				.size = sizeof(PushConstants),
			}
		};

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
			mPerPipelineDescriptorLayout->descriptorSetLayout,
			mPerGeometryDescriptorLayout->descriptorSetLayout
		};
		
		const auto pipelineLayout = RB::CreatePipelineLayout(
			LogicalDevice::Instance->GetVkDevice(),
			static_cast<uint32_t>(descriptorSetLayouts.size()),
			descriptorSetLayouts.data(),
			static_cast<uint32_t>(pushConstantRanges.size()),
			pushConstantRanges.data()
		);

		auto surfaceCapabilities = LogicalDevice::Instance->GetSurfaceCapabilities();

		mPipeline = RB::CreateGraphicPipeline(
			LogicalDevice::Instance->GetVkDevice(),
			static_cast<uint8_t>(shaders.size()),
			shaders.data(),
			1,
			&bindingDescription,
			static_cast<uint8_t>(inputAttributeDescriptions.size()),
			inputAttributeDescriptions.data(),
			surfaceCapabilities.currentExtent,
			mDisplayRenderPass->GetVkRenderPass(),
			pipelineLayout,
			pipelineOptions
		);
	}

	//-------------------------------------------------------------------------------------------------

	void FlatShadingPipeline::CreatePerPipelineDescriptorSets()
	{
		auto const maxFramesPerFlight = LogicalDevice::Instance->GetMaxFramePerFlight();
		mPerPipelineDescriptorSetGroup = RB::CreateDescriptorSet(
			LogicalDevice::Instance->GetVkDevice(),
			mDescriptorPool->descriptorPool,
			mPerPipelineDescriptorLayout->descriptorSetLayout,
			maxFramesPerFlight
		);

		for (uint32_t frameIndex = 0; frameIndex < maxFramesPerFlight; ++frameIndex)
		{

			auto const& descriptorSet = mPerPipelineDescriptorSetGroup.descriptorSets[frameIndex];
			MFA_ASSERT(descriptorSet != VK_NULL_HANDLE);

			DescriptorSetSchema descriptorSetSchema{ descriptorSet };

			/////////////////////////////////////////////////////////////////
			// Vertex shader
			/////////////////////////////////////////////////////////////////

			// ViewProjectionTransform
			VkDescriptorBufferInfo viewProjBufferInfo{
				.buffer = mViewProjBuffer->buffers[frameIndex]->buffer,
				.offset = 0,
				.range = mViewProjBuffer->bufferSize,
			};
			descriptorSetSchema.AddUniformBuffer(&viewProjBufferInfo);

			// LightSourceBuffer
			VkDescriptorBufferInfo lightSourceBufferInfo{
				.buffer = mLightSourceBuffer->buffers[frameIndex]->buffer,
				.offset = 0,
				.range = mLightSourceBuffer->bufferSize
			};
			descriptorSetSchema.AddUniformBuffer(&lightSourceBufferInfo);

			// Sampler
			VkDescriptorImageInfo texturesSamplerInfo{
				.sampler = mSampler->sampler,
				.imageView = VK_NULL_HANDLE,
				.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
			};
			descriptorSetSchema.AddSampler(&texturesSamplerInfo);

			descriptorSetSchema.UpdateDescriptorSets();
		}
	}

	//-------------------------------------------------------------------------------------------------

	void FlatShadingPipeline::reload()
	{
		MFA_LOG_DEBUG("Reloading shading pipeline");

		LogicalDevice::Instance->DeviceWaitIdle();
		CreatePipeline();
	}

	//-------------------------------------------------------------------------------------------------

}
