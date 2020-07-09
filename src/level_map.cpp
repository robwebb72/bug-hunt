
#include "olcPixelGameEngine.h"

#include "map_layout.cpp"

class LevelMap {


private:
	olc::Sprite* terrain_blocks_spr;
	olc::Decal* terrain_blocks_dec;


	void LoadTerrain(std::string filename) {
		terrain_blocks_spr = new olc::Sprite(filename);
		terrain_blocks_dec = new olc::Decal(terrain_blocks_spr);
	}

	void DestroyTerrain() {
		if(terrain_blocks_dec!= nullptr) {
			delete terrain_blocks_dec;
			terrain_blocks_dec = nullptr;
		}
		if(terrain_blocks_spr!= nullptr) {
			delete terrain_blocks_spr;
			terrain_blocks_spr = nullptr;
		}
	}

public:

	int map_width = 100;
	int map_height = 100;

	int terrain_tile_width = 16;
	int terrain_tile_height = 16;
	int tile_sheet_tiles_per_row = 20;
	olc::vi2d map_size { map_width, map_height };
	olc::vi2d world_size { map_size.x * terrain_tile_width, map_size.y * terrain_tile_height };
	olc::vi2d terrain_block_size { terrain_tile_width, terrain_tile_height };




	void Initialise() {
		terrain_blocks_spr = nullptr;
		terrain_blocks_dec = nullptr;
		LoadTerrain("resources/blocks.png");
	}


	void Destroy() {
		DestroyTerrain();
	}

// ========================================================================================================================
// START     Terrain
// ========================================================================================================================


	olc::vi2d WorldToMap(const olc::vf2d& worldcoords) {
		return { WorldXToMap(worldcoords.x), WorldYToMap(worldcoords.y) };
	}

	int WorldXToMap(float world_x) {
		return (int)(world_x / terrain_tile_width);
	}

	int WorldYToMap(float world_y) {
		return (int)(world_y / terrain_tile_height);
	}

	void DrawTerain(olc::PixelGameEngine *engine, olc::vf2d camera_pos) {

		int camera_x = (int) floor(camera_pos.x);
		int camera_y = (int) floor(camera_pos.y);

		olc::vi2d map_start = WorldToMap(camera_pos);

		int map_start_x_block = map_start.x;
		int map_start_y_block = map_start.y;

		int x_offset_to_screen = camera_x % terrain_tile_width;
		int y_offset_to_screen = camera_y % terrain_tile_height;

		int map_screen_width = engine->ScreenWidth() / terrain_tile_width;
		int map_screen_height = engine->ScreenHeight() / terrain_tile_height;
		if (x_offset_to_screen != 0) map_screen_width++;	// if spanning a partial tile, we need to draw an extra tile
		if (y_offset_to_screen != 0) map_screen_height++;
		if (map_start_x_block + map_screen_width > map_size.x) map_screen_width = map_size.x - map_start_x_block;
		if (map_start_y_block + map_screen_height > map_size.y) map_screen_height = map_size.y - map_start_y_block;

		int j_pos = map_start_y_block;
		for (int j = 0; j < map_screen_height; j++) {
			int i_pos = map_start_x_block;
			for (int i = 0; i < map_screen_width; i++) {

				int map_value = map[j_pos * map_width + i_pos]-1;
				int block_x = terrain_tile_width * (map_value % tile_sheet_tiles_per_row);
				int block_y = terrain_tile_height * (map_value / tile_sheet_tiles_per_row);

				engine->DrawPartialSprite(
					{ i * terrain_tile_width - x_offset_to_screen, (int) j * terrain_tile_height - y_offset_to_screen },
						terrain_blocks_spr, 
						{ block_x, block_y }, 
						terrain_block_size);
				//engine->DrawPartialDecal({ (float) (i * terrain_tile_width - x_offset_to_screen), (float) (j * terrain_tile_height - y_offset_to_screen) },
				//	terrain_blocks_dec,
				//	{ (float) block_x, (float) block_y },
				//	terrain_block_size);
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
		int map_value = map[y * map_size.x + x] - 1;
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
					dx_new = (float)((map_block_x + 1) * terrain_tile_width) - bounding_box_x;
				else
					dx_new = (float)((map_block_x) * terrain_tile_width -1) - bounding_box_x;
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
					dy_new = (float)((map_block_y + 1) * terrain_tile_height) - bounding_box_y;
				else
					dy_new = (float)((map_block_y)*terrain_tile_height - 1) - bounding_box_y;
			}
		}
		return { dx_new, dy_new };
	}

// ========================================================================================================================
// END     Collision with Terrain
// ========================================================================================================================
};
