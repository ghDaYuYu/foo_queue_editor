#pragma once

#include "style_manager.h"
#include "../columns_ui-sdk/ui_extension.h" //(2)

// CUI

namespace {

	class CuiStyleCallback : public columns_ui::colours::common_callback,
		public columns_ui::fonts::common_callback {

		static_api_ptr_t<cui::colours::manager> colorManager;
		static_api_ptr_t<cui::fonts::manager> fontManager;
		std::function<void(uint32_t)> callback = nullptr;

	public:

		CuiStyleCallback(std::function<void(uint32_t)> callback) : callback(callback) {
			colorManager->register_common_callback(this);
			fontManager->register_common_callback(this);
		}

		~CuiStyleCallback() {
			colorManager->deregister_common_callback(this);
			fontManager->deregister_common_callback(this);
		}

		virtual void on_colour_changed(uint32_t changed_items_mask) const {
			callback(changed_items_mask);
		}

		virtual void on_bool_changed(uint32_t changed_items_mask) const final {

			callback(changed_items_mask);
		}

		virtual void on_font_changed(uint32_t changed_items_mask) const final {
			callback(changed_items_mask);
		}

	};

	class CuiStyleManager : public StyleManager {

		CuiStyleCallback callback;

	public:

		CuiStyleManager() : callback([&](uint32_t chg) { /*ApplyDark();*/  this->onChange(chg); })
		{
			updateCache();
		}

		virtual ~CuiStyleManager() final {
			//..
		}

	protected:

		virtual COLORREF defaultTitleColor() final {
			return columns_ui::colours::helper(GUID{}).get_colour(
				columns_ui::colours::colour_text);
		}
		//todo: rev
		virtual LOGFONT defaultTitleFont() final {
			LOGFONT font;
			static_api_ptr_t<cui::fonts::manager> fontManager;
			fontManager->get_font(columns_ui::fonts::font_type_items, font);
			return font;
		}
		//todo: rev
		virtual LOGFONT defaultListFont() final {
			LOGFONT font;
			static_api_ptr_t<cui::fonts::manager> fontManager;
			fontManager->get_font(columns_ui::fonts::font_type_items, font);
			return font;
		}
		//todo...
		virtual LOGFONT defaultPlaylistFont() final {
			//todo: rev playlist font
			LOGFONT font;
			static_api_ptr_t<cui::fonts::manager> fontManager;
			fontManager->get_font(columns_ui::fonts::font_type_items, font);
			return font;
		}

		virtual COLORREF defaultBgColor() final {
			return columns_ui::colours::helper(GUID{}).get_colour(
				columns_ui::colours::colour_background);
		}
		virtual COLORREF defaultHotColor() final {
			return columns_ui::colours::helper(GUID{}).get_colour(
				columns_ui::colours::colour_active_item_frame);
		}

		virtual COLORREF defaultSelColor() {
			return columns_ui::colours::helper(GUID{}).get_colour(
				columns_ui::colours::colour_selection_text);
		}
		virtual COLORREF defaultHighColor() {
			return columns_ui::colours::g_get_system_color(columns_ui::colours::colour_selection_text);
		}

	public:

		virtual bool isDark() const override {
			return columns_ui::colours::helper(GUID{}).get_bool(cui::colours::bool_dark_mode_enabled);
		}

		//todo: remove to DUI_StyleManager?
		COLORREF getTitleColor() const override {
			return COLORREF();
		}
		COLORREF getBgColor() const override {
			return COLORREF();
		}
		COLORREF getSelColor() const override {
			return COLORREF();
		}
		COLORREF getHighColor() const override {
			return COLORREF();
		}
		COLORREF getHotColor() const override {
			return cachedHotColor;
		}
		LOGFONT getTitleFont() const override {
			return cachedTitleFont;
		};
		LOGFONT getListFont() const override {
			return cachedListFont;
		};
		LOGFONT getPlaylistFont() const override {
			return cachedPlaylistFont;
		}

	};
}