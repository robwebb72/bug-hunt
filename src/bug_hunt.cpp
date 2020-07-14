#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "Zix_PGE_Controller.h"
#include "anim_sprite.cpp"
#include "font_sheet.h"
#include "level_map.h"

#define SHOW_DEBUG_INFO


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
	

	olc::vf2d player_pos { 432.0f, 272.0f};
	olc::vi2d player_bb_terrain_top_left{ 1 ,1 };
	olc::vi2d player_bb_terrain_btm_right{ player_sprite_size.x - 1 -1, player_sprite_size.y - 1 -1 };

	// terrain
	LevelMap levelMap;

	// fonts
	font::Font rob8bitFont;

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
// START   Info Pane
// ========================================================================================================================

	int player_lives = 3;
	int player_mags = 4;
	int player_keys = 12;
	int player_health = 100;
	int player_ammo = 100;

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
			levelMap.Initialise();
			levelMap.LoadResources("resources/levels/level-00");
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
		levelMap.Destroy();
		delete player_sprite;
		return true;
	}

	olc::vf2d WorldToScreen(olc::vf2d pos) {
		return (pos - camera_pos);
	}

	bool lastFocus = true;

	bool OnUserUpdate(float fElapsedTime) override
	{
		bool focus = false;
		if (IsFocused())
		{
			// DIRTY HACK to solve the issue of key presses not being registered as released while the window is not in focus
			// also had to change the access modifiers of the key state arrays to be public
			if (lastFocus == false) {
				for (int i = 0; i < 256; i++) {
					pKeyOldState[i] = { 0 };
					pKeyNewState[i] = { 0 };
					pKeyboardState[i] = { 0 };
				}
			}

			focus = true;
			controller.Update(fElapsedTime);
			HandleUserInput(fElapsedTime);
			UpdateCameraPos();
			// work out which animation the player sprite should be using and then draw the sprite
			int player_anim_frame = player_reverse ? 8 - player_direction : player_direction;
			player_anim_frame += (player_state == PlayerState::firing) ? 18 : 0;
			player_anim_frame += (player_state == PlayerState::moving) ? 9 : 0;

			player_sprite->Update((player_state == PlayerState::moving) & player_reverse ? -fElapsedTime : fElapsedTime); // if player is moving backwards, play walk anim backwards
			player_sprite->UseAnimation(player_anim_frame);
		}
		lastFocus = focus;
		Clear(olc::DARK_GREY);
		levelMap.DrawTerain(this,camera_pos);

		olc::vf2d pos = WorldToScreen(player_pos);
		player_sprite->Draw(this, WorldToScreen(player_pos));

#ifdef SHOW_DEBUG_INFO
		DrawLine({ 0, ScreenHeight() / 2 }, { ScreenWidth(), ScreenHeight() / 2 });
		DrawLine({ ScreenWidth() / 2, 0 }, { ScreenWidth() / 2, ScreenHeight() });
		DrawRect(WorldToScreen(player_pos), { player_sprite_size.x -1, player_sprite_size.y - 1 }, olc::RED);
		DrawString(WorldToScreen(player_pos), std::to_string(player_direction));
		DrawString({ 200,10 }, "Ply: " + std::to_string(player_pos.x) + ", " + std::to_string(player_pos.y));
		DrawString({ 200,20 }, "Cam: " + std::to_string(camera_pos.x) + ", " + std::to_string(camera_pos.y));
#endif
		DrawInfoPane();
		return true;
	}

	void UpdateCameraPos() {
		camera_pos = player_pos + sprite_centre - screen_centre;
		camera_pos.x = BindToRange(camera_pos.x, 0.0f, (float) levelMap.world_size.x - ScreenWidth());
		camera_pos.y = BindToRange(camera_pos.y, 0.0f, (float) levelMap.world_size.y - ScreenHeight());
	}


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

			player_pos_delta = levelMap.CollisionWithTerrain(player_pos, player_pos_delta, player_bb_terrain_top_left, player_bb_terrain_btm_right);

			player_pos += player_pos_delta;

			player_pos.x = BindToRange(player_pos.x, 0.0f, levelMap.world_size.x - (float)player_sprite_size.x);
			player_pos.y = BindToRange(player_pos.y, 0.0f, levelMap.world_size.y - (float)player_sprite_size.y);


		}

	}

};

int main() {
	BugHunt bugHunt;
	if (bugHunt.Construct(screen_size.x, screen_size.y, 2, 2, false,false))
		bugHunt.Start();
	return 0;
}