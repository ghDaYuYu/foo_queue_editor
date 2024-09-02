#pragma once

#include "stdafx.h"
#include "ui_element_base.h"
#include "../columns_ui-sdk/ui_extension.h"

// {BCC2BC7F-49F3-4D70-ADC1-62B81349FD90}
static const GUID cui_guid =
{ 0xbcc2bc7f, 0x49f3, 0x4d70, { 0xad, 0xc1, 0x62, 0xb8, 0x13, 0x49, 0xfd, 0x90 } }; //mod

// {CAA2E834-7830-4DE5-BC24-B9CF39EA6B20}
static const GUID cui_font_client_guid =
{ 0xcaa2e834, 0x7830, 0x4de5, { 0xbc, 0x24, 0xb9, 0xcf, 0x39, 0xea, 0x6b, 0x20 } }; //mod

// {72FFF50A-E674-4ADE-83B0-CA8CC271437E}
static const GUID cui_colours_client_guid =
{ 0x72fff50a, 0xe674, 0x4ade, { 0x83, 0xb0, 0xca, 0x8c, 0xc2, 0x71, 0x43, 0x7e } }; //mod

class queue_editor_cui_colours_client : public columns_ui::colours::client {
	
	virtual void get_name(pfc::string_base & out) const {
		out.set_string(COMPONENT_NAME_H);
	}

	virtual const GUID & get_client_guid() const {
		return cui_colours_client_guid;
	}

	uint32_t get_supported_bools() const { // bit-mask
		return 0;
	}

	void on_colour_changed(uint32_t changed_items_mask) const override {
		//todo...
	}
	void on_bool_changed(uint32_t changed_items_mask) const override {
		//todo...
	}

	virtual bool get_themes_supported() const {
		return false;
	}
#ifdef x64
	virtual void on_colour_changed(t_size mask) const {
		window_manager::VisualsChanged();
	}
	
	virtual void on_bool_changed(t_size mask) const {
		//todo...
	}
#endif
};
class queue_editor_cui_fonts_client : public columns_ui::fonts::client  {

	virtual void get_name(pfc::string_base & out) const {
		out.set_string(COMPONENT_NAME_HC);
	}
	
	virtual const GUID & get_client_guid() const {
		return cui_font_client_guid;
	}

	virtual columns_ui::fonts::font_type_t get_default_font_type() const {
		return columns_ui::fonts::font_type_items;
	}

	virtual void on_font_changed() const {
		window_manager::VisualsChanged();
	}
};

class cui_element : public ui_element_base, public ui_extension::container_ui_extension { 
//WS_EX_CONTROLPARENT 
private:
	static const GUID extension_guid;
	virtual class_data & get_class_data() const {
		__implement_get_class_data(_T("{6B864D3B-70FB-4990-B44C-AED3F2C3FCAE}"), false); //mod
	}
	LRESULT on_message(HWND wnd, UINT msg, WPARAM wp, LPARAM lp);
public:
	// ui_element_base
	virtual void RefreshVisuals();
	virtual void HideHeader();
	virtual void save_configuration();
	virtual HWND get_wnd();
	HWND get_wnd() const;
	virtual bool is_dui() override;


	virtual const GUID & get_extension_guid() const	{
		return extension_guid;
	}

	virtual void get_name(pfc::string_base & out) const {
		out.set_string(COMPONENT_NAME_HC);
	}

	virtual void get_category(pfc::string_base & out)const {
		out.set_string("Panels");
	}

	unsigned get_type () const {
		/** In this case we are only of type type_panel */
		return ui_extension::type_panel;
	}

	virtual bool have_config_popup(){return false;}
	virtual bool show_config_popup(HWND wnd_parent){return false;};

	// From ui_extension::extension_base
	virtual void set_config(stream_reader * p_reader, t_size p_size, abort_callback & p_abort){
		stream_reader_formatter<false> reader(*p_reader, p_abort);
		if(p_size == 0) {
			// Defaults
			ui_element_settings defaults;
			m_settings = defaults;
			m_settings.m_columns.remove_all();
			if(cfg_ui_columns.first().is_valid()) {
				m_settings.m_columns.add_item(ui_column_settings(cfg_ui_columns.first()->m_key));
			} else {
				// object.m_columns is empty
			}
		}
		reader >> m_settings;

	};
	virtual void get_config(stream_writer * p_writer, abort_callback & p_abort) const {
		stream_writer_formatter<false> writer(*p_writer, p_abort);
		writer << m_settings;
	};
};
