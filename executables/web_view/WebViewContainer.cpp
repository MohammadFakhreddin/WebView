#include "WebViewContainer.hpp"

#include <ranges>

#include "BedrockFile.hpp"
#include "BedrockPath.hpp"
#include "ImportGLTF.hpp"
#include "LogicalDevice.hpp"
#include "litehtml/el_body.h"
#include "litehtml/el_text.h"

//=========================================================================================

WebViewContainer::WebViewContainer(std::shared_ptr<FontRenderer> fontRenderer)
	: litehtml::document_container()
	, _fontRenderer(std::move(fontRenderer))
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
	
	//_html->render(device->GetWindowWidth());
	/*litehtml::position clip{};
	clip.width = device->GetWindowWidth();
	clip.height = device->GetWindowHeight();
	clip.x = 0;
	clip.y = 0;
	_html->draw(0, 0, 0, &clip);*/
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
}

//=========================================================================================

litehtml::element::ptr WebViewContainer::create_element(
	const char* tag_name, 
	const litehtml::string_map& attributes,
	const std::shared_ptr<litehtml::document>& doc
)
{
	return nullptr;
	/*if (strcmp(tag_name, "html") == 0)
	{
		return std::make_shared<litehtml::html_tag>(doc);
	}
	else if (strcmp(tag_name, "body") == 0)
	{
		return std::make_shared<litehtml::el_body>(doc);
	}
	else if (strcmp(tag_name, "text") == 0)
	{
		return std::make_shared<litehtml::el_text>(attributes.begin()->second.c_str(), doc);
	}*/
	//return litehtml::document_container::create_element(tag_name, attributes, doc);
	/*MFA_ASSERT(false);
	return nullptr;*/
	//litehtml::html_tag
	//return std::make_shared<litehtml::element>(doc);
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
	// TODO, save some information about font size
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
	// TODO, Maybe we can use the line renderer here
}

//=========================================================================================

void WebViewContainer::draw_conic_gradient(
	litehtml::uint_ptr hdc, 
	const litehtml::background_layer& layer,
	const litehtml::background_layer::conic_gradient& gradient
)
{
	// TODO
}

//=========================================================================================

void WebViewContainer::draw_image(
	litehtml::uint_ptr hdc, 
	const litehtml::background_layer& layer,
	const std::string& url, 
	const std::string& base_url
)
{
	// TODO: Use image pipeline
}

//=========================================================================================

void WebViewContainer::draw_linear_gradient(
	litehtml::uint_ptr hdc, 
	const litehtml::background_layer& layer,
	const litehtml::background_layer::linear_gradient& gradient
)
{
	// TODO:
}

//=========================================================================================

void WebViewContainer::draw_list_marker(litehtml::uint_ptr hdc, const litehtml::list_marker& marker)
{
	// TODO:
}

//=========================================================================================

void WebViewContainer::draw_radial_gradient(
	litehtml::uint_ptr hdc, 
	const litehtml::background_layer& layer,
	const litehtml::background_layer::radial_gradient& gradient
)
{
	// TODO:
}

//=========================================================================================

void WebViewContainer::draw_solid_fill(
	litehtml::uint_ptr hdc, 
	const litehtml::background_layer& layer,
	const litehtml::web_color& color
)
{
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
	textParams.color.x = color.red;
	textParams.color.y = color.green;
	textParams.color.z = color.blue;
	// TOOD: Support for alpha
	//textParams.color.w = color.alpha;

	auto* device = MFA::LogicalDevice::Instance;
	float x = (float)pos.x / (float)device->GetWindowWidth();
	float y = (float)pos.y / (float)device->GetWindowHeight();

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
	// TODO: Use hFont information
	return _fontRenderer->TextWidth(std::string_view{text, strlen(text)}, FontRenderer::TextParams{});
}

//=========================================================================================

void WebViewContainer::transform_text(litehtml::string& text, litehtml::text_transform tt)
{
	
}

//=========================================================================================
