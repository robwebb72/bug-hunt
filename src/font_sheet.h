
#ifndef __R72_OLC_FONT_SHEET__
#define __R72_OLC_FONT_SHEET__

#include <string>
#include <map>
#include "olcPixelGameEngine.h"



struct FontMetricChar;



class RWFont {

private:
	int char_count = 0;
	std::map<int, FontMetricChar*> font_map;

	void LoadFontMetrics(std::string filename);
	void DeleteFontMetrics();

	olc::Sprite* pSprite = nullptr;
	olc::Decal* pDecal = nullptr;

public:
	void LoadResources(std::string font_name);
	void Print(olc::PixelGameEngine* engine, olc::vi2d pos, std::string text, const olc::Pixel& tint = olc::WHITE);
	~RWFont();
};

#endif