#pragma once

#include "RenderTypes.hpp"

#include "render_pass/DisplayRenderPass.hpp"
#include "utils/ConsolasFontRenderer.hpp"
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

	void SetSelectedButton(int idx);

    void InitFontPipeline();

    void AddFont(char const *name, char const *path);

    [[nodiscard]]
    std::shared_ptr<MFA::Blob> RequestBlob(char const *address, bool force);

    [[nodiscard]]
    std::shared_ptr<MFA::CustomFontRenderer> RequestFont(char const *font);

	std::shared_ptr<MFA::DisplayRenderPass> _displayRenderPass;
	std::unique_ptr<WebViewContainer> _webViewContainer;

	std::shared_ptr<MFA::SolidFillRenderer> _solidFillRenderer;

	std::vector<litehtml::element::ptr> _buttons{};
	int _selectedButtonIdx = 0;

    std::shared_ptr<MFA::TextOverlayPipeline> _fontPipeline{};
    std::unordered_map<std::string, std::shared_ptr<MFA::CustomFontRenderer>> _fontMap{};

};
