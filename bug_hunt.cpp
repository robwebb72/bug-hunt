#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "Zix_PGE_Controller.h"
#include "map.cpp"
#include "anim_sprite.cpp"
//#define SHOW_DEBUG_INFO

olc::vi2d screen_origin = { 0, 0 };
olc::vi2d screen_size = { 640, 400 };


float bindToRange(float value, float min, float max) {
	return value < min ? min : (value > max ? max : value);
}

int bindToRange(int value, int min, int max) {
	return value < min ? min : (value > max ? max : value);
};

class BugHunt : public olc::PixelGameEngine
{	
	// input controls
	ControllerManager controller;
	enum olc::Key key_up = olc::UP;
	enum olc::Key key_down = olc::DOWN;
	enum olc::Key key_left = olc::LEFT;
	enum olc::Key key_right = olc::RIGHT;
	enum olc::Key key_reverse = olc::SHIFT;
	enum olc::Key key_fire = olc::SPACE;

	// screen info
	olc::vi2d screen_centre = screen_size / 2;

	// player location & direction
	enum PlayerState { idling=0, moving=1, firing=2} player_state = idling;
	const int player_direction_none = 4;
	olc::vf2d player_pos = { 230.0f,140.0f };
	float player_speed = 180.0f;
	int player_direction = 7;
	bool player_reverse = false;

	// player sprite
	AnimatedSprite* playerSprite;
	olc::vi2d player_sprite_size{ 32, 32 };
	olc::vi2d sprite_offset { player_sprite_size.x / 2, player_sprite_size.y / 2 };
	float player_idling_anim_speed = 0.15f;
	float player_moving_anim_speed = 0.10f;
	float player_firing_anim_speed = 0.08f;
	
	// terrain
	olc::Sprite* terrain_blocks_spr;
	olc::Decal* terrain_blocks_dec;

	olc::vf2d camera_pos = { 0.0f, 0.0f };

public:
	BugHunt()
	{
		sAppName = "Bug Hunt";
	}


	void InitialisePlayerSprite()
	{
		playerSprite = new AnimatedSprite("resources/gent.png");
		playerSprite->SetFrameSize({ 32, 32 });
		playerSprite->AddAnimation(player_idling_anim_speed, { {0,  0}, {32,  0}, {64,  0} });	// 0 : idle nw
		playerSprite->AddAnimation(player_idling_anim_speed, { {0, 32}, {32, 32}, {64, 32} });	// 1 : idle n
		playerSprite->AddAnimation(player_idling_anim_speed, { {0, 64}, {32, 64}, {64, 64} });	// 2 : idle ne
		playerSprite->AddAnimation(player_idling_anim_speed, { {0, 96}, {32, 96}, {64, 96} });	// 3 : idle w
		playerSprite->AddAnimation(player_idling_anim_speed, { {0,128}, {32,128}, {64,128} });	// 4 : idle --
		playerSprite->AddAnimation(player_idling_anim_speed, { {0,160}, {32,160}, {64,160} });	// 5 : idle e
		playerSprite->AddAnimation(player_idling_anim_speed, { {0,192}, {32,192}, {64,192} });	// 6 : idle sw
		playerSprite->AddAnimation(player_idling_anim_speed, { {0,224}, {32,224}, {64,224} });	// 7 : idle s
		playerSprite->AddAnimation(player_idling_anim_speed, { {0,256}, {32,256}, {64,256} });	// 8 : idle se
		playerSprite->AddAnimation(player_moving_anim_speed, { { 96,  0}, {128,  0}, {160,  0}, {192,  0} }); //  9 : move nw
		playerSprite->AddAnimation(player_moving_anim_speed, { { 96, 32}, {128, 32}, {160, 32}, {192, 32} }); // 10 : move n
		playerSprite->AddAnimation(player_moving_anim_speed, { { 96, 64}, {128, 64}, {160, 64}, {192, 64} });	// 11 : move ne
		playerSprite->AddAnimation(player_moving_anim_speed, { { 96, 96}, {128, 96}, {160, 96}, {192, 96} });	// 12 : move w
		playerSprite->AddAnimation(player_moving_anim_speed, { { 96,128}, {128,128}, {160,128}, {192,128} });	// 13 : move --
		playerSprite->AddAnimation(player_moving_anim_speed, { { 96,160}, {128,160}, {160,160}, {192,160} });	// 14 : move e
		playerSprite->AddAnimation(player_moving_anim_speed, { { 96,192}, {128,192}, {160,192}, {192,192} });	// 15 : move sw
		playerSprite->AddAnimation(player_moving_anim_speed, { { 96,224}, {128,224}, {160,224}, {192,224} });	// 16 : move s
		playerSprite->AddAnimation(player_moving_anim_speed, { { 96,256}, {128,256}, {160,256}, {192,256} });	// 17 : move se
		playerSprite->AddAnimation(player_firing_anim_speed, { {224,  0}, {256,  0}, {288,  0} });	// 18 : fire nw
		playerSprite->AddAnimation(player_firing_anim_speed, { {224, 32}, {256, 32}, {288, 32} });	// 19 : fire n
		playerSprite->AddAnimation(player_firing_anim_speed, { {224, 64}, {256, 64}, {288, 64} });	// 20 : fire ne
		playerSprite->AddAnimation(player_firing_anim_speed, { {224, 96}, {256, 96}, {288, 96} });	// 21 : fire w
		playerSprite->AddAnimation(player_firing_anim_speed, { {224,128}, {256,128}, {288,128} });	// 22 : fire --
		playerSprite->AddAnimation(player_firing_anim_speed, { {224,160}, {256,160}, {288,160} });	// 23 : fire e
		playerSprite->AddAnimation(player_firing_anim_speed, { {224,192}, {256,192}, {288,192} });	// 24 : fire sw
		playerSprite->AddAnimation(player_firing_anim_speed, { {224,224}, {256,224}, {288,224} });	// 25 : fire s
		playerSprite->AddAnimation(player_firing_anim_speed, { {224,256}, {256,256}, {288,256} });	// 27 : fire se
		playerSprite->UseAnimation(7);
	}


// ========================================================================================================================
// START     Terrain
// ========================================================================================================================

	int terrain_tile_width = 16;
	int terrain_tile_height = 16;
	int tile_sheet_tiles_per_row = 20;
	olc::vi2d map_size = { 100, 100 };
	olc::vi2d world_size = { map_size.x * terrain_tile_width, map_size.y * terrain_tile_height };
	olc::vi2d terrain_block_size{ terrain_tile_width, terrain_tile_height };

	void InitialiseTerrain() {
		terrain_blocks_spr = new olc::Sprite("resources/blocks.png");
		terrain_blocks_dec = new olc::Decal(terrain_blocks_spr);
	}

	void DestroyTerrain() {
		delete terrain_blocks_dec;
		delete terrain_blocks_spr;
	}

	void DrawTerain(olc::vu2d position) {

		int camera_x = (int) floor(camera_pos.x);
		int camera_y = (int) floor(camera_pos.y);

		int map_start_x_block = camera_x / terrain_tile_width;
		int map_start_y_block = camera_y / terrain_tile_height;
		int x_offset_to_screen = camera_x % terrain_tile_width;
		int y_offset_to_screen = camera_y % terrain_tile_height;

		int map_screen_width = ScreenWidth() / terrain_tile_width;
		int map_screen_height = ScreenHeight() / terrain_tile_height;
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

				DrawPartialSprite({ i * terrain_tile_width - x_offset_to_screen, j * terrain_tile_height - y_offset_to_screen },
						terrain_blocks_spr, 
						{ block_x, block_y }, 
						terrain_block_size);
				i_pos++;
			}
			j_pos++;
		}
	}

// ========================================================================================================================
// END     Terrain
// ========================================================================================================================



	bool OnUserCreate() override
	{
		controller.Initialize();
		InitialisePlayerSprite();
		InitialiseTerrain();
		return true;
	}

	bool OnUserDestroy() override
	{
		DestroyTerrain();
		delete playerSprite;
		return true;
	}


	bool OnUserUpdate(float fElapsedTime) override
	{
		controller.Update(fElapsedTime);

		HandleUserInput(fElapsedTime);

		UpdateCameraPos();
		Clear(olc::DARK_GREY);

		this->DrawTerain({ 0,0 });

#ifdef SHOW_DEBUG_INFO
		DrawRect(player_pos - camera_pos - sprite_offset, { 32, 32 }, olc::RED);
		DrawString(player_pos-camera_pos - sprite_offset, std::to_string(player_direction));
#endif
		int player_anim_frame = player_reverse ? 8 - player_direction : player_direction;
		playerSprite->Update(player_reverse ? -fElapsedTime : fElapsedTime);

		if (player_state == firing) player_anim_frame += 18;
		if (player_state == moving) player_anim_frame += 9;


		playerSprite->UseAnimation(player_anim_frame);
		playerSprite->Draw(this, player_pos - camera_pos);


//		DrawString({ 10,10 }, "Ply: " + std::to_string(player_pos.x) + ", " + std::to_string(player_pos.y));
//		DrawString({ 10,20 }, "Cam: " + std::to_string(camera_pos.x) + ", " + std::to_string(camera_pos.y));
//		DrawString({ 10,30 }, "Wld: " + std::to_string(world_size.x) + ", " + std::to_string(world_size.y));
		return true;
	}

	void UpdateCameraPos() {
		camera_pos = player_pos + sprite_offset - screen_centre;
		camera_pos.x = bindToRange(camera_pos.x, 0.0f, (float)world_size.x - ScreenWidth());
		camera_pos.y = bindToRange(camera_pos.y, 0.0f, (float)world_size.y - ScreenHeight());
	}

	void HandleUserInput(float elapsedTime) {

		float x_delta = controller.GetLeftStickX();
		float y_delta = controller.GetLeftStickY();
				
		int y_direction = 0;
		int x_direction = 0;

		if (GetKey(key_up).bHeld  || y_delta>0) y_direction = -1;		
		if (GetKey(key_down).bHeld || y_delta<0) y_direction = 1;
		if (GetKey(key_left).bHeld ||x_delta<0) x_direction = -1;
		if (GetKey(key_right).bHeld ||x_delta>0) x_direction = 1;
		if (GetKey(key_reverse).bPressed) player_reverse = !player_reverse;
		if(controller.GetButton(B).bPressed) player_reverse = !player_reverse;

		bool player_firing = ((GetKey(olc::SPACE).bHeld) || controller.GetButton(A).bHeld);
		bool player_moving = (y_direction != 0 || x_direction != 0);

		if (player_firing) player_state = firing;
		else if (player_moving) player_state = moving;
		else player_state = idling;

		// quick bodge to keep char facing in same direction when they stop moving
		int new_player_direction = 1 + x_direction + (y_direction + 1) * 3;
		if (new_player_direction != player_direction_none) player_direction = new_player_direction;

		if(player_state == moving) {
			// move sprite
			float speed = player_speed * elapsedTime;
			if 	(y_direction != 0 && x_direction != 0) speed *= 0.707f;		// account for diagonal travel
			if (player_reverse) speed *= 0.5f;

			olc::vf2d player_pos_delta = { (float)x_direction, (float)y_direction };
			player_pos_delta *= speed;
			player_pos += player_pos_delta;

			player_pos.x = bindToRange(player_pos.x, 0.0f, world_size.x - (float)player_sprite_size.x);
			player_pos.y = bindToRange(player_pos.y, 0.0f, world_size.y - (float)player_sprite_size.y);

			// check for collisions

		}

	}

};

int main()
{
	BugHunt bugHunt;
	if (bugHunt.Construct(screen_size.x, screen_size.y, 2, 2, false,false))
		bugHunt.Start();
	return 0;
}