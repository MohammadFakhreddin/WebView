#include "WebViewApp.hpp"

#include "LogicalDevice.hpp"
#include "Time.hpp"
#include "WebViewContainer.hpp"
#include "BedrockFile.hpp"
#include "BedrockPath.hpp"

using namespace MFA;

//=============================================================

void WebViewApp::Run()
{
    auto* device = LogicalDevice::Instance;

    device->SDL_EventSignal.Register([this](SDL_Event* event)->void{OnSDL_Event(event);});

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

    auto const fontPipeline = std::make_shared<TextOverlayPipeline>(_displayRenderPass, fontSampler);
    _fontRenderer = std::make_shared<ConsolasFontRenderer>(fontPipeline);

    auto const linePipeline = std::make_shared<LinePipeline>(_displayRenderPass, 1e4);
    _lineRenderer = std::make_shared<LineRenderer>(linePipeline);

    auto const solidFillPipeline = std::make_shared<SolidFillPipeline>(_displayRenderPass);
    _solidFillRenderer = std::make_shared<SolidFillRenderer>(solidFillPipeline);

    InstantiateWebViewContainer();

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
    Reload();
}

//=============================================================

void WebViewApp::OnSDL_Event(SDL_Event* event)
{
    if (event->type == SDL_KEYDOWN)
    {
        if (event->key.keysym.sym == SDLK_F5) 
        {
            Reload();
        }
    }
}

//=============================================================

void WebViewApp::Reload()
{
    // TODO: Reload shaders too
    RB::DeviceWaitIdle(LogicalDevice::Instance->GetVkDevice());
    InstantiateWebViewContainer();
}

//=============================================================

void WebViewApp::InstantiateWebViewContainer()
{
    auto * path = Path::Instance;
	auto htmlPath = path->Get("web_view/Test.html");
	auto const htmlBlob = File::Read(htmlPath);    
    _webViewContainer = std::make_unique<WebViewContainer>(htmlBlob, _lineRenderer, _fontRenderer, _solidFillRenderer);
}

//=============================================================
