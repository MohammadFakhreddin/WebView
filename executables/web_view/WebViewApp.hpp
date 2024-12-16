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

	void SetSelectedButton(int idx);

	std::shared_ptr<MFA::DisplayRenderPass> _displayRenderPass;
	std::unique_ptr<WebViewContainer> _webViewContainer;

	std::shared_ptr<MFA::ConsolasFontRenderer> _fontRenderer;
	std::shared_ptr<MFA::SolidFillRenderer> _solidFillRenderer;

	std::vector<litehtml::element::ptr> _buttons{};
	int _selectedButtonIdx = 0;

};
