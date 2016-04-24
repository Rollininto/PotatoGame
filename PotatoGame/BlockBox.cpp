#include "BlockBox.h"
int BlockBox::DOOR_OPEN = 0;
int BlockBox::COIN_ANIM_NEXT = 0;
int BlockBox::BOX_SIZE = 30;

BlockBox::BlockBox(SDL_Rect rect, int type, LTexture* texture, SDL_Rect cl)
{
	bClip = cl;
	bType = type;
	bRect = rect;
	bTexture = texture;
}

int BlockBox::getType()
{
	return bType;
}

SDL_Rect BlockBox::getRect()
{
	return bRect;
}

void BlockBox::render(SDL_Renderer* gR, SDL_Rect cam) {
	//SDL_Rect clip = {0, 0, bRect.w, bRect.h };
	
	//SDL_Rect ren = { bRect.x - cam.x, bRect.y - cam.y, bRect.w, bRect.h };
	//SDL_SetRenderDrawColor(gR, 0xFF, 0x00, 0x00, 0x00);
	
	if (collected != Collection::YES) {
		
		//SDL_Rect clip = { 0, 0, bRect.w, bRect.h };
		SDL_Rect dest = { bRect.x - cam.x, bRect.y - cam.y, bRect.w, bRect.h };		
		if (bType == BL_DOOR && DOOR_OPEN == 1) {
			if( bClip.x < bClip.w - 2)
				bClip.x += 2;
		}
		else if (bType == BL_COLLECTABLE) {
			if (collected == Collection::ANIM) {
 				if (bClip.y < bClip.h - 1)
				{
					bClip.y += 1;
					bClip.x = 0;
				}
				if (bClip.x < bClip.w - 1)
					bClip.x += COIN_ANIM_NEXT;
				else
					collected = Collection::YES;
			}
			else
			{
				if (bClip.x < bClip.w)
					bClip.x += COIN_ANIM_NEXT;
				else					
					bClip.x = 0;
			}
		}
		//SDL_SetRenderDrawColor(gR,255,0,0,0);
		//SDL_RenderDrawRect(gR, &dest);
		bTexture->render(gR, &dest,NULL,&bClip);
		
	}
		
	//SDL_RenderDrawRect(gR, &ren);
}

void BlockBox::collect() {
	collected = Collection::ANIM;
}

bool BlockBox::is_collected() {
	return collected != Collection::NO;
}