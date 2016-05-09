#pragma once
#include <stdlib.h>
#include <vector>
#include <SDL.h>
#include <SDL_ttf.h>
#include "LTexture.h"

class GMenuOption {
	std::string itemText;
	bool available;
	TTF_Font* itemFont;
	SDL_Color itemColor;
	LTexture itemTexture;
public:
	GMenuOption( std::string text, bool avail, SDL_Color col);
	~GMenuOption();
	void setFont(TTF_Font*tf);
	void createTexture(SDL_Renderer* gR);
	std::string Text();
	LTexture* Texture();
	bool Available();
};

class GMenuInfo {
	std::string infoText;
	SDL_Rect infoRect;
	TTF_Font* infoFont;
	SDL_Color infoColor;
	LTexture infoTexture;

public:
	GMenuInfo(std::string text, SDL_Rect irect = {0,0,0,0}, SDL_Color color = { 50,50,50 });
	~GMenuInfo();
	void setFont(TTF_Font*tf);
	void createTexture(SDL_Renderer* gR);
	LTexture* Texture();
};

class GMenu
{
	struct aditionalOpt {
		SDL_Keycode key;
		void (* func)();
	};
	SDL_Renderer* gRenderer;
	SDL_Rect renderWindow;
	std::vector<GMenuOption> Items;
	SDL_Rect MenuRect;
	LTexture* bgTexture;
	LTexture* PointTexture;
	char* FontPath;
	int fSize;
	
	std::vector<GMenuInfo> Infos;
	std::vector<aditionalOpt> AditionalOptions;
	SDL_Rect PointRect;
	std::vector<SDL_Rect> ItemRects;
	std::vector<SDL_Rect> InfoRects;
	SDL_Rect InfoContainerRect;
	int pad;

	int currOption;
	int selOption;
	
	void LoadTextures();	
public:	
	GMenu(SDL_Renderer*gR, SDL_Rect rendWin, SDL_Rect mrect,  char* fPath, int fSz, LTexture* point = NULL, LTexture* bg = NULL, std::vector<GMenuOption> items = {});
	~GMenu();
	void setInfos(std::vector<GMenuInfo> infs);
	void addOption(SDL_Keycode key, void (* func)());
	void Show();
	int getSelectedOption();
};

