#pragma once
#include"Sounds.h"
#include "LTexture.h"
#include "BlockBox.h"
#include "Dot.h"
#include "GMenu.h"
#include "DataStorage.h"
#include <fstream>
#include <sstream>
#include <vector>

class Level
{	
	int lvlId;
	int Width;
	int Height;
	SDL_Renderer* gRenderer;
	std::vector<BlockBox*> lBlocks;
	std::vector<LTexture*> lTextures;
	LTexture* lCoinsTexture;
	LTexture* lTimeTexture;
	Mix_Music *lLevelMusic;
	std::vector<Mix_Chunk*> lSounds;
	SDL_Rect lCamera;
	int lCoinsCount;
	bool is_running;
	Uint32 lStartTime;
	Uint32 lEndTime = 0;

	double bg_scale;

	static SDL_Rect DOOR_CLOSED_CLIPS;
	static SDL_Rect DOOR_OPENED_CLIPS;
	static SDL_Rect COIN_CLIPS;
	static SDL_Rect OBST_UP_CLIPS;
	static SDL_Rect OBST_DOWN_CLIPS;
	static SDL_Rect GROUND_CLIPS;

	static int T_ANIMATE;
	static Uint32 CoinAnimate(Uint32 interval, void *param);
	SDL_TimerID timerCoinRot;

public:
	Level(SDL_Renderer* gR, int w, int h);
	~Level();
	bool Load(sLevelData lvlData, Character userPotato);
	void Draw();
	void DrawBack();
	void UpdateCamera();
	void UpdateDot();
	void UpdateDoor();
	void handleDotEvent(SDL_Event& e);
	
	sLevelData lLevelData;
	static struct Result
	{
		int lvlId;
		bool rLost;
		Uint32 rTime;
		int rGotCoins;
		int rMaxCoins;
	};
	
	


	Dot* potato;
	
	bool isLost();
	bool isRunning();	
	void ExitPrompt();
	void PauseMenu();
	Result GetResult();
	void StartTimer();
	void ResumeTimer();
	Uint32 getTimer();
	void StopTimer();

	static bool PlayAgain;
	static bool ShowResult;
};

