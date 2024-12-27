#include "WebViewApp.hpp"

#include "BedrockFile.hpp"
#include "BedrockPath.hpp"
#include "ImportTexture.hpp"
#include "LogicalDevice.hpp"
#include "Time.hpp"
#include "WebViewContainer.hpp"


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
        InstantiateWebViewContainer();
        //Resize();
    });

    {// Font
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
        _fontPipeline = std::make_shared<TextOverlayPipeline>(_displayRenderPass, fontSampler);
        AddFont(
            "JetBrainsMono",
            Path::Instance()->Get("fonts/JetBrains-Mono/JetBrainsMono-Bold.ttf").c_str()
        );
        AddFont(
            "PublicSans",
            Path::Instance()->Get("fonts/PublicSans/PublicSans-Bold.ttf").c_str()
        );
    }

    {// Solid fill
        auto const solidFillPipeline = std::make_shared<SolidFillPipeline>(_displayRenderPass);
        _solidFillRenderer = std::make_shared<SolidFillRenderer>(solidFillPipeline);
    }

    {// Image
        auto const imageSampler = RB::CreateSampler(device->GetVkDevice(), {});
        auto const imagePipeline = std::make_shared<ImagePipeline>(_displayRenderPass, imageSampler);
        _imageRenderer = std::make_shared<ImageRenderer>(imagePipeline);
    }

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

void WebViewApp::Render(RT::CommandRecordState & recordState)
{
    auto* device = LogicalDevice::Instance;
    // device->BeginCommandBuffer(
    //     recordState,
    //     RT::CommandBufferType::Compute
    // );
    // device->EndCommandBuffer(recordState);

    device->BeginCommandBuffer(
        recordState,
        RT::CommandBufferType::Graphic
    );

    _webViewContainer->UpdateBuffer(recordState);
    _displayRenderPass->Begin(recordState);
    _webViewContainer->DisplayPass(recordState);
    _displayRenderPass->End(recordState);

    device->EndCommandBuffer(recordState);
}

//=============================================================

void WebViewApp::Resize()
{
    auto const * device = LogicalDevice::Instance;

    litehtml::position clip;
    clip.x = 0;
    clip.y = 0;
    clip.width = device->GetWindowWidth();
    clip.height = device->GetWindowHeight();

    _webViewContainer->OnResize(clip);
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
        else if (event->key.keysym.sym == SDLK_DOWN)
        {
            SetSelectedButton(_selectedButtonIdx + 1);
        }
        else if (event->key.keysym.sym == SDLK_UP)
        {
            SetSelectedButton(_selectedButtonIdx - 1);
        }
    }
}

//=============================================================

void WebViewApp::Reload()
{
    // TODO: Reload shaders too
    auto const * device = LogicalDevice::Instance;

    RB::DeviceWaitIdle(device->GetVkDevice());

    litehtml::position clip;
    clip.x = 0;
    clip.y = 0;
    clip.width = device->GetWindowWidth();
    clip.height = device->GetWindowHeight();

    _webViewContainer->OnReload(clip);

    QueryButtons();
}

//=============================================================

void WebViewApp::InstantiateWebViewContainer()
{
    auto const path = Path::Instance();

    auto const * device = LogicalDevice::Instance;

	auto const htmlPath = path->Get("web_view/Test.html");
	
    litehtml::position clip;
    clip.x = 0;
    clip.y = 0;
    clip.width = device->GetWindowWidth();
    clip.height = device->GetWindowHeight();

    WebViewContainer::Params params
    {
        .solidFillRenderer = _solidFillRenderer,
        .imageRenderer = _imageRenderer,
        .requestBlob = [this](char const *address, bool force) { return RequestBlob(address, force); },
        .requestFont = [this](char const * font) { return RequestFont(font); },
        .requestImage = [this](char const * image) {return RequestImage(image);}
    };

    _webViewContainer = std::make_unique<WebViewContainer>(htmlPath.c_str(), clip, params);

    QueryButtons();
}

//=============================================================

void WebViewApp::QueryButtons()
{
    _buttons.clear();
    _buttons.emplace_back(_webViewContainer->GetElementById("new-game"));
    _buttons.emplace_back(_webViewContainer->GetElementById("continue"));
    _buttons.emplace_back(_webViewContainer->GetElementById("settings"));
    _buttons.emplace_back(_webViewContainer->GetElementById("exit"));

    SetSelectedButton(0);
}

//=============================================================

void WebViewApp::SetSelectedButton(int const idx)
{
    static constexpr char const * SelectedKeyword = " selected";
    static const size_t SelectedKeywordSize = strlen(SelectedKeyword);
    _selectedButtonIdx = (idx + _buttons.size()) % _buttons.size();
    for (int i = 0; i < _buttons.size(); ++i)
    {
        auto const & button = _buttons[i];
        std::string classAttr = button->get_attr("class");
        if (i == _selectedButtonIdx)
        {
            classAttr = classAttr.append(SelectedKeyword);
        }
        else
        {
            auto const findResult = classAttr.find(SelectedKeyword);
            if (findResult != std::string::npos)
            {
                classAttr.erase(findResult, SelectedKeywordSize);
            }
        }
        button->set_attr("class", classAttr.c_str());
        _webViewContainer->InvalidateStyles(button);
    }
}

//=============================================================

void WebViewApp::InitFontPipeline()
{
}

//=============================================================

void WebViewApp::AddFont(char const *name, char const *path)
{
    MFA_ASSERT(_fontMap.contains(name) == false);
    MFA_ASSERT(std::filesystem::exists(path) == true);
    auto const fontData = File::Read(path);
    _fontMap[name] = std::make_shared<CustomFontRenderer>(
        _fontPipeline,
        Alias{fontData->Ptr(), fontData->Len()},
        200.0f
    );
}

// TODO: We have to reuse stuff here instead
//=============================================================

std::shared_ptr<Blob> WebViewApp::RequestBlob(char const *address, bool const ignoreCache)
{
    if (ignoreCache == false)
    {
        auto const findResult = _blobMap.find(address);
        if (findResult != _blobMap.end())
        {
            return findResult->second;
        }
    }
    auto const blob = File::Read(address);
    _blobMap[address] = blob;
    return blob;
}

//=============================================================

std::shared_ptr<CustomFontRenderer> WebViewApp::RequestFont(char const *font)
{
    auto const findResult = _fontMap.find(font);
    if (findResult != _fontMap.end())
    {
        return findResult->second;
    }
    MFA_LOG_WARN("Failed to find font with name %s", font);
    return _fontMap.begin()->second;
}

//=============================================================

std::shared_ptr<RT::GpuTexture> WebViewApp::RequestImage(char const *imageName)
{
    auto const findResult = _imageMap.find(imageName);
    if (findResult != _imageMap.end())
    {
        return findResult->second;
    }
    // In this case we want to render a 3d scene as well.
    if (strncmp(imageName, "scene", strlen("scene")) == 0)
    {
        MFA_LOG_ERROR("Not implemented yet!");
    }
    else
    {
        auto * device = LogicalDevice::Instance;

        auto const path = Path::Instance()->Get(imageName);

        auto const commandBuffer = RB::BeginSingleTimeCommand(device->GetVkDevice(), device->GetGraphicCommandPool());

        auto const cpuTexture = Importer::UncompressedImage(path);
        
        auto [gpuTexture, stageBuffer] = RB::CreateTexture(
            *cpuTexture,
            device->GetVkDevice(),
            device->GetPhysicalDevice(),
            commandBuffer
        );

        RB::EndAndSubmitSingleTimeCommand(
            device->GetVkDevice(),
            device->GetGraphicCommandPool(),
            device->GetGraphicQueue(),
            commandBuffer
        );

        _imageMap[imageName] = gpuTexture;

        return gpuTexture;
    }

    return nullptr;
}

//=============================================================
