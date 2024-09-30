#include "WebViewContainer.hpp"

#include "BedrockFile.hpp"
#include "BedrockPath.hpp"
#include "ImportGLTF.hpp"
#include "LogicalDevice.hpp"
#include "RenderBackend.hpp"

#include <ranges>

//=========================================================================================

WebViewContainer::WebViewContainer(
	std::shared_ptr<LineRenderer> lineRenderer,
	std::shared_ptr<FontRenderer> fontRenderer
)
	: litehtml::document_container()
	, _fontRenderer(std::move(fontRenderer))
	, _lineRenderer(std::move(lineRenderer))
{
	auto * path = MFA::Path::Instance;
	auto htmlPath = path->Get("web_view/Test.html");
	set_base_url(htmlPath.c_str());
	auto const blob = MFA::File::Read(htmlPath);
	char const * htmlText = blob->As<char const>();
	_html = litehtml::document::createFromString(
		htmlText, 
		this
	);
	MFA_ASSERT(_html != nullptr);

	auto* device = MFA::LogicalDevice::Instance;

	litehtml::position clip{};
	clip.width = device->GetWindowWidth();
	clip.height = device->GetWindowHeight();
	clip.x = 0;
	clip.y = 0;

	_html->render(clip.width);
	_html->draw(0, 0, 0, &clip);
}

//=========================================================================================

WebViewContainer::~WebViewContainer() = default;

//=========================================================================================

void WebViewContainer::Update()
{
}

//=========================================================================================

void WebViewContainer::UpdateBuffers(const MFA::RT::CommandRecordState& recordState)
{
	for (auto& textData : _textDataMap | std::views::values)
	{
		textData->vertexData->Update(recordState);
	}
}

//=========================================================================================

void WebViewContainer::DisplayPass(MFA::RT::CommandRecordState& recordState)
{
	for (auto& textData : _textDataMap | std::views::values)
	{
		_fontRenderer->Draw(recordState, *textData);
	}
	for (auto & drawCalls : _drawCallsMap | std::views::values)
	{
		for (auto & drawCall : drawCalls)
		{
			drawCall(recordState);
		}
	}
}

//=========================================================================================

litehtml::element::ptr WebViewContainer::create_element(
	const char* tag_name, 
	const litehtml::string_map& attributes,
	const std::shared_ptr<litehtml::document>& doc
)
{
	return nullptr;
}

//=========================================================================================

litehtml::uint_ptr WebViewContainer::create_font(
	const char* faceName, 
	int size, 
	int weight,
	litehtml::font_style italic, 
	unsigned int decoration, 
	litehtml::font_metrics* fm
)
{
	fm->height = static_cast<int>(_fontRenderer->TextHeight() * MFA::LogicalDevice::Instance->GetWindowHeight());
	fm->draw_spaces = false;
	return 1;
}

//=========================================================================================

void WebViewContainer::del_clip()
{
	// TODO:
}

//=========================================================================================

void WebViewContainer::delete_font(litehtml::uint_ptr hFont)
{
	// TODO
}

//=========================================================================================

void WebViewContainer::draw_borders(
	litehtml::uint_ptr hdc, 
	const litehtml::borders& borders,
	const litehtml::position& draw_pos, 
	bool root
)
{
	_drawCallsMap[hdc].emplace_back([this, draw_pos, borders](MFA::RT::CommandRecordState& recordState)->void
	{
		float windowWidth = MFA::LogicalDevice::Instance->GetWindowWidth();
		float windowHeight = MFA::LogicalDevice::Instance->GetWindowHeight();

		glm::vec3 topLeft {
			static_cast<float>(draw_pos.x) / windowWidth,
			static_cast<float>(draw_pos.y) / windowHeight,
			0.0f
		};
		glm::vec3 bottomRight = topLeft + glm::vec3 {windowWidth, windowHeight, 0.0f};
		glm::vec3 topRight = topLeft + glm::vec3{ windowWidth, 0.0f, 0.0f };
		glm::vec3 bottomLeft = topLeft + glm::vec3{ 0.0f, windowHeight, 0.0f };

		_lineRenderer->Draw(recordState, topLeft, topRight, glm::vec4{ ConvertColor(borders.top.color) , 1.0f });
		_lineRenderer->Draw(recordState, topLeft, bottomLeft, glm::vec4{ ConvertColor(borders.left.color) , 1.0f });
		_lineRenderer->Draw(recordState, bottomLeft, bottomRight, glm::vec4{ ConvertColor(borders.bottom.color) , 1.0f });
		_lineRenderer->Draw(recordState, topRight, bottomRight, glm::vec4{ ConvertColor(borders.right.color) , 1.0f});
	});
	
	// TODO, Maybe we can use the line renderer here
}

//=========================================================================================

void WebViewContainer::draw_conic_gradient(
	litehtml::uint_ptr hdc, 
	const litehtml::background_layer& layer,
	const litehtml::background_layer::conic_gradient& gradient
)
{
	int here;
}

//=========================================================================================

void WebViewContainer::draw_image(
	litehtml::uint_ptr hdc, 
	const litehtml::background_layer& layer,
	const std::string& url, 
	const std::string& base_url
)
{
	int here;
}

//=========================================================================================

void WebViewContainer::draw_linear_gradient(
	litehtml::uint_ptr hdc, 
	const litehtml::background_layer& layer,
	const litehtml::background_layer::linear_gradient& gradient
)
{
	int here;
}

//=========================================================================================

void WebViewContainer::draw_list_marker(litehtml::uint_ptr hdc, const litehtml::list_marker& marker)
{
	int here;
}

//=========================================================================================

void WebViewContainer::draw_radial_gradient(
	litehtml::uint_ptr hdc, 
	const litehtml::background_layer& layer,
	const litehtml::background_layer::radial_gradient& gradient
)
{
	int here;
}

//=========================================================================================

void WebViewContainer::draw_solid_fill(
	litehtml::uint_ptr hdc, 
	const litehtml::background_layer& layer,
	const litehtml::web_color& color
)
{
	//int here;

	//layer.
	//_drawCallsMap[hdc].emplace_back([this, layer, color](MFA::RT::CommandRecordState & recordState)->void
	//{
		// TODO: We need a solid color pipline to draw color
		//VkViewport viewport = {};
		//viewport.x = layer.border_box.x;
		//viewport.y = layer.border_box.y;
		//viewport.width = static_cast<float>(layer.border_box.width);
		//viewport.height = static_cast<float>(layer.border_box.height);
		//viewport.minDepth = 0.0f;
		//viewport.maxDepth = 1.0f;

		//VkRect2D scissor = {};
		//scissor.offset = { layer.border_box.x, layer.border_box.y };
		//scissor.extent = VkExtent2D{ (uint32_t)layer.border_box.width, (uint32_t)layer.border_box.height };

		//MFA::RB::SetViewport(recordState.commandBuffer, viewport);
		//MFA::RB::SetScissor(recordState.commandBuffer, scissor);

		//auto myColor = ConvertColor(color);

		//std::vector<VkClearValue> clearValues(3);
		//clearValues[0].color = VkClearColorValue{.float32 = {myColor.x, myColor.y, myColor.z, 1.0f}};

		//vkCmdClearAttachments(recordState.commandBuffer,
		//	recordState.frameIndex, // first attachment index
		//	1, // number of attachments to clear
		//	clearValues.data(),
		//	nullptr); // pObjectRects

	//});

}

//=========================================================================================

void WebViewContainer::draw_text(
	litehtml::uint_ptr hdc, 
	const char* text, 
	litehtml::uint_ptr hFont,
	litehtml::web_color color, 
	const litehtml::position& pos
)
{
	auto & textData = _textDataMap[hdc];
	if (textData == nullptr)
	{
		textData = _fontRenderer->AllocateTextData();
	}
	FontRenderer::TextParams textParams{};
	// TODO: Font size and style and other things
	// TODO: Alpha
	textParams.color = ConvertColor(color);

	textParams.hTextAlign = MFA::ConsolasFontRenderer::HorizontalTextAlign::Left;
	
	float x = pos.x;
	float y = pos.y;

	_fontRenderer->AddText(*textData, text, x, y, textParams);
}

//=========================================================================================

void WebViewContainer::get_client_rect(litehtml::position& client) const
{
	auto* device = MFA::LogicalDevice::Instance;
	client.width = device->GetWindowWidth();
	client.height = device->GetWindowHeight();
	client.x = 0;
	client.y = 0;
}

//=========================================================================================

const char* WebViewContainer::get_default_font_name() const
{
	// TODO
	return "consolos";
}

//=========================================================================================

int WebViewContainer::get_default_font_size() const
{
	// TODO
	return 14;
}

//=========================================================================================

void WebViewContainer::get_image_size(const char* src, const char* baseurl, litehtml::size& sz)
{
}

//=========================================================================================

void WebViewContainer::get_language(litehtml::string& language, litehtml::string& culture) const
{
}

//=========================================================================================

void WebViewContainer::get_media_features(litehtml::media_features& media) const
{
}

//=========================================================================================

void WebViewContainer::import_css(litehtml::string& text, const litehtml::string& url, litehtml::string& baseurl)
{
}

//=========================================================================================

void WebViewContainer::link(const std::shared_ptr<litehtml::document>& doc, const litehtml::element::ptr& el)
{
}

//=========================================================================================

void WebViewContainer::load_image(const char* src, const char* baseurl, bool redraw_on_ready)
{
}

//=========================================================================================

void WebViewContainer::on_anchor_click(const char* url, const litehtml::element::ptr& el)
{
}

//=========================================================================================

void WebViewContainer::on_mouse_event(const litehtml::element::ptr& el, litehtml::mouse_event event)
{
	
}

//=========================================================================================

int WebViewContainer::pt_to_px(int pt) const
{
	return 1;// TODO
}

//=========================================================================================

void WebViewContainer::set_base_url(const char* base_url)
{
}

//=========================================================================================

void WebViewContainer::set_caption(const char* caption)
{
}

//=========================================================================================

void WebViewContainer::set_clip(const litehtml::position& pos, const litehtml::border_radiuses& bdr_radius)
{
}

//=========================================================================================

void WebViewContainer::set_cursor(const char* cursor)
{
}

//=========================================================================================

int WebViewContainer::text_width(const char* text, litehtml::uint_ptr hFont)
{
	auto const windowWidth = static_cast<float>(MFA::LogicalDevice::Instance->GetWindowWidth()) * 0.5f;
	return static_cast<int>(_fontRenderer->TextWidth(std::string_view{text, strlen(text)}, FontRenderer::TextParams{}) * windowWidth);
}

//=========================================================================================

void WebViewContainer::transform_text(litehtml::string& text, litehtml::text_transform tt)
{
	
}

//=========================================================================================

glm::vec3 WebViewContainer::ConvertColor(litehtml::web_color const& webColor)
{
	return glm::vec3
	{
		webColor.red / 255.0f,
		webColor.green / 255.0f,
		webColor.blue / 255.0f
	};
}

//=========================================================================================
