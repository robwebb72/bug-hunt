
#include "level_map.h"
#include <iostream>
#include <fstream>
#include <map>

// SPRITE_TERRAIN is used to either draw the terrain using olc::Sprites or olc::Decals - decals are much faster but can cover over debugging info depending on how you display it
#define SPRITE_TERRAIN

namespace map {


	int GetFieldValue(std::string line, std::string field_name) {
		std::size_t pos = line.find(field_name);
		if (pos == std::string::npos) throw "Load level map: error reading field (" + field_name + ")";
		pos += field_name.size();
		try {
			return stoi(line.substr(pos));
		}
		catch (...) {
			throw "Load level map: error reading field (" + field_name + ")";
		}
	}


	std::string GetFieldString(std::string line, std::string field_name) {
		std::size_t pos1 = line.find(field_name);
		if (pos1 == std::string::npos) throw "Load level map: error reading field (" + field_name + ")";
		pos1 += field_name.size();
		std::size_t pos2 = line.find("\"", pos1);
		if (pos2 == std::string::npos) throw "Load level map: error reading field (" + field_name + ")";
		return (line.substr(pos1, pos2-pos1));
	}


	constexpr int FLOOR = 0;
	constexpr int BLOCK_PLAYER = 1;
	constexpr int BLOCK_CRITTER = 2;
	constexpr int BLOCK_PLAYER_BULLET = 4;
	constexpr int BLOCK_CRITTER_BULLET = 8;
	constexpr int WALL = 15;


	class Tile {

	public:
		TileSheet* tile_sheet;
		int x=0;
		int y=0;
		int type=0;

		Tile(TileSheet *tile_sheet, std::string types, int _x, int _y) {
			this->tile_sheet = tile_sheet;

			type |= (types.find("block_player") != std::string::npos) ? BLOCK_PLAYER : 0;
			type |= (types.find("block_player_bullet") != std::string::npos) ? BLOCK_PLAYER_BULLET : 0;
			type |= (types.find("block_alien") != std::string::npos) ? BLOCK_CRITTER : 0;
			type |= (types.find("block_alien_bullet") != std::string::npos) ? BLOCK_CRITTER_BULLET : 0;
			type |= (types.find("wall") != std::string::npos) ? WALL : 0;
			x = _x;
			y = _y;
		}
	};


	class MapBlock {
	public:
		Tile* pTile;

	};


	class TileSheet {

	private:
		void LoadSpriteSheet(std::string image_filename) {
			sheet_sprite = new olc::Sprite("resources\\levels\\" + image_filename);
			if (sheet_sprite->width == 0 || sheet_sprite->height == 0) throw "Error loading tile image: resources\\levels\\" + image_filename;
			sheet_decal = new olc::Decal(sheet_sprite);
		}


	public:
		olc::Sprite* sheet_sprite = nullptr;
		olc::Decal* sheet_decal = nullptr;
		int tile_sheet_columns= 0;
		std::map<int, Tile*> tiles;


		void LoadTSXFile(std::string filename) {

			std::ifstream tsx_file(filename);
			if (!tsx_file) throw "Error opening level map file: " + filename;

			std::string current_line;
			int line_number = 0;

			while (std::getline(tsx_file, current_line)) {
				if (current_line.find("<tileset") != std::string::npos) {
					tile_sheet_columns = map::GetFieldValue(current_line, "columns=\"");
					continue;
				}
				if (current_line.find("<image") != std::string::npos) {
					std::string image_filename = map::GetFieldString(current_line, "source=\"");
					LoadSpriteSheet(image_filename);
					continue;
				}
				if (current_line.find("<tile") != std::string::npos) {
					if (tile_sheet_columns == 0) throw "Error reading file: " + filename + " (columns is zero)";
					int tile_id = map::GetFieldValue(current_line, "id=\"");
					std::string tile_type = map::GetFieldString(current_line, "type=\"");
					tiles[tile_id] = new Tile(this, tile_type, tile_id % tile_sheet_columns, (int)tile_id / tile_sheet_columns);
					continue;
				}
			}
			if (tile_sheet_columns == 0) throw "Error reading file: " + filename + " (columns is zero)";
			if (sheet_sprite == nullptr) throw "Error reading file: " + filename + " (image file not loaded)";
		}


		void DestroyTiles() {
			for (auto item : tiles) {
				delete item.second;
			}
			tiles.clear();

		}


		void Destroy() {
			DestroyTiles();
			if (sheet_decal != nullptr) {
				delete sheet_decal;
				sheet_decal = nullptr;
			}
			if (sheet_sprite != nullptr) {
				delete sheet_sprite;
				sheet_sprite = nullptr;
			}
		}

	};


	void LevelMap::LoadMapBlockData(std::ifstream &file_stream) {
		if (map_size.x < 1) throw "Error map width cannot be less than 1";
		if (map_size.y < 1) throw "Error map height cannot be less than 1";
		if (map != nullptr) throw "Multiple data entries in level map files";
		map = new MapBlock[map_size.x * map_size.y];
		int tile_counter = 0;
		std::string current_line;
		while (std::getline(file_stream, current_line)) {

			if (current_line.find("</data>") != std::string::npos) break;
			std::stringstream stream(current_line);
			int val;
			while (stream >> val) {
				if (tile_counter >= map_size.x * map_size.y) throw "Error reading map data - too many values";
				auto it = tile_sheet->tiles.find(val - 1);
				if (it == tile_sheet->tiles.end()) throw "Error reading map - can't find tile value " + val;
				map[tile_counter++].pTile = it->second;
				if (stream.peek() == ',') stream.ignore();
			}
		}
	}


	void LevelMap::LoadMap(std::string resourcename) {

		std::ifstream tmx_file(resourcename+".tmx");
		if (!tmx_file) throw "Error opening level map file: " + resourcename+ ".tmx";
	
		map_size = { 0,0 };
		map = nullptr;

		std::string current_line;
		int line_number = 0;

		while (std::getline(tmx_file, current_line)) {
			if(current_line.find("<map") != std::string::npos) {
				map_size.x = map::GetFieldValue(current_line, "width=\"");
				map_size.y = map::GetFieldValue(current_line, "height=\"");
				tile_size.x = map::GetFieldValue(current_line, "tilewidth=\"");
				tile_size.y = map::GetFieldValue(current_line, "tileheight=\"");
				continue;
			}
			if (current_line.find("<tileset") != std::string::npos) {
				std::string filename = map::GetFieldString(current_line, "source=\"");
				tile_sheet = new TileSheet();
				tile_sheet->LoadTSXFile("resources/levels/" + filename);
				continue;
			}
			if(current_line.find("<data") != std::string::npos) {
				LoadMapBlockData(tmx_file);
			}
		}
		if (map == nullptr) throw "Error - no map data loaded";
		if (tile_sheet == nullptr) throw "Error - no tile data loaded";

	}


	void LevelMap::Initialise() {
	}


	void LevelMap::LoadResources(std::string resourcename) {
		LoadMap(resourcename);
		world_size = {map_size.x * tile_size.x, map_size.y * tile_size.y };

	}


	void LevelMap::Destroy() {
		tile_sheet->Destroy();
		delete [] map;
		map = nullptr;
		delete tile_sheet;
		tile_sheet = nullptr;
	}


	inline olc::vi2d LevelMap::WorldToMap(const olc::vf2d& worldcoords) {
		return { WorldXToMap(worldcoords.x), WorldYToMap(worldcoords.y) };
	}


	inline int LevelMap::WorldXToMap(float world_x) {
		return (int)(world_x /tile_size.x);
	}


	inline int LevelMap::WorldYToMap(float world_y) {
		return (int)(world_y / tile_size.y);
	}


	void LevelMap::DrawTerain(olc::PixelGameEngine *engine, olc::vf2d camera_pos) {

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
				int offset = j_pos * map_size.x + i_pos;
				MapBlock* mb = &map[offset];
				Tile* tile2 = mb->pTile;
				Tile * tile = map[j_pos * map_size.x + i_pos].pTile;
	//			int map_value = map[j_pos * map_size.x + i_pos];

				// TODO: [R:2020.07.11] This will need rewriting to take the new Tile class into account
				//int block_x = tile_size.x * (map_value % tile_sheet->tile_sheet_columns);
				//int block_y = tile_size.y * (map_value / tile_sheet->tile_sheet_columns);
				int block_x = tile_size.x * tile->x;
				int block_y = tile_size.y * tile->y;

				olc::vi2d screen_pos = { i * tile_size.x - x_offset_to_screen, (int)j * tile_size.y - y_offset_to_screen };

				int screen_pos_x = i * tile_size.x - x_offset_to_screen;
				int screen_pos_y = j * tile_size.y - y_offset_to_screen;
	#ifdef SPRITE_TERRAIN
				engine->DrawPartialSprite(
					{ (int) i * tile_size.x - x_offset_to_screen, (int) j * tile_size.y - y_offset_to_screen },
						tile->tile_sheet->sheet_sprite, 
						{ block_x, block_y }, 
						tile_size);
	#else
				engine->DrawPartialDecal({ (float) (i * tile_size.x - x_offset_to_screen), (float) (j * tile_size.y - y_offset_to_screen) },
					tile_sheet->sheet_decal,
					{ (float) block_x, (float) block_y },
					tile_size);
	#endif
				i_pos++;
			}
			j_pos++;
		}
	}


	bool LevelMap::IsMapPassable(int x, int y) {
		if (x < 0 || x >= map_size.x) return false;
		if (y < 0 || y >= map_size.y) return false;
		return (map[(y * map_size.x) + x].pTile->type & BLOCK_PLAYER) == 0;
	}


	olc::vf2d LevelMap::CollisionWithTerrain(const olc::vf2d& position, const olc::vf2d& delta, const olc::vi2d& bb_tl, const olc::vi2d& bb_br) {

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

}