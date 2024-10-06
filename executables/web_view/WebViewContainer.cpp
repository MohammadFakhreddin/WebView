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
	std::shared_ptr<FontRenderer> fontRenderer,
	std::shared_ptr<SolidFillRenderer> solidFillRenderer
)
	: litehtml::document_container()
	, _fontRenderer(std::move(fontRenderer))
	, _lineRenderer(std::move(lineRenderer))
	, _solidFillRenderer(std::move(solidFillRenderer))
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
	// TODO: We have to redraw after resize
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
	for (auto & textData : _textDataList)
	{
		textData->vertexData->Update(recordState);
	}

	for (auto& tracker : _solidFillBuffers)
	{
		tracker->Update(recordState);
	}
}

//=========================================================================================

void WebViewContainer::DisplayPass(MFA::RT::CommandRecordState& recordState)
{
	for (auto & drawCall : _drawCalls)
	{
		drawCall(recordState);
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
	fm->height = static_cast<int>(_fontRenderer->TextHeight() * MFA::LogicalDevice::Instance->GetWindowHeight()) * 0.5f;
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
	_drawCalls.emplace_back([this, draw_pos, borders](MFA::RT::CommandRecordState& recordState)->void
	{
		auto const windowWidth = static_cast<float>(MFA::LogicalDevice::Instance->GetWindowWidth());
		auto const windowHeight = static_cast<float>(MFA::LogicalDevice::Instance->GetWindowHeight());

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
}

//=========================================================================================

void WebViewContainer::draw_conic_gradient(
	litehtml::uint_ptr hdc, 
	const litehtml::background_layer& layer,
	const litehtml::background_layer::conic_gradient& gradient
)
{

}

//=========================================================================================

void WebViewContainer::draw_image(
	litehtml::uint_ptr hdc, 
	const litehtml::background_layer& layer,
	const std::string& url, 
	const std::string& base_url
)
{

}

//=========================================================================================

void WebViewContainer::draw_linear_gradient(
	litehtml::uint_ptr hdc, 
	const litehtml::background_layer& layer,
	const litehtml::background_layer::linear_gradient& gradient
)
{

}

//=========================================================================================

void WebViewContainer::draw_list_marker(litehtml::uint_ptr hdc, const litehtml::list_marker& marker)
{

}

//=========================================================================================

void WebViewContainer::draw_radial_gradient(
	litehtml::uint_ptr hdc, 
	const litehtml::background_layer& layer,
	const litehtml::background_layer::radial_gradient& gradient
)
{

}

//=========================================================================================

void WebViewContainer::draw_solid_fill(
	litehtml::uint_ptr hdc, 
	const litehtml::background_layer& layer,
	const litehtml::web_color& color
)
{
	auto* device = MFA::LogicalDevice::Instance;
	auto const windowWidth = static_cast<float>(device->GetWindowWidth());
	auto const windowHeight = static_cast<float>(device->GetWindowHeight());

	auto const halfWidth = windowWidth * 0.5f;
	auto const halfHeight = windowHeight * 0.5f;

	float borderX = (static_cast<float>(layer.border_box.x) - halfWidth) / halfWidth;
	float borderY = (static_cast<float>(layer.border_box.y) - halfHeight) / halfHeight;

	float solidWidth = static_cast<float>(layer.border_box.width) / halfWidth;
	float solidHeight = static_cast<float>(layer.border_box.height) / halfHeight;

	glm::vec2 pos0{ borderX, borderY };
	glm::vec3 color0 = ConvertColor(color);

	glm::vec2 pos1 = pos0 + glm::vec2{ solidWidth, 0.0f };
	glm::vec3 color1 = color0;

	glm::vec2 pos2 = pos0 + glm::vec2{ 0.0f, solidHeight };
	glm::vec3 color2 = color0;

	glm::vec2 pos3 = pos0 + glm::vec2{ solidWidth, solidHeight };
	glm::vec3 color3 = color0;

	std::shared_ptr<MFA::LocalBufferTracker> bufferTracker = _solidFillRenderer->AllocateBuffer(
		pos0, 
		pos1, 
		pos2, 
		pos3, 
		color0, 
		color1, 
		color2, 
		color3,
		static_cast<float>(layer.border_radius.bottom_left_x) / windowWidth // TODO: Each of them need a border radius
	);
	_solidFillBuffers.emplace_back(bufferTracker);

	_drawCalls.emplace_back([this, bufferTracker](MFA::RT::CommandRecordState& recordState)->void
	{
		_solidFillRenderer->Draw(recordState, *bufferTracker);
	});
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
	std::shared_ptr textData = _fontRenderer->AllocateTextData();

	FontRenderer::TextParams textParams{};
	textParams.color = ConvertColor(color);
	textParams.hTextAlign = FontRenderer::HorizontalTextAlign::Left;
	textParams.vTextAlign = FontRenderer::VerticalTextAlign::Top;
	
	float x = pos.x;
	float y = pos.y;

	_fontRenderer->AddText(*textData, text, x, y, textParams);
	_textDataList.emplace_back(textData);

	_drawCalls.emplace_back([this, textData](MFA::RT::CommandRecordState& recordState)->void
	{
		_fontRenderer->Draw(recordState, *textData);
	});
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
		static_cast<float>(webColor.red) / 255.0f,
		static_cast<float>(webColor.green) / 255.0f,
		static_cast<float>(webColor.blue) / 255.0f
	};
}

//=========================================================================================
