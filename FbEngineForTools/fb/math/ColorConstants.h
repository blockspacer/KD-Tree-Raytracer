#ifndef FB_MATH_COLORCONSTANTS_H
#define FB_MATH_COLORCONSTANTS_H

FB_PACKAGE1(math)

#define COLOR_COMP_MAX_IMPL 1.0f
#define COLOR_COMP_MID_IMPL 0.5f
#define COLOR_COMP_MID_25_IMPL 0.25f
#define COLOR_COMP_MID_75_IMPL 0.75f

// a macro for compacting the use of the constants...
#define FB_COLC(p_color) fb::math::ColorConstants::get##p_color()

/**
 * These are our generic base colors. 
 * These are the ones that are basically derived from R,G,B channels on/off permutations and grays.
 * (Black, Red, Green, Blue, Yellow, Cyan, Magenta, White + Gray)
 *
 * This does NOT define some fancy web color constants like "Honeydew", etc. :)
 *
 * To quickly create other colors, try using the mixColorsRGB(...) (or alternatively mixColorsHSV) with some of these.
 * Say, if you want the orange, try mixColorsRGB(ColorConstants::getRed(), ColorConstants::getYellow())
 */
class ColorConstants
{
public:
	// safe to use in static initialization versions

	// black and white
	static inline COL getBlack() { return COL(0, 0, 0); }
	static inline COL getWhite() { return COL(COLOR_COMP_MAX_IMPL, COLOR_COMP_MAX_IMPL, COLOR_COMP_MAX_IMPL); }

	// gray
	static inline COL getGray() { return COL(COLOR_COMP_MID_IMPL, COLOR_COMP_MID_IMPL, COLOR_COMP_MID_IMPL); }

	// the usual max intensity colors
	static inline COL getRed() { return COL(COLOR_COMP_MAX_IMPL, 0, 0); }
	static inline COL getGreen() { return COL(0, COLOR_COMP_MAX_IMPL, 0); }
	static inline COL getBlue() { return COL(0, 0, COLOR_COMP_MAX_IMPL); }

	static inline COL getYellow() { return COL(COLOR_COMP_MAX_IMPL, COLOR_COMP_MAX_IMPL, 0); }
	static inline COL getCyan() { return COL(0, COLOR_COMP_MAX_IMPL, COLOR_COMP_MAX_IMPL); }
	static inline COL getMagenta() { return COL(COLOR_COMP_MAX_IMPL, 0, COLOR_COMP_MAX_IMPL); }

	// darker versions of gray and the max intensity colors
	static inline COL getDarkGray() { return COL(COLOR_COMP_MID_25_IMPL, COLOR_COMP_MID_25_IMPL, COLOR_COMP_MID_25_IMPL); }

	static inline COL getDarkRed() { return COL(COLOR_COMP_MID_IMPL, 0, 0); }
	static inline COL getDarkGreen() { return COL(0, COLOR_COMP_MID_IMPL, 0); }
	static inline COL getDarkBlue() { return COL(0, 0, COLOR_COMP_MID_IMPL); }

	static inline COL getDarkYellow() { return COL(COLOR_COMP_MID_IMPL, COLOR_COMP_MID_IMPL, 0); }
	static inline COL getDarkCyan() { return COL(0, COLOR_COMP_MID_IMPL, COLOR_COMP_MID_IMPL); }
	static inline COL getDarkMagenta() { return COL(COLOR_COMP_MID_IMPL, 0, COLOR_COMP_MID_IMPL); }

	// lighter versions of gray and the max intensity colors
	static inline COL getBrightGray() { return COL(COLOR_COMP_MID_75_IMPL, COLOR_COMP_MID_75_IMPL, COLOR_COMP_MID_75_IMPL); }

	static inline COL getBrightRed() { return COL(COLOR_COMP_MAX_IMPL, COLOR_COMP_MID_IMPL, COLOR_COMP_MID_IMPL); }
	static inline COL getBrightGreen() { return COL(COLOR_COMP_MID_IMPL, COLOR_COMP_MAX_IMPL, COLOR_COMP_MID_IMPL); }
	static inline COL getBrightBlue() { return COL(COLOR_COMP_MID_IMPL, COLOR_COMP_MID_IMPL, COLOR_COMP_MAX_IMPL); }

	static inline COL getBrightYellow() { return COL(COLOR_COMP_MAX_IMPL, COLOR_COMP_MAX_IMPL, COLOR_COMP_MID_IMPL); }
	static inline COL getBrightCyan() { return COL(COLOR_COMP_MID_IMPL, COLOR_COMP_MAX_IMPL, COLOR_COMP_MAX_IMPL); }
	static inline COL getBrightMagenta() { return COL(COLOR_COMP_MAX_IMPL, COLOR_COMP_MID_IMPL, COLOR_COMP_MAX_IMPL); }

	// the darkest versions of gray and the max intensity colors
	//static inline COL getVeryDarkGray() { return COL(COLOR_COMP_MID_25_IMPL * 0.5f, COLOR_COMP_MID_25_IMPL * 0.5f, COLOR_COMP_MID_25_IMPL * 0.5f); }

	static inline COL getVeryDarkRed() { return COL(COLOR_COMP_MID_25_IMPL, 0, 0); }
	static inline COL getVeryDarkGreen() { return COL(0, COLOR_COMP_MID_25_IMPL, 0); }
	static inline COL getVeryDarkBlue() { return COL(0, 0, COLOR_COMP_MID_25_IMPL); }

	static inline COL getVeryDarkYellow() { return COL(COLOR_COMP_MID_25_IMPL, COLOR_COMP_MID_25_IMPL, 0); }
	static inline COL getVeryDarkCyan() { return COL(0, COLOR_COMP_MID_25_IMPL, COLOR_COMP_MID_25_IMPL); }
	static inline COL getVeryDarkMagenta() { return COL(COLOR_COMP_MID_25_IMPL, 0, COLOR_COMP_MID_25_IMPL); }

	// the lightest versions of the max intensity colors
	static inline COL getVeryBrightRed() { return COL(COLOR_COMP_MAX_IMPL, (COLOR_COMP_MID_IMPL + COLOR_COMP_MAX_IMPL) * 0.5f, (COLOR_COMP_MID_IMPL + COLOR_COMP_MAX_IMPL) * 0.5f); }
	static inline COL getVeryBrightGreen() { return COL((COLOR_COMP_MID_IMPL + COLOR_COMP_MAX_IMPL) * 0.5f, COLOR_COMP_MAX_IMPL, (COLOR_COMP_MID_IMPL + COLOR_COMP_MAX_IMPL) * 0.5f); }
	static inline COL getVeryBrightBlue() { return COL((COLOR_COMP_MID_IMPL + COLOR_COMP_MAX_IMPL) * 0.5f, (COLOR_COMP_MID_IMPL + COLOR_COMP_MAX_IMPL) * 0.5f, COLOR_COMP_MAX_IMPL); }

	static inline COL getVeryBrightYellow() { return COL(COLOR_COMP_MAX_IMPL, COLOR_COMP_MAX_IMPL, (COLOR_COMP_MID_IMPL + COLOR_COMP_MAX_IMPL) * 0.5f); }
	static inline COL getVeryBrightCyan() { return COL((COLOR_COMP_MID_IMPL + COLOR_COMP_MAX_IMPL) * 0.5f, COLOR_COMP_MAX_IMPL, COLOR_COMP_MAX_IMPL); }
	static inline COL getVeryBrightMagenta() { return COL(COLOR_COMP_MAX_IMPL, (COLOR_COMP_MID_IMPL + COLOR_COMP_MAX_IMPL) * 0.5f, COLOR_COMP_MAX_IMPL); }
};


FB_END_PACKAGE1()

#endif
