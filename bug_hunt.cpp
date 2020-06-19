#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"

olc::vi2d screen_origin = { 0, 0 };
olc::vi2d screen_size = { 640, 400 };

class BugHunt : public olc::PixelGameEngine
{

	olc::vf2d player_pos = { 30.0f, 200.0f };
	olc::vi2d player_size = { 32 ,32 };
	float player_speed = 40.0f;

	int player_direction = 0;

	const int player_anim_frame_idle_start = 0;
	const int player_anim_frame_move_start = 3;
	const int player_anim_frame_fire_start = 7;
	const int player_anim_frame_fire_end = 9;
	float player_anim_frame_counter = 0.0f;
	int player_anim_frame = 0;

	bool player_moving = false;
	float player_moving_anim_speed = 0.2f;

	bool player_firing = true;
	float player_firing_anim_speed = 0.1f;


public:
	BugHunt()
	{
		sAppName = "Bug Hunt";
	}


public:
	bool OnUserCreate() override
	{
		return true;
	}

	bool OnUserDestroy() override
	{
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{

		FillRect(screen_origin, screen_size, olc::BLUE);
		HandleUserInput(fElapsedTime);
		DrawRect(player_pos, player_size, olc::RED);
		DrawString(player_pos, std::to_string(player_direction) + "  " + std::to_string(player_anim_frame));

		return true;
	}

	enum olc::Key key_up = olc::Q;
	enum olc::Key key_down = olc::A;
	enum olc::Key key_left = olc::O;
	enum olc::Key key_right = olc::P;

	void HandleUserInput(float elapsedTime) {

		int y_direction = 0;
		int x_direction = 0;
		if (GetKey(key_up).bHeld) y_direction = -1;
		if (GetKey(key_down).bHeld) y_direction = 1;
		if (GetKey(key_left).bHeld) x_direction = -1;
		if (GetKey(key_right).bHeld) x_direction = 1;
		player_firing = (GetKey(olc::SPACE).bHeld);

		player_moving = (y_direction != 0 || x_direction != 0);
		player_direction = 1 + x_direction + (y_direction + 1) * 3;

		if(player_moving && !player_firing) {
			// player is moving

			// move sprite
			float speed = (y_direction != 0 && x_direction != 0) ? player_speed * 0.707f : player_speed;
			player_pos.x += speed * x_direction * elapsedTime;
			player_pos.y += speed * y_direction * elapsedTime;		

			// work out which frame of animation
			if (player_anim_frame < player_anim_frame_move_start || player_anim_frame >= player_anim_frame_fire_start) {
				player_anim_frame = player_anim_frame_move_start;
				player_anim_frame_counter = 0.0f;
			} else {
				player_anim_frame_counter += elapsedTime;
			}

			if (player_anim_frame_counter > player_moving_anim_speed) {
				player_anim_frame_counter -= player_moving_anim_speed;
				player_anim_frame++;
			}
			if (player_anim_frame >= player_anim_frame_fire_start) player_anim_frame = player_anim_frame_move_start;
		}
		else if (player_firing) {

			if (player_anim_frame < player_anim_frame_fire_start || player_anim_frame > player_anim_frame_fire_end) {
				player_anim_frame = player_anim_frame_fire_start;
				player_anim_frame_counter = 0.0f;
			}
			else {
				player_anim_frame_counter += elapsedTime;
			}
			if (player_anim_frame_counter > player_firing_anim_speed) {
				player_anim_frame_counter -= player_firing_anim_speed;
				player_anim_frame++;
			}
			if (player_anim_frame > player_anim_frame_fire_end) player_anim_frame = player_anim_frame_fire_start;

		}
		else {
		}
	}
};

int main()
{
	BugHunt bugHunt;
	if (bugHunt.Construct(screen_size.x, screen_size.y, 2, 2, false, false))
		bugHunt.Start();
	return 0;
}