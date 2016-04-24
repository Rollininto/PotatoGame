#pragma once
#include <SDL.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <sstream>
#include "LTexture.h"
#include "BlockBox.h"

class Dot
{
public:
	//The dimensions of the dot
	static int DOT_WIDTH;
	static int DOT_HEIGHT;
	//Maximum axis velocity of the dot
	static double DOT_VEL;
	static double LEVEL_GRAVITY;
	static int LEVEL_JUMP_POWER;

	bool _grounded = false;

	//Initializes the variables
	Dot(std::vector<BlockBox*>* v, LTexture* t, SDL_Renderer* gR, int mapW, int mapH);
	~Dot();

	//Takes key presses and adjusts the dot's velocity
	void handleEvent(SDL_Event& e);

	//Moves the dot
	void move();
	bool check_collision(SDL_Rect B);
	void stop();
	int fail_anim();
	int getState();

	//Shows the dot on the screen relative to the camera
	void render(int camX, int camY);

	//Position accessors
	int getPosX();
	int getPosY();

	std::string getCoinCountStr();
	int getCoinCountInt();
	static enum States {
		DOT_ALIVE,
		DOT_DEAD,
		DOT_WON
	};
private:
	//The X and Y offsets of the dot
	int mPosX, mPosY;

	//The velocity of the dot
	double mVelX, mVelY;

	int mCoins;
	int State;

	int mapWidth;
	int mapHeight;

	LTexture* dTexture;
	std::vector<BlockBox*>*lBlocks;
	SDL_Renderer* gRenderer;

	SDL_RendererFlip tFlip;
};