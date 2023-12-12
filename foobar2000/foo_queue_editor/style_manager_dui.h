#pragma once

#include "style_manager.h"

// DUI

namespace {

	class DuiStyleManager : public StyleManager {

	public:

		DuiStyleManager(ui_element_instance_callback::ptr instance_callback)
			: instance_callback(instance_callback) {
			updateCache();
		}

		DuiStyleManager() {
			//..
		}

		virtual ~DuiStyleManager() final {
			instance_callback = nullptr;
		}

	public:

		virtual bool isDark() const final {
			return instance_callback->is_dark_mode();
		}

		virtual COLORREF getTitleColor() const final {
			return cachedTitleColor;
		}
		virtual COLORREF getBgColor() const final {
			return cachedBgColor;
		}
		virtual COLORREF getSelColor() const final {
			return cachedSelColor;
		}
		virtual COLORREF getHighColor() const final {
			return cachedHighColor;
		}
		virtual COLORREF getHotColor() const final {
			return cachedHotColor;
		}

		virtual LOGFONT getTitleFont() const final {
			return cachedTitleFont;
		};
		virtual LOGFONT getListFont() const final {
			return cachedListFont;
		};
		virtual LOGFONT getPlaylistFont() const final {
			return cachedPlaylistFont;
		}

	protected:

		virtual COLORREF defaultTitleColor() final {
			return instance_callback->query_std_color(ui_color_text);
		}
		virtual COLORREF defaultBgColor() final {
			return instance_callback->query_std_color(ui_color_background);
		}
		virtual COLORREF defaultSelColor() final {
			return instance_callback->query_std_color(ui_color_selection);
		}
		virtual COLORREF defaultHighColor() final {
			return instance_callback->query_std_color(ui_color_highlight);
		}
		virtual COLORREF defaultHotColor() final {
			return instance_callback->getSysColor(COLOR_HOTLIGHT);
		}

		virtual LOGFONT defaultTitleFont() final {
			LOGFONT font = {};
			HFONT hfont = instance_callback->query_font_ex(ui_font_default);
			check(GetObject(hfont, sizeof(font), &font));
			return font;
		}

		virtual LOGFONT defaultListFont() final {
			LOGFONT font = {};
			HFONT hfont = instance_callback->query_font_ex(ui_font_lists);
			check(GetObject(hfont, sizeof(font), &font));
			return font;
		}

		virtual LOGFONT defaultPlaylistFont() final {
			LOGFONT font = {};
			HFONT hfont = instance_callback->query_font_ex(ui_font_playlists);
			check(GetObject(hfont, sizeof(font), &font));
			return font;
		}

	private:

		// Wrapper for win32 api calls to raise an exception on failure
		template <typename T>
		inline T check(T a) {
			if (a != NULL) {
				return a;
			}
			else {
				WIN32_OP_FAIL();
			}
		}

		ui_element_instance_callback::ptr instance_callback = nullptr;
	};
}
