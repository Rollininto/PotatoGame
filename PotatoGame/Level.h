#pragma once
#include "LTexture.h"
#include "BlockBox.h"
#include "Dot.h"
#include <fstream>
#include <sstream>
#include <vector>

class Level
{
	int Width;
	int Height;
	SDL_Renderer* gRenderer;
	std::vector<BlockBox*> lBlocks;
	std::vector<LTexture*> lTextures;
	LTexture* lCoinsTexture;
	LTexture* lTimeTexture;
	SDL_Rect lCamera;
	int lCoinsCount;
	bool is_running;
	Uint32 lStartTime;
	Uint32 lEndTime = 0;

	double bg_scale;

	static SDL_Rect DOOR_CLOSED_CLIPS;
	static SDL_Rect DOOR_OPENED_CLIPS;
	static SDL_Rect COIN_CLIPS;
	static SDL_Rect OBST_CLIPS;
	static SDL_Rect GROUND_CLIPS;

	static int T_ANIMATE;
	static Uint32 CoinAnimate(Uint32 interval, void *param);

public:
	Level(SDL_Renderer* gR, int w, int h);
	~Level();
	bool Load(std::string levelName, std::string PotatoStyle);
	void Draw();
	void DrawBack();
	void UpdateCamera();
	void handleDotEvent(SDL_Event& e);
	

	static struct Result
	{
		bool rLost;
		Uint32 rTime;
		int rGotCoins;
		int rMaxCoins;
	};

	
	Dot* potato;
	
	bool isLost();
	bool isRunning();	
	void exit();
	Result GetResult();
	void StartTimer();
	Uint32 getTimer();
	void StopTimer();
};

