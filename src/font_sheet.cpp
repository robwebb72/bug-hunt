
#ifndef __R72_OLC_FONT_SHEET__
#define __R72_OLC_FONT_SHEET__

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include "olcPixelGameEngine.h"


// TODO: [2020-06-29]  This code is rather fragile and might fall down if the file is in an unexpected format.  Add error handling!  RW.

struct FontMetricChar {

private:

	olc::Sprite* pSprite = nullptr;
	olc::Decal* pDecal = nullptr;

	int GetFieldValue(std::string line, std::string field_name) {
		std::size_t pos = 0;
		pos = line.find(field_name);
		if (pos == std::string::npos) throw "Font metric: error reading field (" + field_name + ")";
		pos += field_name.size();
		return stoi(line.substr(pos));
	}

public:
	int id = 0;
	int x = 0;
	int y = 0;
	int width = 0;
	int height = 0;
	int xoffset = 0;
	int yoffset = -1;
	int xadvance = 0;

	FontMetricChar(const FontMetricChar& other) : id(other.id), x(other.x), y(other.y),
		width(other.width), height(other.height),
		xoffset(other.xoffset), yoffset(other.yoffset),
		xadvance(other.xadvance)
	{}


	FontMetricChar(std::string line) {
		id = GetFieldValue(line, "char id=");
		x = GetFieldValue(line, "x=");
		y = GetFieldValue(line, "y=");
		width = GetFieldValue(line, "width=");
		height = GetFieldValue(line, "height=");
		xoffset = GetFieldValue(line, "xoffset=");
		yoffset = GetFieldValue(line, "yoffset=");
		xadvance = GetFieldValue(line, "xadvance=");
	}

};


class RWFont {

private:
	int char_count = 0;
	std::map<int, FontMetricChar*> font_map;

	void LoadFontMetrics(std::string filename) {
		std::ifstream metric_file(filename);
		if (!metric_file) throw "Error opening font metric file: " + filename;
		std::string current_line;
		int line_number = 0;

		while (std::getline(metric_file, current_line)) {
			std::size_t found_pos = current_line.find("char id");
			if (found_pos == std::string::npos) continue;
			try {
				FontMetricChar char_metric(current_line);
				if (char_metric.id < 32 || char_metric.id > 255) continue;
				font_map[char_metric.id] = new FontMetricChar(char_metric);
				char_count++;
			}
			catch (std::string& error) {
				throw error + " file: " + filename + " (line:" + std::to_string(line_number)+")";
			}
		}
		if (char_count < 96) {
			DeleteFontMetrics();
			throw "Error loading font metric file: " + filename + " ( <96 characters found)";
		}
	}

	void DeleteFontMetrics() {
		for (std::map<int, FontMetricChar*>::iterator it = font_map.begin(); it != font_map.end(); ++it) {
			delete it->second;
		}
		font_map.clear();
	}

public:
	RWFont() {
	}

	void LoadResources(std::string font_name) {
		LoadFontMetrics(font_name + ".fnt");
	}


	~RWFont() {
		DeleteFontMetrics();
	}
};








#endif