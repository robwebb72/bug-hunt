#ifndef __R72_OLC_LEVEL_MAP__
#define __R72_OLC_LEVEL_MAP__

#include "olcPixelGameEngine.h"




namespace map {
	class TileSheet;
	class MapBlock;

	class LevelMap {

	private:
		MapBlock* map = nullptr;
		olc::vi2d map_size;
		TileSheet* tile_sheet = nullptr;
		olc::vi2d tile_size;

		void LoadMap(std::string resourcename);
		void LoadMapBlockData(std::ifstream & file_stream);

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
