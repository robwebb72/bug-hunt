#ifndef __R72_OLC_LEVEL_MAP__
#define __R72_OLC_LEVEL_MAP__

#include "olcPixelGameEngine.h"


// SPRITE_TERRAIN is used to either draw the terrain using olc::Sprites or olc::Decals - decals are much faster but can cover over debugging info depending on how you display it
#define SPRITE_TERRAIN

namespace map {
	class TileSheet;
	class MapBlock;

	class LevelMap {


	private:
		MapBlock* map;

		// MAP INFO
		olc::vi2d map_size;

		// TILE SHEET INFO
		TileSheet* tile_sheet = nullptr;

		int tile_sheet_columns;
		olc::vi2d tile_size;


		void DestroyTileSheet();
		void LoadMap(std::string resourcename);

		olc::vi2d WorldToMap(const olc::vf2d& worldcoords);
		int WorldXToMap(float world_x);
		int WorldYToMap(float world_y);
		bool IsMapPassable(int x, int y);

	public:

		olc::vi2d world_size;
		void Initialise();
		void LoadResources(std::string resourcename);
		void Destroy();

		void DrawTerain(olc::PixelGameEngine* engine, olc::vf2d camera_pos);
		olc::vf2d CollisionWithTerrain(const olc::vf2d& position, const olc::vf2d& delta, const olc::vi2d& bb_tl, const olc::vi2d& bb_br);
	};
}
#endif
