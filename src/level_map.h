#ifndef __R72_OLC_LEVEL_MAP__
#define __R72_OLC_LEVEL_MAP__

#include "olcPixelGameEngine.h"


// SPRITE_TERRAIN is used to either draw the terrain using olc::Sprites or olc::Decals - decals are much faster but can cover over debugging info depending on how you display it
#define SPRITE_TERRAIN


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


	void LoadTileSheet(std::string filename);
	void DestroyTileSheet();
	int GetFieldValue(std::string line, std::string field_name);
	std::string GetFieldString(std::string line, std::string field_name);
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

#endif
