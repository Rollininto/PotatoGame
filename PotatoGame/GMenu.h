#pragma once
#include <stdlib.h>
#include <vector>
#include <SDL.h>
#include <SDL_ttf.h>
#include "LTexture.h"

class GMenu
{
	SDL_Renderer* gRenderer;
	std::vector<std::string> Items;
	SDL_Rect MenuRect;
	const char* FontPath;
	SDL_Color TextColor;
	LTexture* bgTexture;
	LTexture* PointTexture;

	SDL_Rect PointRect;
	std::vector<LTexture> ItemTextures;
	std::vector<SDL_Rect> ItemRects;
	
	int currOption;
	int selOption;
	
	void LoadTextures();	
public:	
	GMenu(SDL_Renderer*gR, std::vector<std::string> items, SDL_Rect mrect, const char* fPath, SDL_Color miCol = {0,0,0}, LTexture* point = NULL);
	~GMenu();
	void SetBackground(LTexture* bgText);
	void Show();
	int getSelectedOption();
};

