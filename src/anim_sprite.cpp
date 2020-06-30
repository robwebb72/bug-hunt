
#ifndef __R72_OLC_ANIMATED_SPRITE__
#define __R72_OLC_ANIMATED_SPRITE__

#include "olcPixelGameEngine.h"
#include <iostream>
#include <vector>


struct SpriteAnimation {

private:
	std::vector<olc::vu2d> Frames;
	int numberOfFrames = 0;
	int currentFrame = 0;
	float framecounter = 0.0f;
	float frameTime = 0.0f;

public: 

	void Reset() {
		framecounter = 0.0f;
		currentFrame = 0;
	}

	SpriteAnimation(float _time, std::vector<olc::vu2d>frames) {
		frameTime = _time;
		AddFrames(frames);
	}

	void SetFrameTime(float time) {
		frameTime = time;
	}

	void AddFrames(std::vector<olc::vu2d> positions) {
		for(auto const & position: positions)
			Frames.push_back(position);
		numberOfFrames += (int) positions.size();
	}

	olc::vu2d CurrentFrame() {
		return Frames.at(currentFrame);
	}

	// we can run the animation forwards or backwards so have to handle wrapping around in both directions
	void Update(float elapsedTime) {
		framecounter += elapsedTime;
		if (framecounter < 0) {
			framecounter += frameTime;
			currentFrame--;
		}
		if (framecounter > frameTime) {
			framecounter -= frameTime;
			currentFrame++;
		}
		if (currentFrame < 0) currentFrame = numberOfFrames - 1;
		if (currentFrame >= numberOfFrames) currentFrame = 0;
	}
};


class AnimatedSprite {

private:
	olc::Sprite* pSprite = nullptr;
	olc::Decal* pDecal = nullptr;
	olc::vu2d frameSize{ 0,0 };
	std::vector<SpriteAnimation *> animations;
	int currentAnimation = 0;

public: 

	void LoadResource(std::string filename) {
		pSprite = new olc::Sprite();
		olc::rcode ret = pSprite->LoadFromFile(filename);
		if (ret != olc::OK) throw "Error reading file: " + filename;
		pDecal = new olc::Decal(pSprite);
	}

	~AnimatedSprite() {
		for (auto animation : animations) delete animation;
		if (pDecal!=nullptr) delete pDecal;
		if (pSprite!=nullptr) delete pSprite;
	}

	void UseAnimation(int i) {
		if (i != currentAnimation) {
			currentAnimation = i;
			animations.at(currentAnimation)->Reset();
		}
	}

	void Update(float time) {
		animations.at(currentAnimation)->Update(time);
	}

	void SetFrameSize(olc::vu2d size) {
		frameSize.x = size.x;
		frameSize.y = size.y;
	}

	void Draw(olc::PixelGameEngine* engine, olc::vf2d position) {
		engine->DrawPartialDecal(position, frameSize, pDecal,
			animations.at(currentAnimation)->CurrentFrame(), frameSize);
	}

	void AddAnimation(float frameTime, std::vector<olc::vu2d> frames) {
		animations.push_back(new SpriteAnimation(frameTime, frames));
	}

};


#endif
