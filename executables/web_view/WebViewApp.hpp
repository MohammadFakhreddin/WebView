#pragma once

#include "RenderTypes.hpp"

#include "render_pass/DisplayRenderPass.hpp"
#include "render_resource/DepthRenderResource.hpp"
#include "render_resource/MSAA_RenderResource.hpp"
#include "render_resource/SwapChainRenderResource.hpp"
#include "utils/ConsolasFontRenderer.hpp"
#include "Time.hpp"
#include "WebViewContainer.hpp"

class WebViewApp
{
public:

	void Run();

private:

	void Update(float deltaTime);

	void Render(MFA::RT::CommandRecordState & recordState);

	void Resize();

	std::shared_ptr<MFA::DisplayRenderPass> _displayRenderPass;
	std::shared_ptr<MFA::ConsolasFontRenderer> _fontRenderer;
	std::unique_ptr<WebViewContainer> _webViewContainer;

};
