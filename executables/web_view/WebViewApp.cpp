#include "WebViewApp.hpp"

#include "LogicalDevice.hpp"
#include "WebViewContainer.hpp"

using namespace MFA;

//=============================================================

void WebViewApp::Run()
{
    auto* device = LogicalDevice::Instance;

    device->SDL_EventSignal.Register([&](SDL_Event* event)->void
    {
        // TODO: For webview keys
    });

    auto const swapChainResource = std::make_shared<SwapChainRenderResource>();
    auto const depthResource = std::make_shared<DepthRenderResource>();
    auto const msaaResource = std::make_shared<MSSAA_RenderResource>();
    _displayRenderPass = std::make_shared<DisplayRenderPass>(
        swapChainResource,
        depthResource,
        msaaResource
    );

    device->ResizeEventSignal2.Register([this]()->void {
        Resize();
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

    auto const pipeline = std::make_shared<TextOverlayPipeline>(_displayRenderPass, fontSampler);
    _fontRenderer = std::make_shared<ConsolasFontRenderer>(pipeline);
    _webViewContainer = std::make_unique<WebViewContainer>(_fontRenderer);

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
            device->SubmitQueues(recordState);
            device->Present(recordState, swapChainResource->GetSwapChainImages().swapChain);
        }

        time->Update();
    }

    time.reset();

    device->DeviceWaitIdle();
}

//=============================================================

void WebViewApp::Update(float deltaTime)
{
    _webViewContainer->Update();
}

//=============================================================

void WebViewApp::Render(MFA::RT::CommandRecordState & recordState)
{
    auto* device = LogicalDevice::Instance;
    device->BeginCommandBuffer(
        recordState,
        RT::CommandBufferType::Compute
    );
    device->EndCommandBuffer(recordState);

    device->BeginCommandBuffer(
        recordState,
        RT::CommandBufferType::Graphic
    );

    _webViewContainer->UpdateBuffers(recordState);

    _displayRenderPass->Begin(recordState);
    _webViewContainer->DisplayPass(recordState);
    _displayRenderPass->End(recordState);

    device->EndCommandBuffer(recordState);
}

//=============================================================

void WebViewApp::Resize()
{

}

//=============================================================
