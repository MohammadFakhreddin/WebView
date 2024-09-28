#include "RenderTypes.hpp"

#include "BedrockAssert.hpp"
#include "RenderBackend.hpp"
#include "LogicalDevice.hpp"

#include <utility>

namespace MFA::RenderTypes
{
	//-------------------------------------------------------------------------------------------------

	ImageGroup::ImageGroup(
		VkImage image_,
		VkDeviceMemory memory_
	)
		: image(image_)
		, memory(memory_)
	{}

	//-------------------------------------------------------------------------------------------------

	ImageGroup::~ImageGroup()
	{
		RB::DestroyImage(LogicalDevice::Instance->GetVkDevice(), *this);
	}

	//-------------------------------------------------------------------------------------------------

	ImageViewGroup::ImageViewGroup(VkImageView imageView_)
		: imageView(imageView_)
	{
		MFA_ASSERT(imageView != VK_NULL_HANDLE);
	}

	//-------------------------------------------------------------------------------------------------

	ImageViewGroup::~ImageViewGroup()
	{
		RB::DestroyImageView(LogicalDevice::Instance->GetVkDevice(), *this);
	}

	//-------------------------------------------------------------------------------------------------

	DepthImageGroup::DepthImageGroup(
		std::shared_ptr<ImageGroup> imageGroup_,
		std::shared_ptr<ImageViewGroup> imageView_,
		VkFormat imageFormat_
	)
		: imageGroup(std::move(imageGroup_))
		, imageView(std::move(imageView_))
		, imageFormat(imageFormat_)
	{}

	//-------------------------------------------------------------------------------------------------

	DepthImageGroup::~DepthImageGroup() = default;

	//-------------------------------------------------------------------------------------------------

	DescriptorSetLayoutGroup::DescriptorSetLayoutGroup(VkDescriptorSetLayout descriptorSetLayout_)
		: descriptorSetLayout(descriptorSetLayout_)
	{
		MFA_ASSERT(descriptorSetLayout != VK_NULL_HANDLE);
	}

	//-------------------------------------------------------------------------------------------------

	DescriptorSetLayoutGroup::~DescriptorSetLayoutGroup()
	{
		RB::DestroyDescriptorSetLayout(
			LogicalDevice::Instance->GetVkDevice(), 
			descriptorSetLayout
		);
	}

	//-------------------------------------------------------------------------------------------------

	PipelineGroup::PipelineGroup(
		VkPipelineLayout pipelineLayout_,
		VkPipeline pipeline_
	)
		: pipelineLayout(pipelineLayout_)
		, pipeline(pipeline_)
	{
		MFA_ASSERT(IsValid());
	}

	//-------------------------------------------------------------------------------------------------

	PipelineGroup::~PipelineGroup()
	{
		RB::DestroyPipeline(LogicalDevice::Instance->GetVkDevice(), *this);
	}

	//-------------------------------------------------------------------------------------------------

	bool PipelineGroup::IsValid() const noexcept
	{
		return pipelineLayout != VK_NULL_HANDLE && pipeline != VK_NULL_HANDLE;
	}

	//-------------------------------------------------------------------------------------------------

	SwapChainGroup::SwapChainGroup(
		VkSwapchainKHR swapChain_,
		VkFormat swapChainFormat_,
		std::vector<VkImage> swapChainImages_,
		std::vector<std::shared_ptr<ImageViewGroup>> swapChainImageViews_
	)
		: swapChain(swapChain_)
		, swapChainFormat(swapChainFormat_)
		, swapChainImages(std::move(swapChainImages_))
		, swapChainImageViews(std::move(swapChainImageViews_))
	{}

	//-------------------------------------------------------------------------------------------------

	SwapChainGroup::~SwapChainGroup()
	{
		RB::DestroySwapChain(LogicalDevice::Instance->GetVkDevice(), *this);
	}

	//-------------------------------------------------------------------------------------------------

	ColorImageGroup::ColorImageGroup(std::shared_ptr<ImageGroup> 
		imageGroup_,
		std::shared_ptr<ImageViewGroup> imageView_, 
		VkFormat imageFormat_
	)
		: imageGroup(std::move(imageGroup_))
		, imageView(std::move(imageView_))
		, imageFormat(imageFormat_)
	{
	}

	//-------------------------------------------------------------------------------------------------

	ColorImageGroup::~ColorImageGroup() = default;

	//-------------------------------------------------------------------------------------------------

	RenderPass::RenderPass(VkRenderPass renderPass_)
		: vkRenderPass(renderPass_)
	{
	}

	//-------------------------------------------------------------------------------------------------

	RenderPass::~RenderPass()
	{
		RB::DestroyRenderPass(LogicalDevice::Instance->GetVkDevice(), vkRenderPass);
	}

	//-------------------------------------------------------------------------------------------------

	FrameBuffer::FrameBuffer(VkFramebuffer framebuffer_)
		: framebuffer(framebuffer_)
	{
	}

	//-------------------------------------------------------------------------------------------------

	FrameBuffer::~FrameBuffer()
	{
		RB::DestroyFrameBuffer(LogicalDevice::Instance->GetVkDevice(), framebuffer);
	}

	//-------------------------------------------------------------------------------------------------

	GpuShader::GpuShader(
		VkShaderModule const& shaderModule, 
		VkShaderStageFlagBits const stageFlags,
		std::string entryPointName
	)
		: shaderModule(shaderModule)
		, stageFlags(stageFlags)
		, entryPointName(std::move(entryPointName))
	{
		MFA_ASSERT(shaderModule != VK_NULL_HANDLE);
	}

	//-------------------------------------------------------------------------------------------------

	GpuShader::~GpuShader()
	{
		RB::DestroyShader(LogicalDevice::Instance->GetVkDevice(), *this);
	}

	//-------------------------------------------------------------------------------------------------

	DescriptorPool::DescriptorPool(VkDescriptorPool descriptorPool_)
		: descriptorPool(descriptorPool_)
	{}

	//-------------------------------------------------------------------------------------------------

	DescriptorPool::~DescriptorPool()
	{
		RB::DestroyDescriptorPool(LogicalDevice::Instance->GetVkDevice(), descriptorPool);
	}

	//-------------------------------------------------------------------------------------------------

	BufferAndMemory::BufferAndMemory(VkBuffer buffer_, VkDeviceMemory memory_, VkDeviceSize size_)
		: buffer(buffer_)
		, memory(memory_)
		, size(size_)
	{
	}

	//-------------------------------------------------------------------------------------------------

	BufferAndMemory::~BufferAndMemory()
	{
		RB::DestroyBuffer(LogicalDevice::Instance->GetVkDevice(), *this);
	}

	//-------------------------------------------------------------------------------------------------

	BufferGroup::BufferGroup(
		std::vector<std::shared_ptr<BufferAndMemory>> buffers_,
		size_t bufferSize_,
		VkMemoryPropertyFlags memoryPropertyFlags_,
		VkBufferUsageFlags bufferUsageFlags_
	)
		: buffers(std::move(buffers_))
		, bufferSize(bufferSize_)
		, memoryPropertyFlags(memoryPropertyFlags_)
		, bufferUsageFlags(bufferUsageFlags_)
	{}

	//-------------------------------------------------------------------------------------------------

	BufferGroup::~BufferGroup() = default;

	//-------------------------------------------------------------------------------------------------

	SamplerGroup::SamplerGroup(VkSampler sampler_)
		: sampler(sampler_)
	{}

	//-------------------------------------------------------------------------------------------------

	SamplerGroup::~SamplerGroup()
	{
		RB::DestroySampler(LogicalDevice::Instance->GetVkDevice(), *this);
	}

	//-------------------------------------------------------------------------------------------------

	GpuTexture::GpuTexture() = default;

	//-------------------------------------------------------------------------------------------------

	GpuTexture::GpuTexture(
		std::shared_ptr<ImageGroup> imageGroup, 
		std::shared_ptr<ImageViewGroup> imageView
	)
		: imageGroup(std::move(imageGroup))
		, imageView(std::move(imageView))
	{}

	//-------------------------------------------------------------------------------------------------

	GpuTexture::~GpuTexture() = default;

	//-------------------------------------------------------------------------------------------------

}
