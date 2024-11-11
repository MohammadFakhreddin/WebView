#include "WebViewContainer.hpp"

#include "ImportGLTF.hpp"
#include "LogicalDevice.hpp"
#include "RenderBackend.hpp"

#include <ranges>

//=========================================================================================
// TODO: Create class that extends from this class for specific tasks
WebViewContainer::WebViewContainer(
	std::shared_ptr<MFA::Blob> const & htmlBlob,
	std::shared_ptr<LineRenderer> lineRenderer,
	std::shared_ptr<FontRenderer> fontRenderer,
	std::shared_ptr<SolidFillRenderer> solidFillRenderer
)
	: litehtml::document_container()
	, _fontRenderer(std::move(fontRenderer))
	, _lineRenderer(std::move(lineRenderer))
	, _solidFillRenderer(std::move(solidFillRenderer))
{
	// TODO: We can use createFromString functionality
	char const * htmlText = htmlBlob->As<char const>();

	//std::string text = htmlText;
	//auto replaceAll = [](std::string& s, std::string o, std::string n) {  s.replace(s.find(o), o.size(), n); };

	//std::string search = "center-column button";
	//std::string replace = "center-column button selected";
	//replaceAll(text, search, replace);
	//text.replace("id=\"new - game\" class=\"center - column button\"", "id=\"new - game\" class=\"center - column button selected\"");
	_html = litehtml::document::createFromString(
		htmlText,
		this
	);
	//MFA_ASSERT(_html != nullptr);
	
	auto root = _html->root();
	
	//auto newGameButton = GetElementById("new-game", root);
	//MFA_ASSERT(newGameButton != nullptr);
	//std::string classAttr = newGameButton->get_attr("class");
	//classAttr = classAttr.append(" selected");
	//MFA_LOG_INFO("Class attr: %s", classAttr.c_str());
	//newGameButton->set_attr("class", classAttr.c_str());
	//newGameButton->parse_attributes();
	//newGameButton->compute_styles();

	/*litehtml::position::vector reDraw {};
	_html->root()->find_styles_changes(reDraw);
	*/
	auto* device = MFA::LogicalDevice::Instance;
	
	litehtml::position clip{};
	clip.width = device->GetWindowWidth();
	clip.height = device->GetWindowHeight();
	clip.x = 0;
	clip.y = 0;

	// _html->draw(0, clip.x, clip.y, &clip);
	_html->render(clip.width, litehtml::render_all);
	_html->draw(0, clip.x, clip.y, &clip);

	// clip.width = device->GetWindowWidth() * 0.5f;
	// clip.height = device->GetWindowHeight();
	// clip.x = device->GetWindowWidth() * 0.5f;
	// clip.y = 0;

	// _html->render(clip.width);
	// _html->draw(1, clip.x, clip.y, &clip);
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
	int const size, 
	int const weight,
	litehtml::font_style italic, 
	unsigned int decoration, 
	litehtml::font_metrics* fm
)
{
	float const scale = static_cast<float>(size) / static_cast<float>(get_default_font_size());
	float const windowHeight = static_cast<float>(MFA::LogicalDevice::Instance->GetWindowHeight());
	fm->height = static_cast<int>(_fontRenderer->TextHeight(scale) * windowHeight * 0.5f);
	fm->draw_spaces = false;
	_fontScales.emplace_back(scale);
	return _fontScales.size();
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
	//_drawCalls.emplace_back([this, draw_pos, borders](MFA::RT::CommandRecordState& recordState)->void
	//{
	//	auto const windowWidth = static_cast<float>(MFA::LogicalDevice::Instance->GetWindowWidth());
	//	auto const windowHeight = static_cast<float>(MFA::LogicalDevice::Instance->GetWindowHeight());

	//	glm::vec3 topLeft {
	//		static_cast<float>(draw_pos.x) / windowWidth,
	//		static_cast<float>(draw_pos.y) / windowHeight,
	//		0.0f
	//	};
	//	glm::vec3 bottomRight = topLeft + glm::vec3 {windowWidth, windowHeight, 0.0f};
	//	glm::vec3 topRight = topLeft + glm::vec3{ windowWidth, 0.0f, 0.0f };
	//	glm::vec3 bottomLeft = topLeft + glm::vec3{ 0.0f, windowHeight, 0.0f };

	//	_lineRenderer->Draw(recordState, topLeft, topRight, glm::vec4{ ConvertColor(borders.top.color) , 1.0f });
	//	_lineRenderer->Draw(recordState, topLeft, bottomLeft, glm::vec4{ ConvertColor(borders.left.color) , 1.0f });
	//	_lineRenderer->Draw(recordState, bottomLeft, bottomRight, glm::vec4{ ConvertColor(borders.bottom.color) , 1.0f });
	//	_lineRenderer->Draw(recordState, topRight, bottomRight, glm::vec4{ ConvertColor(borders.right.color) , 1.0f});
	//});
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
	MFA_LOG_INFO(
		"url=%s, base_url=%s, layer.width: %d, layer.height: %d"
		, url.c_str()
		, base_url.c_str()
		, layer.border_box.width
		, layer.border_box.height
	);
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
	textParams.scale = _fontScales[hFont - 1];
	
	float x = static_cast<float>(pos.x);
	float y = static_cast<float>(pos.y);

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
	return "consolos";
}

//=========================================================================================

int WebViewContainer::get_default_font_size() const
{
	return 14;
}

//=========================================================================================

void WebViewContainer::get_image_size(const char* src, const char* baseurl, litehtml::size& sz)
{
	MFA_LOG_INFO("src: %s, baseurl: %s", src, baseurl);
	sz.width = 100;
	sz.height = 100;
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
	MFA_LOG_INFO("src: %s, baseurl: %s", src, baseurl);
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
	FontRenderer::TextParams params{};
	params.scale = _fontScales[hFont - 1];
	return static_cast<int>(_fontRenderer->TextWidth(std::string_view{text, strlen(text)}, params) * windowWidth);
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

litehtml::element::ptr WebViewContainer::GetElementById(char const * targetId, litehtml::element::ptr element)
{
	auto const * elementId = element->get_attr("id");
	if (elementId != nullptr && strcmp(elementId, targetId) == 0)
	{
		return element;
	}

	for (auto & child : element->children())
	{
		auto result = GetElementById(targetId, child);
		if (result != nullptr)
		{
			return result;
		}
	}

	return nullptr;
}

// GumboNode * WebViewContainer::GetElementById(const char *id, GumboNode * document) 
// {

// 	if (GUMBO_NODE_DOCUMENT != document->type && GUMBO_NODE_ELEMENT != document->type) 
// 	{
// 		return nullptr;
// 	}

// 	GumboAttribute *node_id =
// 	gumbo_get_attribute(&document->v.element.attributes, "id");
// 	if (node_id && 0 == strcmp(id, node_id->value)) 
// 	{
// 		return document;
// 	}

// 	// iterate all children
// 	GumboVector *children = &document->v.element.children;
// 	for (unsigned int i = 0; i < children->length; i++) 
// 	{
// 		GumboNode *node = GetElementById(id, (GumboNode *)children->data[i]);
// 		if (node) return node;
// 	}

// 	return nullptr;
// }

//=========================================================================================

// bool WebViewContainer::ParseHTML(std::string & string)
// {
// 	if (_gumboOutput != nullptr)
// 	{
// 		gumbo_destroy_output(&kGumboDefaultOptions, _gumboOutput);
// 	}

// 	// Create litehtml::document
// 	_html = make_shared<litehtml::document>(this);

// 	// Parse document into GumboOutput
// 	GumboOutput* output = _html->parse_html(string);

// 	// mode must be set before doc->create_node because it is used in html_tag::set_attr
// 	switch (output->document->v.document.doc_type_quirks_mode)
// 	{
// 	case GUMBO_DOCTYPE_NO_QUIRKS:      doc->m_mode = no_quirks_mode;      break;
// 	case GUMBO_DOCTYPE_QUIRKS:         doc->m_mode = quirks_mode;         break;
// 	case GUMBO_DOCTYPE_LIMITED_QUIRKS: doc->m_mode = limited_quirks_mode; break;
// 	}

// 	// Create litehtml::elements.
// 	elements_list root_elements;
// 	doc->create_node(output->root, root_elements, true);
// 	if (!root_elements.empty())
// 	{
// 		doc->m_root = root_elements.back();
// 	}

// 	// Destroy GumboOutput
// 	gumbo_destroy_output(&kGumboDefaultOptions, output);

// 	if (master_styles != "")
// 	{
// 		doc->m_master_css.parse_css_stylesheet(master_styles, "", doc);
// 		doc->m_master_css.sort_selectors();
// 	}
// 	if (user_styles != "")
// 	{
// 		doc->m_user_css.parse_css_stylesheet(user_styles, "", doc);
// 		doc->m_user_css.sort_selectors();
// 	}

// 	// Let's process created elements tree
// 	if (doc->m_root)
// 	{
// 		doc->container()->get_media_features(doc->m_media);

// 		doc->m_root->set_pseudo_class(_root_, true);

// 		// apply master CSS
// 		doc->m_root->apply_stylesheet(doc->m_master_css);

// 		// parse elements attributes
// 		doc->m_root->parse_attributes();

// 		// parse style sheets linked in document
// 		for (const auto& css : doc->m_css)
// 		{
// 			media_query_list_list::ptr media;
// 			if (css.media != "")
// 			{
// 				auto mq_list = parse_media_query_list(css.media, doc);
// 				media = make_shared<media_query_list_list>();
// 				media->add(mq_list);
// 			}
// 			doc->m_styles.parse_css_stylesheet(css.text, css.baseurl, doc, media);
// 		}
// 		// Sort css selectors using CSS rules.
// 		doc->m_styles.sort_selectors();

// 		// Apply media features.
// 		doc->update_media_lists(doc->m_media);

// 		// Apply parsed styles.
// 		doc->m_root->apply_stylesheet(doc->m_styles);

// 		// Apply user styles if any
// 		doc->m_root->apply_stylesheet(doc->m_user_css);

// 		// Initialize element::m_css
// 		doc->m_root->compute_styles();

// 		// Create rendering tree
// 		doc->m_root_render = doc->m_root->create_render_item(nullptr);

// 		// Now the m_tabular_elements is filled with tabular elements.
// 		// We have to check the tabular elements for missing table elements 
// 		// and create the anonymous boxes in visual table layout
// 		doc->fix_tables_layout();

// 		// Finally initialize elements
// 		// init() returns pointer to the render_init element because it can change its type
// 		doc->m_root_render = doc->m_root_render->init();
// 	}
// }

//=========================================================================================
