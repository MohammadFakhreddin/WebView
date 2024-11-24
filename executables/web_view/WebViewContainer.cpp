#include "WebViewContainer.hpp"

#include "ImportGLTF.hpp"
#include "LogicalDevice.hpp"
#include "RenderBackend.hpp"

#include <ranges>

//=========================================================================================

WebViewContainer::WebViewContainer(
	std::shared_ptr<MFA::Blob> const & htmlBlob,
	litehtml::position clip,
	std::shared_ptr<LineRenderer> lineRenderer,
	std::shared_ptr<FontRenderer> fontRenderer,
	std::shared_ptr<SolidFillRenderer> solidFillRenderer
)
	: litehtml::document_container()
	, _clip(clip)
	, _fontRenderer(std::move(fontRenderer))
	, _lineRenderer(std::move(lineRenderer))
	, _solidFillRenderer(std::move(solidFillRenderer))
{
	char const * htmlText = htmlBlob->As<char const>();

	_html = litehtml::document::createFromString(
		htmlText,
		this
	);

    auto const bodyTag = GetElementByTag("body");
    MFA_ASSERT(bodyTag != nullptr);

    auto * device = MFA::LogicalDevice::Instance;
    auto const windowWidth = static_cast<float>(device->GetWindowWidth());
    auto const windowHeight = static_cast<float>(device->GetWindowHeight());

    float halfWidth = windowWidth * 0.5f;
    float halfHeight = windowHeight * 0.5f;
    _modelMat = glm::transpose(glm::scale(glm::identity<glm::mat4>(), glm::vec3{1.0f / halfWidth, 1.0f / halfHeight, 1.0f }) *
        glm::translate(glm::identity<glm::mat4>(), glm::vec3{ -halfWidth, -halfHeight, 0.0f }));

    MFA_LOG_INFO("Start!");
}

//=========================================================================================

WebViewContainer::~WebViewContainer() = default;

//=========================================================================================

void WebViewContainer::Update()
{
	if (_isDirty == true)
	{
		MFA::RB::DeviceWaitIdle(MFA::LogicalDevice::Instance->GetVkDevice());
		_isDirty = false;
		_drawCalls.clear();
		_textDataList.clear();
		_solidFillBuffers.clear();
		_html->render(_clip.width, litehtml::render_all);
		_html->draw(0, _clip.x, _clip.y, &_clip);
        // TODO: We can record command buffer once
	}
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
	float const borderX = static_cast<float>(layer.border_box.x);
    float const borderY = static_cast<float>(layer.border_box.y);

    float const solidWidth = static_cast<float>(layer.border_box.width);;
    float const solidHeight = static_cast<float>(layer.border_box.height);

    glm::vec2 const topLeftPos{ borderX, borderY };
    auto const topLeftColor = ConvertColor(color);
    float const topLeftX = (float)layer.border_radius.top_left_x;
    float const topLeftY = (float)layer.border_radius.top_left_y;
    auto const topLeftRadius = glm::vec2{ topLeftX, topLeftY };

    glm::vec2 const topRightPos = topLeftPos + glm::vec2{ solidWidth, 0.0f };
    auto const topRightColor = topLeftColor;
    float const topRightX = (float)layer.border_radius.top_right_x;
    float const topRightY = (float)layer.border_radius.top_right_y;
    auto const topRightRadius = glm::vec2{ topRightX, topRightY };;

    glm::vec2 const bottomLeftPos = topLeftPos + glm::vec2{ 0.0f, solidHeight };
    auto const bottomLeftColor = topLeftColor;
    float const bottomLeftX = (float)layer.border_radius.bottom_left_x;
    float const bottomLeftY = (float)layer.border_radius.bottom_left_y;
    auto const bottomLeftRadius = glm::vec2{ bottomLeftX, bottomLeftY };

    glm::vec2 const bottomRightPos = topLeftPos + glm::vec2{ solidWidth, solidHeight };
    auto const bottomRightColor = topLeftColor;
    float const bottomRightX = (float)layer.border_radius.bottom_right_x;
    float const bottomRightY = (float)layer.border_radius.bottom_right_y;
	auto const bottomRightRadius = glm::vec2{bottomRightX, bottomRightY};

	std::shared_ptr<MFA::LocalBufferTracker> bufferTracker = _solidFillRenderer->AllocateBuffer(
		topLeftPos,
		bottomLeftPos,
		topRightPos,
		bottomRightPos,

		topLeftColor,
		bottomLeftColor,
		topRightColor,
		bottomRightColor,

		topLeftRadius,
		bottomLeftRadius,
		topRightRadius,
		bottomRightRadius
	);
	_solidFillBuffers.emplace_back(bufferTracker);

	_drawCalls.emplace_back([this, bufferTracker](MFA::RT::CommandRecordState& recordState)->void
	{
        _solidFillRenderer->Draw(
            recordState,
            MFA::SolidFillPipeline::PushConstants{.model = _modelMat},
            *bufferTracker
        );
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
	// We can also use ImGui Draw text
	std::shared_ptr textData = _fontRenderer->AllocateTextData();

	FontRenderer::TextParams textParams{};
	textParams.color = ConvertColor(color);
	textParams.hTextAlign = FontRenderer::HorizontalTextAlign::Left;
	textParams.vTextAlign = FontRenderer::VerticalTextAlign::Top;
	textParams.scale = _fontScales[hFont - 1];
	
	auto const x = static_cast<float>(pos.x);
	auto const y = static_cast<float>(pos.y);

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

glm::vec4 WebViewContainer::ConvertColor(litehtml::web_color const& webColor)
{
	return glm::vec4
	{
		static_cast<float>(webColor.red) / 255.0f,
		static_cast<float>(webColor.green) / 255.0f,
		static_cast<float>(webColor.blue) / 255.0f,
		static_cast<float>(webColor.alpha) / 255.0f
	};
}

//=========================================================================================

litehtml::element::ptr WebViewContainer::GetElementById(char const * id) const
{
	return GetElementById(id, _html->root());
}

//=========================================================================================

litehtml::element::ptr WebViewContainer::GetElementById(char const * id, litehtml::element::ptr element)
{
	auto const * elementId = element->get_attr("id");
	if (elementId != nullptr && strcmp(elementId, id) == 0)
	{
		return element;
	}

	for (auto & child : element->children())
	{
		auto const result = GetElementById(id, child);
		if (result != nullptr)
		{
			return result;
		}
	}

	return nullptr;
}

//=========================================================================================

litehtml::element::ptr WebViewContainer::GetElementByTag(char const* tag) const
{
    return GetElementByTag(tag, _html->root());
}

//=========================================================================================

litehtml::element::ptr WebViewContainer::GetElementByTag(char const * tag, litehtml::element::ptr element)
{
    auto const * tagName = element->get_tagName();
    if (tagName != nullptr && strcmp(tagName, tag) == 0)
    {
        return element;
    }

    for (auto & child : element->children())
    {
        auto const result = GetElementById(tag, child);
        if (result != nullptr)
        {
            return result;
        }
    }

    return nullptr;
}

//=========================================================================================

void WebViewContainer::InvalidateStyles(litehtml::element::ptr element)
{
	element->apply_stylesheet(_html->m_styles);
	element->compute_styles();
	_isDirty = true;
}

//=========================================================================================
