#pragma once

#include "utils/ConsolasFontRenderer.hpp"

#include "litehtml.h"
#include "utils/LineRenderer.hpp"


class WebViewContainer : public litehtml::document_container
{

public:

	using FontRenderer = MFA::ConsolasFontRenderer;
	using LineRenderer = MFA::LineRenderer;

	// TODO: We need an image renderer class as well
	explicit WebViewContainer(
		std::shared_ptr<LineRenderer> lineRenderer,
		std::shared_ptr<FontRenderer> fontRenderer
	);

	~WebViewContainer() override;

	void Update();

	void UpdateBuffers(const MFA::RT::CommandRecordState& recordState);

	void DisplayPass(MFA::RT::CommandRecordState& recordState);
	
protected:

	litehtml::element::ptr create_element(
		const char* tag_name, 
		const litehtml::string_map& attributes, 
		const std::shared_ptr<litehtml::document>& doc
	) override;

	litehtml::uint_ptr create_font(const char* faceName, 
		int size, 
		int weight, 
		litehtml::font_style italic, 
		unsigned int decoration, 
		litehtml::font_metrics* fm
	) override;

	void del_clip() override;

	void delete_font(litehtml::uint_ptr hFont) override;

	void draw_borders(
		litehtml::uint_ptr hdc, 
		const litehtml::borders& borders, 
		const litehtml::position& draw_pos, 
		bool root
	) override;

	void draw_conic_gradient(
		litehtml::uint_ptr hdc, 
		const litehtml::background_layer& layer, 
		const litehtml::background_layer::conic_gradient& gradient
	) override;

	void draw_image(
		litehtml::uint_ptr hdc, 
		const litehtml::background_layer& layer, 
		const std::string& url, 
		const std::string& base_url
	) override;

	void draw_linear_gradient(
		litehtml::uint_ptr hdc, 
		const litehtml::background_layer& layer, 
		const litehtml::background_layer::linear_gradient& gradient
	) override;

	void draw_list_marker(
		litehtml::uint_ptr hdc, 
		const litehtml::list_marker& marker
	) override;

	void draw_radial_gradient(
		litehtml::uint_ptr hdc, 
		const litehtml::background_layer& layer, 
		const litehtml::background_layer::radial_gradient& gradient
	) override;

	void draw_solid_fill(
		litehtml::uint_ptr hdc, 
		const litehtml::background_layer& layer, 
		const litehtml::web_color& color
	) override;

	void draw_text(litehtml::uint_ptr hdc, 
		const char* text, 
		litehtml::uint_ptr hFont, 
		litehtml::web_color color, 
		const litehtml::position& pos
	) override;

	void get_client_rect(litehtml::position& client) const override;

	const char* get_default_font_name() const override;

	int get_default_font_size() const override;

	void get_image_size(
		const char* src, 
		const char* baseurl, 
		litehtml::size& sz
	) override;

	void get_language(litehtml::string& language, litehtml::string& culture) const override;

	void get_media_features(litehtml::media_features& media) const override;

	void import_css(litehtml::string& text, const litehtml::string& url, litehtml::string& baseurl) override;

	void link(const std::shared_ptr<litehtml::document>& doc, const litehtml::element::ptr& el) override;

	void load_image(const char* src, const char* baseurl, bool redraw_on_ready) override;

	void on_anchor_click(const char* url, const litehtml::element::ptr& el) override;

	void on_mouse_event(const litehtml::element::ptr& el, litehtml::mouse_event event) override;

	int pt_to_px(int pt) const override;

	void set_base_url(const char* base_url) override;

	void set_caption(const char* caption) override;

	void set_clip(const litehtml::position& pos, const litehtml::border_radiuses& bdr_radius) override;

	void set_cursor(const char* cursor) override;

	int text_width(const char* text, litehtml::uint_ptr hFont) override;

	void transform_text(litehtml::string& text, litehtml::text_transform tt) override;

private:

	static glm::vec3 ConvertColor(litehtml::web_color const & webColor);

	std::shared_ptr<FontRenderer> _fontRenderer = nullptr;
	std::shared_ptr<LineRenderer> _lineRenderer = nullptr;

	litehtml::document::ptr _html = nullptr;

	std::unordered_map<litehtml::uint_ptr, std::unique_ptr<FontRenderer::TextData>> _textDataMap{};
	std::unordered_map<litehtml::uint_ptr, std::vector<std::function<void(MFA::RT::CommandRecordState&)>>> _drawCallsMap{};

};
