#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "Zix_PGE_Controller.h"
#include "map.cpp"
#include "anim_sprite.cpp"
#include "font_sheet.h"

//#define SHOW_DEBUG_INFO
olc::vi2d screen_size = { 640, 400 };


float BindToRange(float value, float min, float max) {
	return value < min ? min : (value > max ? max : value);
}

int BindToRange(int value, int min, int max) {
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
	enum class PlayerState { idling=0, moving=1, firing=2} player_state = PlayerState::idling;
	const int player_direction_none = 4;
	olc::vf2d player_pos = { 384.0f, 160.0f};
	float player_speed = 180.0f;
	int player_direction = 7;
	bool player_reverse = false;

	// player sprite
	AnimatedSprite* player_sprite;
	olc::vi2d player_sprite_size{ 32, 32 };
	olc::vi2d sprite_centre { player_sprite_size.x / 2, player_sprite_size.y / 2 };
	float player_idling_anim_speed = 0.15f;
	float player_moving_anim_speed = 0.10f;
	float player_firing_anim_speed = 0.08f;
	
	// terrain
	olc::Sprite* terrain_blocks_spr;
	olc::Decal* terrain_blocks_dec;

	// fonts
	RWFont rob8bitFont;

	olc::vf2d camera_pos = { 0.0f, 0.0f };

public:
	BugHunt()
	{
		sAppName = "Bug Hunt";
	}


	void InitialisePlayerSprite()
	{
		player_sprite = new AnimatedSprite();
		player_sprite->LoadResource("resources/gent.png");

		player_sprite->SetFrameSize({ 32, 32 });
		player_sprite->AddAnimation(player_idling_anim_speed, { {0,  0}, {32,  0}, {64,  0} });	// 0 : idle nw
		player_sprite->AddAnimation(player_idling_anim_speed, { {0, 32}, {32, 32}, {64, 32} });	// 1 : idle n
		player_sprite->AddAnimation(player_idling_anim_speed, { {0, 64}, {32, 64}, {64, 64} });	// 2 : idle ne
		player_sprite->AddAnimation(player_idling_anim_speed, { {0, 96}, {32, 96}, {64, 96} });	// 3 : idle w
		player_sprite->AddAnimation(player_idling_anim_speed, { {0,128}, {32,128}, {64,128} });	// 4 : idle --
		player_sprite->AddAnimation(player_idling_anim_speed, { {0,160}, {32,160}, {64,160} });	// 5 : idle e
		player_sprite->AddAnimation(player_idling_anim_speed, { {0,192}, {32,192}, {64,192} });	// 6 : idle sw
		player_sprite->AddAnimation(player_idling_anim_speed, { {0,224}, {32,224}, {64,224} });	// 7 : idle s
		player_sprite->AddAnimation(player_idling_anim_speed, { {0,256}, {32,256}, {64,256} });	// 8 : idle se
		player_sprite->AddAnimation(player_moving_anim_speed, { { 96,  0}, {128,  0}, {160,  0}, {192,  0} }); //  9 : move nw
		player_sprite->AddAnimation(player_moving_anim_speed, { { 96, 32}, {128, 32}, {160, 32}, {192, 32} }); // 10 : move n
		player_sprite->AddAnimation(player_moving_anim_speed, { { 96, 64}, {128, 64}, {160, 64}, {192, 64} });	// 11 : move ne
		player_sprite->AddAnimation(player_moving_anim_speed, { { 96, 96}, {128, 96}, {160, 96}, {192, 96} });	// 12 : move w
		player_sprite->AddAnimation(player_moving_anim_speed, { { 96,128}, {128,128}, {160,128}, {192,128} });	// 13 : move --
		player_sprite->AddAnimation(player_moving_anim_speed, { { 96,160}, {128,160}, {160,160}, {192,160} });	// 14 : move e
		player_sprite->AddAnimation(player_moving_anim_speed, { { 96,192}, {128,192}, {160,192}, {192,192} });	// 15 : move sw
		player_sprite->AddAnimation(player_moving_anim_speed, { { 96,224}, {128,224}, {160,224}, {192,224} });	// 16 : move s
		player_sprite->AddAnimation(player_moving_anim_speed, { { 96,256}, {128,256}, {160,256}, {192,256} });	// 17 : move se
		player_sprite->AddAnimation(player_firing_anim_speed, { {224,  0}, {256,  0}, {288,  0} });	// 18 : fire nw
		player_sprite->AddAnimation(player_firing_anim_speed, { {224, 32}, {256, 32}, {288, 32} });	// 19 : fire n
		player_sprite->AddAnimation(player_firing_anim_speed, { {224, 64}, {256, 64}, {288, 64} });	// 20 : fire ne
		player_sprite->AddAnimation(player_firing_anim_speed, { {224, 96}, {256, 96}, {288, 96} });	// 21 : fire w
		player_sprite->AddAnimation(player_firing_anim_speed, { {224,128}, {256,128}, {288,128} });	// 22 : fire --
		player_sprite->AddAnimation(player_firing_anim_speed, { {224,160}, {256,160}, {288,160} });	// 23 : fire e
		player_sprite->AddAnimation(player_firing_anim_speed, { {224,192}, {256,192}, {288,192} });	// 24 : fire sw
		player_sprite->AddAnimation(player_firing_anim_speed, { {224,224}, {256,224}, {288,224} });	// 25 : fire s
		player_sprite->AddAnimation(player_firing_anim_speed, { {224,256}, {256,256}, {288,256} });	// 27 : fire se
		player_sprite->UseAnimation(7);
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

	olc::vi2d WorldToMap(const olc::vf2d& worldcoords) {
		return { WorldXToMap(worldcoords.x), WorldYToMap(worldcoords.y) };
	}

	int WorldXToMap(float world_x) {
		return (int)(world_x / terrain_tile_width);
	}

	int WorldYToMap(float world_y) {
		return (int)(world_y / terrain_tile_height);
	}

	void DrawTerain(olc::vu2d position) {

		int camera_x = (int) floor(camera_pos.x);
		int camera_y = (int) floor(camera_pos.y);

		olc::vi2d map_start = WorldToMap(camera_pos);

		int map_start_x_block = map_start.x;
		int map_start_y_block = map_start.y;

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

				DrawPartialSprite(
					{ i * terrain_tile_width - x_offset_to_screen, j * terrain_tile_height - y_offset_to_screen },
						terrain_blocks_spr, 
						{ block_x, block_y }, 
						terrain_block_size);
				//DrawPartialDecal({ (float) (i * terrain_tile_width - x_offset_to_screen), (float) (j * terrain_tile_height - y_offset_to_screen) },
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

// ========================================================================================================================
// START   Info Pane
// ========================================================================================================================

	olc::Sprite* info_pane_sprite = nullptr;
	olc::Decal* info_pane_decal = nullptr;

	olc::Sprite* health_bar_sprite = nullptr;
	olc::Decal* health_bar_decal = nullptr;

	olc::Sprite* char_icon_sprite = nullptr;
	olc::Decal* char_icon_decal = nullptr;

	void InitialiseInfoPane() {
		// load graphics into sprites and decals
		info_pane_sprite = new olc::Sprite();
		health_bar_sprite = new olc::Sprite();
		char_icon_sprite = new olc::Sprite();

		olc::rcode ret;

		ret = info_pane_sprite->LoadFromFile("resources/info_pane.png");
		if (ret != olc::OK) throw "Error reading image file: resources/info_pane.png";
		ret = health_bar_sprite->LoadFromFile("resources/health_bar.png");
		if (ret != olc::OK) throw "Error reading image file: resources/health_bar.png";
		ret = char_icon_sprite->LoadFromFile("resources/gent_icon.png");
		if (ret != olc::OK) throw "Error reading image file: resources/gent_icon.png";

		info_pane_decal = new olc::Decal(info_pane_sprite);
		health_bar_decal = new olc::Decal(health_bar_sprite);
		char_icon_decal = new olc::Decal(char_icon_sprite);

	}

	int player_lives = 3;
	int player_mags = 4;
	int player_keys = 12;
	int player_health = 100;
	int player_ammo = 100;

	void DrawInfoPane() {

		DrawDecal({ 0.0f, 0.0f }, info_pane_decal);
		DrawDecal({ 0.0f, 0.0f }, char_icon_decal);
		float health_bar = player_health / 100.0f * 30.0f;
		float ammo_bar = player_ammo / 100.0f * 30.0f;

		DrawPartialDecal({ 34.0f, 11.0f }, { health_bar, 4.0f }, health_bar_decal, { 0.0f, 0.0f }, { health_bar, 4.0f });
		DrawPartialDecal({ 70.0f, 11.0f }, { ammo_bar, 4.0f }, health_bar_decal, { 0.0f, 0.0f }, { ammo_bar, 4.0f });

		rob8bitFont.Print(this, { 44, 1 }, std::to_string(player_lives), olc::BLACK);
		rob8bitFont.Print(this, { 43, 0 }, std::to_string(player_lives));

		rob8bitFont.Print(this, { 81, 1 }, std::to_string(player_mags), olc::BLACK);
		rob8bitFont.Print(this, { 80, 0 }, std::to_string(player_mags));

		rob8bitFont.Print(this, { 108, 8 }, std::to_string(player_keys), olc::BLACK);
		rob8bitFont.Print(this, { 107, 7 }, std::to_string(player_keys));
	}

	void DestroyInfoPane() {
		if (info_pane_decal != nullptr) delete info_pane_decal;
		if (health_bar_decal != nullptr) delete health_bar_decal;
		if (char_icon_decal != nullptr) delete char_icon_decal;
		if (info_pane_sprite != nullptr) delete info_pane_sprite;
		if (health_bar_sprite != nullptr) delete health_bar_sprite;
		if (char_icon_sprite != nullptr) delete char_icon_sprite;
	}

// ========================================================================================================================
// END     Info Pane
// ========================================================================================================================


	bool OnUserCreate() override
	{
		controller.Initialize();
		try {
			InitialisePlayerSprite();
			InitialiseTerrain();
			InitialiseInfoPane();
			rob8bitFont.LoadResources("resources/font/rob8bit");
		}
		catch (std::string error) {
			return false;
		}
		return true;
	}

	bool OnUserDestroy() override
	{
		DestroyInfoPane();
		DestroyTerrain();
		delete player_sprite;
		return true;
	}

	olc::vf2d WorldToScreen(olc::vf2d pos) {
		return (pos - camera_pos);
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		Clear(olc::DARK_GREY);
		controller.Update(fElapsedTime);
		HandleUserInput(fElapsedTime);
		UpdateCameraPos();
		DrawTerain({ 0,0 });

		// work out which animation the player sprite should be using and then draw the sprite
		int player_anim_frame = player_reverse ? 8 - player_direction : player_direction;
		player_anim_frame += (player_state == PlayerState::firing) ? 18 : 0;
		player_anim_frame += (player_state == PlayerState::moving) ? 9 : 0;

		player_sprite->Update((player_state == PlayerState::moving) & player_reverse ? -fElapsedTime : fElapsedTime); // if player is moving backwards, play walk anim backwards
		player_sprite->UseAnimation(player_anim_frame);
		player_sprite->Draw(this, WorldToScreen(player_pos));

#ifdef SHOW_DEBUG_INFO
		DrawLine({ 0, ScreenHeight() / 2 }, { ScreenWidth(), ScreenHeight() / 2 });
		DrawLine({ ScreenWidth() / 2, 0 }, { ScreenWidth() / 2, ScreenHeight() });
		DrawRect(WorldToScreen(player_pos), { player_sprite_size.x -1, player_sprite_size.y - 1 }, olc::RED);
		DrawString(WorldToScreen(player_pos), std::to_string(player_direction));
		DrawString({ 10,10 }, "Ply: " + std::to_string(player_pos.x) + ", " + std::to_string(player_pos.y));
		DrawString({ 10,20 }, "Cam: " + std::to_string(camera_pos.x) + ", " + std::to_string(camera_pos.y));
		DrawString({ 10,30 }, "Wld: " + std::to_string(world_size.x) + ", " + std::to_string(world_size.y));
#endif
		DrawInfoPane();
		return true;
	}

	void UpdateCameraPos() {
		camera_pos = player_pos + sprite_centre - screen_centre;
		camera_pos.x = BindToRange(camera_pos.x, 0.0f, (float)world_size.x - ScreenWidth());
		camera_pos.y = BindToRange(camera_pos.y, 0.0f, (float)world_size.y - ScreenHeight());
	}

	olc::vi2d player_bb_terrain_top_left{ 0, 0};
	olc::vi2d player_bb_terrain_btm_right{ player_sprite_size.x - 1, player_sprite_size.y - 1 };

	void HandleUserInput(float elapsedTime) {

		float x_delta = controller.GetLeftStickX();
		float y_delta = controller.GetLeftStickY();
				
		int y_direction = 0;
		int x_direction = 0;

		if (GetKey(olc::NP7).bHeld) {
			y_direction = -1; x_direction = -1;
		}
		if (GetKey(olc::NP9).bHeld) {
			y_direction = -1; x_direction = 1;
		}
		if (GetKey(olc::NP1).bHeld) {
			y_direction = 1; x_direction = -1;
		}
		if (GetKey(olc::NP3).bHeld) {
			y_direction = 1; x_direction = 1;
		}


		if (GetKey(key_up).bHeld  || y_delta>0) y_direction = -1;		
		if (GetKey(key_down).bHeld || y_delta<0) y_direction = 1;
		if (GetKey(key_left).bHeld ||x_delta<0) x_direction = -1;
		if (GetKey(key_right).bHeld ||x_delta>0) x_direction = 1;
		if (GetKey(key_reverse).bPressed) player_reverse = !player_reverse;
		if(controller.GetButton(B).bPressed) player_reverse = !player_reverse;

		bool player_firing = ((GetKey(olc::SPACE).bHeld) || controller.GetButton(A).bHeld);
		bool player_moving = (y_direction != 0 || x_direction != 0);

		if (player_firing) player_state = PlayerState::firing;
		else if (player_moving) player_state = PlayerState::moving;
		else player_state = PlayerState::idling;

		// quick bodge to keep char facing in same direction when they stop moving
		int new_player_direction = 1 + x_direction + (y_direction + 1) * 3;
		if (new_player_direction != player_direction_none) player_direction = new_player_direction;

		if(player_state == PlayerState::moving) {
			// move sprite
			float speed = player_speed * elapsedTime;
			if 	(y_direction != 0 && x_direction != 0) speed *= 0.707f;		// account for diagonal travel
			if (player_reverse) speed *= 0.5f;

			olc::vf2d player_pos_delta = { (float)x_direction, (float)y_direction };
			player_pos_delta *= speed;

			// collision detection here against terrain (before player_pos is updated)

			player_pos_delta = CollisionWithTerrain(player_pos, player_pos_delta, player_bb_terrain_top_left, player_bb_terrain_btm_right);

			player_pos += player_pos_delta;

			player_pos.x = BindToRange(player_pos.x, 0.0f, world_size.x - (float)player_sprite_size.x);
			player_pos.y = BindToRange(player_pos.y, 0.0f, world_size.y - (float)player_sprite_size.y);


		}

	}

};

int main() {
	BugHunt bugHunt;
	if (bugHunt.Construct(screen_size.x, screen_size.y, 2, 2, false,false))
		bugHunt.Start();
	return 0;
}