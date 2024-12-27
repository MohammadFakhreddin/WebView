#include "WebViewContainer.hpp"

#include "ImportGLTF.hpp"
#include "LogicalDevice.hpp"
#include "RenderBackend.hpp"
#include "BedrockFile.hpp"
#include "BedrockPath.hpp"

#include <ranges>

#include "litehtml/render_item.h"

using namespace MFA;

// TODO: For image rendering we have to use set Scissor and viewport.

//=========================================================================================

WebViewContainer::WebViewContainer(
    char const * htmlAddress,
    litehtml::position clip,
    Params params
)
	: litehtml::document_container()
    , _htmlAddress(htmlAddress)
	, _solidFillRenderer(std::move(params.solidFillRenderer))
    , _imageRenderer(std::move(params.imageRenderer))
	, _requestBlob(std::move(params.requestBlob))
    , _requestFont(std::move(params.requestFont))
    , _requestImage(std::move(params.requestImage))
{
    MFA_ASSERT(std::filesystem::exists(htmlAddress));
    _parentAddress = std::filesystem::path(htmlAddress).parent_path().string();
    MFA_ASSERT(_solidFillRenderer != nullptr);
    MFA_ASSERT(_imageRenderer != nullptr);
    MFA_ASSERT(_requestBlob != nullptr);
    MFA_ASSERT(_requestFont != nullptr);
    MFA_ASSERT(_requestImage != nullptr);

    OnReload(std::move(clip));
}

//=========================================================================================

WebViewContainer::~WebViewContainer() = default;

//=========================================================================================

void WebViewContainer::Update()
{
    for (auto &state : _states)
    {
        if (state.lifeTime > 0)
        {
            state.lifeTime -= 1;
        }
    }
    if (_isDirty == true)
    {
        _isDirty = false;
        SwitchActiveState();
        _html->render(_clip.width, litehtml::render_all);
        _html->draw(0, _clip.x, _clip.y, &_clip);
    }
}

//=========================================================================================

void WebViewContainer::UpdateBuffer(RT::CommandRecordState & recordState)
{
    for (auto &bufferCall : _activeState->bufferCalls)
    {
        bufferCall(recordState);
    }
}

//=========================================================================================

void WebViewContainer::DisplayPass(RT::CommandRecordState & recordState)
{
    for (auto &drawCall : _activeState->drawCalls)
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
    auto fontRenderer = _requestFont(faceName);
    MFA_ASSERT(fontRenderer != nullptr);

    fm->height = static_cast<int>(fontRenderer->TextHeight((float)size));
	fm->draw_spaces = false;

    _fontList.emplace_back();
    auto & font = _fontList.back();
    font.renderer = fontRenderer;
    font.size = size;
    font.id = (int)_fontList.size();
	return font.id;
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
	//_drawCalls.emplace_back([this, draw_pos, borders](RT::CommandRecordState& recordState)->void
	//{
	//	auto const windowWidth = static_cast<float>(LogicalDevice::Instance->GetWindowWidth());
	//	auto const windowHeight = static_cast<float>(LogicalDevice::Instance->GetWindowHeight());

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
{}

//=========================================================================================

void WebViewContainer::draw_image(
	litehtml::uint_ptr hdc, 
	const litehtml::background_layer& layer,
	const std::string& url, 
	const std::string& base_url
)
{
    // TODO: We have to reuse existing buffer and only create new one when needed.
    auto const imagePath = Path::Get(url.c_str(), _parentAddress.c_str());
    std::shared_ptr gpuTexture = _requestImage(imagePath.c_str());
    std::shared_ptr imageData = _imageRenderer->AllocateImageData(
        *gpuTexture,
        ImageRenderer::Extent {
            .x = layer.origin_box.x,
            .y = layer.origin_box.y,
            .width = layer.origin_box.width,
            .height = layer.origin_box.height
        }
    );

    _activeState->drawCalls.emplace_back([this, imageData](RT::CommandRecordState & recordState)->void
    {
        _imageRenderer->Draw(recordState, ImagePipeline::PushConstants {.model = _modelMat}, *imageData);
    });

    _activeState->bufferCalls.emplace_back([this, imageData](RT::CommandRecordState & recordState)->void
    {
        imageData->vertexData->Update(recordState);
    });
	//MFA_LOG_INFO(
	//	"url=%s, base_url=%s, layer.width: %d, layer.height: %d"
	//	, url.c_str()
	//	, base_url.c_str()
	//	, layer.border_box.width
	//	, layer.border_box.height
	//);
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
    // TODO: Inverstiage border box.
	auto const borderX = static_cast<float>(layer.border_box.x);
    auto const borderY = static_cast<float>(layer.border_box.y);

    auto const solidWidth = static_cast<float>(layer.border_box.width);;
    auto const solidHeight = static_cast<float>(layer.border_box.height);

    glm::vec2 const topLeftPos{ borderX, borderY };
    auto const topLeftColor = ConvertColor(color);
    auto const topLeftX = (float)layer.border_radius.top_left_x;
    auto const topLeftY = (float)layer.border_radius.top_left_y;
    auto const topLeftRadius = glm::vec2{ topLeftX, topLeftY };

    glm::vec2 const topRightPos = topLeftPos + glm::vec2{ solidWidth, 0.0f };
    auto const topRightColor = topLeftColor;
    auto const topRightX = (float)layer.border_radius.top_right_x;
    auto const topRightY = (float)layer.border_radius.top_right_y;
    auto const topRightRadius = glm::vec2{ topRightX, topRightY };;

    glm::vec2 const bottomLeftPos = topLeftPos + glm::vec2{ 0.0f, solidHeight };
    auto const bottomLeftColor = topLeftColor;
    auto const bottomLeftX = (float)layer.border_radius.bottom_left_x;
    auto const bottomLeftY = (float)layer.border_radius.bottom_left_y;
    auto const bottomLeftRadius = glm::vec2{ bottomLeftX, bottomLeftY };

    glm::vec2 const bottomRightPos = topLeftPos + glm::vec2{ solidWidth, solidHeight };
    auto const bottomRightColor = topLeftColor;
    auto const bottomRightX = (float)layer.border_radius.bottom_right_x;
    auto const bottomRightY = (float)layer.border_radius.bottom_right_y;
	auto const bottomRightRadius = glm::vec2{bottomRightX, bottomRightY};

	std::shared_ptr<LocalBufferTracker> bufferTracker = _solidFillRenderer->AllocateBuffer(
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

	_activeState->drawCalls.emplace_back([this, bufferTracker](RT::CommandRecordState &recordState) -> void
	{
	    _solidFillRenderer->Draw(
            recordState,
            SolidFillPipeline::PushConstants{.model = _modelMat},
            *bufferTracker
        );
	});

    _activeState->bufferCalls.emplace_back([this, bufferTracker](RT::CommandRecordState &recordState) -> void
    {
        bufferTracker->Update(recordState);
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
    auto & fontData = _fontList[hFont - 1];
    // TODO: We can reuse the buffer instead. There is no need to destroy and recreate the buffer everytime.
    std::shared_ptr textData = fontData.renderer->AllocateTextData();

	FontRenderer::TextParams textParams{};
	textParams.color = ConvertColor(color);
	textParams.hTextAlign = FontRenderer::HorizontalTextAlign::Left;
    textParams.fontSizeInPixels = (float)fontData.size;
	
	auto const x = static_cast<float>(pos.x);
	auto const y = static_cast<float>(pos.y);

	fontData.renderer->AddText(*textData, text, x, y, textParams);

    _activeState->drawCalls.emplace_back([this, textData, &fontData](RT::CommandRecordState &recordState) -> void
	{
	    fontData.renderer->Draw(
            recordState,
            TextOverlayPipeline::PushConstants{ .model = _modelMat },
            *textData
        );
	});

    _activeState->bufferCalls.emplace_back([this, textData](RT::CommandRecordState &recordState) -> void
    {
        textData->vertexData->Update(recordState);
    });
}

//=========================================================================================

void WebViewContainer::get_client_rect(litehtml::position& client) const
{
	auto* device = LogicalDevice::Instance;
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
	return (int)FontRenderer::DefaultFontSize;
}

//=========================================================================================

void WebViewContainer::get_image_size(const char* src, const char* baseurl, litehtml::size& sz)
{
    auto image = _requestImage(Path::Instance()->Get(src, _parentAddress.c_str()).c_str());
    // TODO
	//MFA_LOG_INFO("src: %s, baseurl: %s", src, baseurl);
	// sz.width = 100;
	// sz.height = 100;
}

//=========================================================================================

void WebViewContainer::get_language(litehtml::string& language, litehtml::string& culture) const
{
}

//=========================================================================================

void WebViewContainer::get_media_features(litehtml::media_features& media) const
{
    auto * device = LogicalDevice::Instance;
    media.width = device->GetWindowWidth();
    media.height = device->GetWindowHeight();
    media.device_width = media.width;
    media.device_height = media.height;
    media.color = 4;
    media.type = litehtml::media_type_screen;
    media.resolution = 1;
}

//=========================================================================================

void WebViewContainer::import_css(
    litehtml::string& text,
    const litehtml::string& url,
    litehtml::string& baseurl
)
{
    // MFA_LOG_INFO(
    //     "Requested import css.\nText: %s\nurl: %s\nbaseurl: %s"
    //     , text.c_str(), url.c_str(), baseurl.c_str()
    // );
    auto const cssPath = Path::Get(url.c_str(), _parentAddress.c_str());
    auto const cssBlob = _requestBlob(cssPath.c_str(), true);
    text = cssBlob->As<char const>();
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
    auto * device = LogicalDevice::Instance;
    auto const windowWidth = static_cast<float>(device->GetWindowWidth());
    auto const windowHeight = static_cast<float>(device->GetWindowHeight());

    float scaleFactorX = 1.0f;
    float scaleFactorY = 1.0f;

    float bodyWidth = _bodyWidth;
    if (bodyWidth <= 0)
    {
        bodyWidth = windowWidth;
    }
    float wScale = (float)windowWidth / bodyWidth;
    scaleFactorX = wScale;

    float bodyHeight = _bodyHeight;
    if (bodyHeight <= 0)
    {
        bodyHeight = windowHeight;
    }
    float hScale = (float)windowHeight / bodyHeight;
    scaleFactorY = hScale;

    float scaleFactor = std::min(scaleFactorX, scaleFactorY);
     scaleFactor = std::max(scaleFactor, 1.0f);

    // float halfWidth = windowWidth * 0.5f;
    // float halfHeight = windowHeight * 0.5f;
    // float scaleX = (1.0f / halfWidth) * scaleFactor;
    // float scaleY = (1.0f / halfHeight) * scaleFactor;
	return pt * scaleFactor;
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
    auto & fontData = _fontList[hFont - 1];
    auto const textWidth = fontData.renderer->TextWidth(std::string_view{text, strlen(text)}, fontData.size);
    return static_cast<int>(textWidth);
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

void WebViewContainer::SwitchActiveState()
{
    if (_activeState != nullptr)
    {
        _activeState->lifeTime = LogicalDevice::Instance->GetMaxFramePerFlight();
    }
    _activeIdx = -1;
    for (int i = 0; i < _states.size(); ++i)
    {
        if (_states[i].lifeTime <= 0)
        {
            _activeIdx = i;
        }
    }
    if (_activeIdx < 0)
    {
        _activeIdx = _states.size();
        _states.emplace_back();
    }
    _activeState = &_states[_activeIdx];
    _activeState->drawCalls.clear();
    _activeState->bufferCalls.clear();
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
        auto const result = GetElementByTag(tag, child);
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
    // TODO: Try this
    //element->draw(0, 0, 0, &_clip, );
	_isDirty = true;
}

//=========================================================================================

void WebViewContainer::OnReload(litehtml::position clip)
{
    _htmlBlob = File::Read(_htmlAddress);
    char const *htmlText = _htmlBlob->As<char const>();
    _html = litehtml::document::createFromString(htmlText, this);

    auto const bodyTag = GetElementByTag("body");
    if (bodyTag != nullptr)
    {
        try
        {
            _bodyWidth = std::stoi(bodyTag->get_attr("width", "-1"));
            _bodyHeight = std::stoi(bodyTag->get_attr("height", "-1"));
        }
        catch (const std::exception &e)
        {
            MFA_LOG_WARN("Failed to parse body width and height with error\n %s", e.what());
        }
    }

    OnResize(std::move(clip));
}

//=========================================================================================

void WebViewContainer::OnResize(litehtml::position clip)
{
    auto *device = LogicalDevice::Instance;
    auto const windowWidth = static_cast<float>(device->GetWindowWidth());
    auto const windowHeight = static_cast<float>(device->GetWindowHeight());
    
     /*float scaleFactorX = 1.0f;
     float scaleFactorY = 1.0f;
    
     float bodyWidth = _bodyWidth;
     if (bodyWidth <= 0)
     {
         bodyWidth = windowWidth;
     }
     float wScale = (float)windowWidth / bodyWidth;
     scaleFactorX = wScale;
    
     float bodyHeight = _bodyHeight;
     if (bodyHeight <= 0)
     {
         bodyHeight = windowHeight;
     }
     float hScale = (float)windowHeight / bodyHeight;
     scaleFactorY = hScale;*/

    //float scaleFactor = std::min(scaleFactorX, scaleFactorY);
     //scaleFactor = std::max(scaleFactor, 1.0f);
    float scaleFactor = 1.0f;

    float halfWidth = windowWidth * 0.5f;
    float halfHeight = windowHeight * 0.5f;
    float scaleX = (1.0f / halfWidth) * scaleFactor;
    float scaleY = (1.0f / halfHeight) * scaleFactor;

    _clip = std::move(clip);
   
    _modelMat = glm::transpose(
        glm::scale(glm::identity<glm::mat4>(), glm::vec3{scaleX, scaleY, 1.0f}) *
        glm::translate(glm::identity<glm::mat4>(), glm::vec3{-halfWidth, -halfHeight, 0.0f})
    );

    _isDirty = true;

    // It is not working as expected at the moment
    _html->media_changed();
}

//=========================================================================================
