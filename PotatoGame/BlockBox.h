#pragma once
#include <SDL.h>
#include "LTexture.h"

class BlockBox
{
	SDL_Rect bRect;
	SDL_Rect bClip;
	int bType;
	LTexture* bTexture;
	int collected = 0;
public:
	BlockBox(SDL_Rect rect, int type, LTexture* texture, SDL_Rect cl);
	int getType();
	SDL_Rect getRect();
	void render(SDL_Renderer* gR, SDL_Rect cam);
	static enum {
		BL_EMPTY,
		BL_GROUND,
		BL_OBSTACLE,
		BL_COLLECTABLE,
		BL_DOOR,
		BL_TYPENUM
	};
	static int DOOR_OPEN;
	static int COIN_ANIM_NEXT;
	static int BOX_SIZE;
	static enum Collection
	{
		NO,
		ANIM,
		YES
	};
	void collect();
	bool is_collected();
};
