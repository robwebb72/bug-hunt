
#include "level_map.h"
#include <iostream>
#include <fstream>



class TileSheet {

};


class LevelMap {


private:
	int* map;

	// MAP INFO
	olc::vi2d map_size;

	// TILE SHEET INFO
	int tile_sheet_columns;
	olc::vi2d tile_size;
	olc::Sprite* tile_sheet_sprite;
	olc::Decal* tile_sheet_decal;


	void LoadTileSheet(std::string filename) {
		tile_sheet_columns = 20;
		tile_size = { 16, 16 };
		tile_sheet_sprite = nullptr;
		tile_sheet_decal = nullptr;
		tile_sheet_sprite = new olc::Sprite(filename);
		tile_sheet_decal = new olc::Decal(tile_sheet_sprite);
	}

	void DestroyTileSheet() {
		if(tile_sheet_decal!= nullptr) {
			delete tile_sheet_decal;
			tile_sheet_decal = nullptr;
		}
		if(tile_sheet_sprite!= nullptr) {
			delete tile_sheet_sprite;
			tile_sheet_sprite = nullptr;
		}
	}

	int GetFieldValue(std::string line, std::string field_name) {
		std::size_t pos = 0;
		pos = line.find(field_name);
		if (pos == std::string::npos) throw "TMX file: error reading field (" + field_name + ")";
		pos += field_name.size();
		return stoi(line.substr(pos));
	}

	std::string GetFieldString(std::string line, std::string field_name) {
		std::size_t pos1 = 0;
		std::size_t pos2 = 0;
		pos1 = line.find(field_name);
		if (pos1 == std::string::npos) throw "TMX file: error reading field (" + field_name + ")";
		pos1 += field_name.size();
		pos2 = line.find("\"", pos1)-pos1;
		return (line.substr(pos1, pos2));
	}

	void LoadMap(std::string resourcename) {
	
		map_size = { 0,0 };
		map = nullptr;
		LoadTileSheet(resourcename + "-tiles.png");

		std::ifstream tmx_file(resourcename+".tmx");
		if (!tmx_file) throw "Error opening level map file: " + resourcename+ ".tmx";
		
		std::string current_line;
		int line_number = 0;

		while (std::getline(tmx_file, current_line)) {

			if(current_line.find("<map") != std::string::npos) {
				map_size.x = GetFieldValue(current_line, "width=\"");
				map_size.y = GetFieldValue(current_line, "height=\"");
				continue;
			}
			if (current_line.find("<tileset") != std::string::npos) {
				std::string filename = GetFieldString(current_line, "source=\"");
				// TODO: [RW:2020-07-10] Load the tile set information here
				continue;
			}
			if(current_line.find("<data") != std::string::npos) {
				if (map_size.x < 1) throw "Error map width cannot be less than 1";
				if (map_size.y < 1) throw "Error map height cannot be less than 1";

				map = new int[(int)map_size.x * map_size.y];
				int tile_counter = 0;
				while (std::getline(tmx_file, current_line)) {

					if (current_line.find("</data>") != std::string::npos) break;

					std::stringstream stream(current_line);
					int val;
					while (stream >> val) {
						if (tile_counter >= map_size.x * map_size.y) throw "Error reading map data - too many values";
						map[tile_counter++] = val-1;
						if (stream.peek() == ',') stream.ignore();
					}
				}
			}
		}

		if (map == nullptr) throw "Error - no map data loaded";
	}


public:

	olc::vi2d world_size;

	void Initialise() {
	}

	void LoadResources(std::string resourcename) {
		LoadMap(resourcename);
		world_size = {map_size.x * tile_size.x, map_size.y * tile_size.y };

	}

	void Destroy() {
		DestroyTileSheet();
		delete [] map;
		map = nullptr;
	}

// ========================================================================================================================
// START     Terrain
// ========================================================================================================================


	olc::vi2d WorldToMap(const olc::vf2d& worldcoords) {
		return { WorldXToMap(worldcoords.x), WorldYToMap(worldcoords.y) };
	}

	int WorldXToMap(float world_x) {
		return (int)(world_x /tile_size.x);
	}

	int WorldYToMap(float world_y) {
		return (int)(world_y / tile_size.y);
	}

	void DrawTerain(olc::PixelGameEngine *engine, olc::vf2d camera_pos) {

		int camera_x = (int) floor(camera_pos.x);
		int camera_y = (int) floor(camera_pos.y);

		olc::vi2d map_start = WorldToMap(camera_pos);

		int map_start_x_block = map_start.x;
		int map_start_y_block = map_start.y;

		int x_offset_to_screen = camera_x % tile_size.x;
		int y_offset_to_screen = camera_y % tile_size.y;

		int map_screen_width = engine->ScreenWidth() / tile_size.x;
		int map_screen_height = engine->ScreenHeight() / tile_size.y;
		if (x_offset_to_screen != 0) map_screen_width++;	// if spanning a partial tile, we need to draw an extra tile
		if (y_offset_to_screen != 0) map_screen_height++;
		if (map_start_x_block + map_screen_width > map_size.x) map_screen_width = map_size.x - map_start_x_block;
		if (map_start_y_block + map_screen_height > map_size.y) map_screen_height = map_size.y - map_start_y_block;

		int j_pos = map_start_y_block;
		for (int j = 0; j < map_screen_height; j++) {
			int i_pos = map_start_x_block;
			for (int i = 0; i < map_screen_width; i++) {
				int map_value = map[j_pos * map_size.x + i_pos];
				int block_x = tile_size.x * (map_value % tile_sheet_columns);
				int block_y = tile_size.y * (map_value / tile_sheet_columns);
				olc::vi2d screen_pos = { i * tile_size.x - x_offset_to_screen, (int)j * tile_size.y - y_offset_to_screen };


#ifdef SPRITE_TERRAIN
				engine->DrawPartialSprite(
					{ i * tile_size.x - x_offset_to_screen, (int) j * tile_size.y - y_offset_to_screen },
						tile_sheet_sprite, 
						{ block_x, block_y }, 
						tile_size);
#else
				engine->DrawPartialDecal({ (float) (i * tile_size.x - x_offset_to_screen), (float) (j * tile_size.y - y_offset_to_screen) },
					tile_sheet_decal,
					{ (float) block_x, (float) block_y },
					tile_size);
#endif
				i_pos++;
			}
			j_pos++;
		}
	}

// ========================================================================================================================
// END     Terrain
// ========================================================================================================================


// ========================================================================================================================
// START   Collision with Terrain
// ========================================================================================================================


	bool IsMapPassable(int x, int y) {
		if (x < 0 || x >= map_size.x) return false;
		if (y < 0 || y >= map_size.y) return false;
		int map_value = map[(y * map_size.x) + x];
		return (map_value < 20 || map_value>= 40);
	}


	olc::vf2d CollisionWithTerrain(const olc::vf2d& position, const olc::vf2d& delta, const olc::vi2d& bb_tl, const olc::vi2d& bb_br) {

		float dx_orig = delta.x;
		float dy_orig = delta.y;
		float dx_new = dx_orig;
		float dy_new = dy_orig;

		if (dx_orig != 0.0f) {	
			float bounding_box_x = position.x;
			bounding_box_x += (dx_orig < 0) ? bb_tl.x : bb_br.x;
			int map_block_x = WorldXToMap(bounding_box_x + dx_orig);
			int map_block_y_top = WorldYToMap(position.y + bb_tl.y);
			int map_block_y_btm = WorldYToMap(position.y + bb_br.y);

			bool blocked = false;
			for (int map_block_y = map_block_y_top; map_block_y <= map_block_y_btm; map_block_y++) {
				if (!IsMapPassable(map_block_x, map_block_y)) {
					blocked = true;
					break;
				}
			}
			if (blocked) {
				if (dx_orig < 0)
					dx_new = (float)((map_block_x + 1) * tile_size.x) - bounding_box_x;
				else
					dx_new = (float)((map_block_x) *tile_size.x -1) - bounding_box_x;
			}
		}

		if (dy_orig != 0.0f) {
			float bounding_box_y = position.y;
			bounding_box_y += (dy_orig < 0) ? bb_tl.y : bb_br.y;

			int map_block_y = WorldYToMap(bounding_box_y + dy_orig);
			int map_block_x_left = WorldXToMap(position.x + bb_tl.x);
			int map_block_x_right = WorldXToMap(position.x + bb_br.x);

			bool blocked = false;
			for (int map_block_x = map_block_x_left; map_block_x <= map_block_x_right; map_block_x++) {
				if (!IsMapPassable(map_block_x, map_block_y)) {
					blocked = true;
					break;
				}
			}
			if (blocked) {
				if (dy_orig < 0)
					dy_new = (float)((map_block_y + 1) * tile_size.y) - bounding_box_y;
				else
					dy_new = (float)((map_block_y)*tile_size.y - 1) - bounding_box_y;
			}
		}

		// if we are still moving diagonally after checking horizontal and vertical collision, best check the diagonal direction
		if (dx_new != 0 && dy_new != 0) {
			float worldX = dx_new + position.x;
			worldX += (float) (dx_new < 0) ? bb_tl.x : bb_br.x;
			float worldY = dy_new + position.y;
			worldY += (float) (dy_new < 0) ? bb_tl.y : bb_br.y;
			int map_block_x = WorldXToMap(worldX);
			int map_block_y = WorldYToMap(worldY);
			if(!IsMapPassable(map_block_x, map_block_y)) {
				dx_new = 0;		// only reset dx_new, setting dy_new as well stops the movement completely
			}
		}
		return { dx_new, dy_new };
	}

// ========================================================================================================================
// END     Collision with Terrain
// ========================================================================================================================
};
