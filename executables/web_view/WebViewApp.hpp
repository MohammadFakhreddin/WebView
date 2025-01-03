#pragma once

#include "RenderTypes.hpp"
#include "render_pass/DisplayRenderPass.hpp"
#include "WebViewContainer.hpp"

class WebViewApp
{
public:

	void Run();

private:

	void Update(float deltaTime);

	void Render(MFA::RT::CommandRecordState & recordState);

	void Resize();

	void OnSDL_Event(SDL_Event* event);

	void Reload();

	void InstantiateWebViewContainer();

    void QueryButtons();

	void SetSelectedElement(int idx);

    void ModifySelectedElement(int value);

    void InitFontPipeline();

    void AddFont(char const *name, char const *path);

    [[nodiscard]]
    std::shared_ptr<MFA::Blob> RequestBlob(char const *address, bool ignoreCache);

    [[nodiscard]]
    std::shared_ptr<MFA::CustomFontRenderer> RequestFont(char const *font);

    [[nodiscard]]
    std::tuple<std::shared_ptr<MFA::RT::GpuTexture>, glm::vec2> RequestImage(char const *imageName);

	std::shared_ptr<MFA::DisplayRenderPass> _displayRenderPass;
	std::unique_ptr<WebViewContainer> _webViewContainer;

	std::shared_ptr<MFA::SolidFillRenderer> _solidFillRenderer;
    std::shared_ptr<MFA::BorderRenderer> _borderRenderer;
    std::shared_ptr<MFA::ImageRenderer> _imageRenderer;
    std::shared_ptr<MFA::TextOverlayPipeline> _fontPipeline;

	std::vector<litehtml::element::ptr> _elements{};
    enum class ElementType
    {
        Button,
        Slider,
        Checkbox
    };
    std::vector<ElementType> _elementsType{};
	int _selectedElementIndex = 0;

    std::unordered_map<std::string, std::shared_ptr<MFA::Blob>> _blobMap;
    std::unordered_map<std::string, std::shared_ptr<MFA::CustomFontRenderer>> _fontMap{};
    std::unordered_map<std::string, std::tuple<std::shared_ptr<MFA::RT::GpuTexture>, glm::vec2>> _imageMap;

};
