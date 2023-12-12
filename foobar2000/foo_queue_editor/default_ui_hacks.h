#pragma once

#include "stdafx.h"


class default_ui_hacks {
// http://www.hydrogenaudio.org/forums/index.php?showtopic=78644

public:
	static t_uint32 DetermineSelectedTextColor(t_uint32 bk) {
		// Original threshold in the thread was 0.6 but it didn't work for one of the default themes, so therefore
		// I changed it to 0.7.
		return Luminance(bk) > 0.7 ? 0 : 0xFFFFFF;
	}
	static t_uint32 DetermineAlternativeBackgroundColor(t_uint32 bkColor_base) {
		return DriftColor(bkColor_base,4, 0);;
	}

private:
	static double Luminance(t_uint32 color) {
		double r = extractbyte(color,0), g = extractbyte(color,1), b = extractbyte(color,2);
		return (0.2126 * r + 0.7152 * g + 0.0722 * b) / 255.0;
		//return (r * 0.3 + g * 0.59 + b * 0.11) / 255.0;
	}

	static t_uint32 DriftColor(t_uint32 p_color,unsigned p_delta,bool p_direction) throw() {
		t_uint32 ret = 0;
		for(t_size walk = 0; walk < 3; ++walk) {
			unsigned val = extractbyte(p_color, walk);
			if (p_direction) val = 0xFF - val;
			if (val < p_delta) val = p_delta;
			val += p_delta;
			if (val > 0xFF) val = 0xFF;
			if (p_direction) val = 0xFF - val;
			ret |= (t_uint32)val << (walk * 8);
		}
		return ret;
	}

	static t_uint16 extractbyte(t_uint32 word, t_uint16 pos){
		t_uint16 result;
		if(pos == 0) {
			result = word & 0xFF;
		}else{
			result = (word >> 8*pos) & 0xFF;
		}

		return result;
	}
};

