#include "BedrockPath.hpp"
#include "BedrockAssert.hpp"
#include "LogicalDevice.hpp"
#include "render_pass/DisplayRenderPass.hpp"
#include "render_resource/DepthRenderResource.hpp"
#include "render_resource/MSAA_RenderResource.hpp"
#include "render_resource/SwapChainRenderResource.hpp"
#include "utils/ConsolasFontRenderer.hpp"
#include "Time.hpp"

using namespace MFA;

int main()
{
    auto path = Path::Instantiate();
	
	LogicalDevice::InitParams params
	{
		.windowWidth = 1000,
		.windowHeight = 1000,
		.resizable = true,
		.fullScreen = false,
		.applicationName = "Mesh-Viewer-App"
	};

    auto const device = LogicalDevice::Instantiate(params);
	MFA_ASSERT(device->IsValid() == true);

    {
        device->SDL_EventSignal.Register([&](SDL_Event* event)->void
        {
            // TODO: For webview keys
        });

        auto const swapChainResource = std::make_shared<SwapChainRenderResource>();
        auto const depthResource = std::make_shared<DepthRenderResource>();
        auto const msaaResource = std::make_shared<MSSAA_RenderResource>();
        auto const displayRenderPass = std::make_shared<DisplayRenderPass>(
            swapChainResource,
            depthResource,
            msaaResource
        );
        
        device->ResizeEventSignal2.Register([]()->void {
            // TODO
        });

        RB::CreateSamplerParams fontSamplerParams{};
        fontSamplerParams.magFilter = VK_FILTER_LINEAR;
        fontSamplerParams.minFilter = VK_FILTER_LINEAR;
        fontSamplerParams.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        fontSamplerParams.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        fontSamplerParams.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        fontSamplerParams.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        fontSamplerParams.mipLodBias = 0.0f;
        fontSamplerParams.compareOp = VK_COMPARE_OP_NEVER;
        fontSamplerParams.minLod = 0.0f;
        fontSamplerParams.maxLod = 1.0f;
        fontSamplerParams.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;

        auto const fontSampler = RB::CreateSampler(
            device->GetVkDevice(),
            fontSamplerParams
        );
        MFA_ASSERT(fontSampler != nullptr);

        auto const pipeline = std::make_shared<TextOverlayPipeline>(displayRenderPass, fontSampler);
        auto const fontRenderer = std::make_unique<ConsolasFontRenderer>(pipeline);
        auto const textData = fontRenderer->AllocateTextData();

        SDL_GL_SetSwapInterval(0);
        SDL_Event e;
        
        auto time = Time::Instantiate(120, 30);
        
        bool shouldQuit = false;

        while (shouldQuit == false)
        {
            //Handle events
            while (SDL_PollEvent(&e) != 0)
            {
                //User requests quit
                if (e.type == SDL_QUIT)
                {
                    shouldQuit = true;
                }
            }

            device->Update();
            Update(Time::DeltaTimeSec());

            auto recordState = device->AcquireRecordState(swapChainResource->GetSwapChainImages().swapChain);
            if (recordState.isValid == true)
            {
                Render(recordState);
            }

            time->Update();
        }

        time.reset();

        device->DeviceWaitIdle();
    }
}