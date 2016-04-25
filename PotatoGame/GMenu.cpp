#include "GMenu.h"


GMenu::GMenu(SDL_Renderer * gR, std::vector<std::string> items, SDL_Rect mrect, const char* fPath, int fSz, SDL_Color miCol, LTexture* point, LTexture* bg)
{
	gRenderer = gR;
	Items = items;
	MenuRect = mrect;
	FontPath = fPath;
	TextColor = miCol;
	PointTexture = point;
	PointRect = { MenuRect.w / 2, 0, 0, 0 };
	fSize = fSz;
	PointRect = { MenuRect.x + MenuRect.w / 2, 0, 0, 0 };
	bgTexture = bg;
}


GMenu::~GMenu()
{
	int tSize = ItemTextures.size();
	for (int i = 0; i < tSize; i++)
		ItemTextures[i]->free();
	ItemTextures.clear();
}

void GMenu::LoadTextures()
{	
	int margin = 15;
	int x = 0, y = MenuRect.y + margin;
	int textureCnt = Items.size();
	int itemHeight = (MenuRect.h - 2 * margin) / textureCnt;
	if (fSize*textureCnt > MenuRect.h)
		fSize = 0.8*itemHeight;
	else
		itemHeight = 1.25*fSize;	
	int spacing = itemHeight - fSize;
	TTF_Font* tF = TTF_OpenFont(FontPath, fSize);
	PointRect.w = fSize;
	PointRect.h = fSize;

	for (int i = 0; i < textureCnt; i++) {
		int a, b;
		TTF_SizeText(tF, Items[i].c_str(), &a, &b);
		while (a > MenuRect.w - 2 * PointRect.w) {
			fSize--;
			tF = TTF_OpenFont(FontPath, fSize);
			TTF_SizeText(tF, Items[i].c_str(), &a, &b);
		}
	}
	itemHeight = 1.25*fSize;
	spacing = itemHeight - fSize;
	PointRect.w = fSize;
	PointRect.h = fSize;
	if (PointTexture == NULL) {
		PointTexture = new LTexture();
		PointTexture->loadFromRenderedText(">>", { 255,0,0 }, gRenderer,tF);
	}
	ItemTextures.clear();
	ItemRects.clear();

	for (int i = 0; i < textureCnt; i++) {
		ItemTextures.push_back(new LTexture());
		ItemRects.push_back(SDL_Rect());
		ItemTextures[i]->loadFromRenderedText(Items[i], TextColor, gRenderer, tF);
		ItemRects[i].w = ItemTextures[i]->getWidth();
		ItemRects[i].h = ItemTextures[i]->getHeight();
		ItemRects[i].y = y;
		y += ItemRects[i].h + spacing;
		ItemRects[i].x = MenuRect.x + (MenuRect.w - ItemRects[i].w) / 2;
		if (ItemRects[i].x - PointRect.w - 10 < PointRect.x)
			PointRect.x = ItemRects[i].x - PointRect.w - 10;
	}	
}




void GMenu::Show()
{
	bool quit = false;
	SDL_Event e;
	currOption = 0;

	LoadTextures();
	while (!quit)
	{
		SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 0);
		SDL_RenderClear(gRenderer);
		//Handle events on queue
		while (SDL_PollEvent(&e) != 0)
		{
			//User requests quit
			if (e.type == SDL_QUIT )
			{
				quit = true;
				selOption = Items.size() - 1;
			}
			else if (e.type == SDL_KEYDOWN) {
				SDL_Keycode s = e.key.keysym.sym;
				switch (s)
				{
				case SDLK_DOWN:
					if (currOption < Items.size() - 1)
						currOption++;
					break;
				case SDLK_UP:
					if (currOption > 0)
						currOption--;
					break;
				case SDLK_RETURN:
					selOption = currOption;
					quit = true;
					break;
				default:
					break;
				}
			}
		}
		if (bgTexture != NULL) {
			SDL_Rect rc = { 0,0,0,0 };
			bgTexture->render(gRenderer, &rc);
		}
		int iSize = Items.size();
		for (int i = 0; i < iSize; i++) {
			ItemTextures[i]->render(gRenderer, &ItemRects[i]);
		}
		PointRect.y = ItemRects[currOption].y;
		PointTexture->render(gRenderer, &PointRect);
		SDL_RenderPresent(gRenderer);
	}
}

int GMenu::getSelectedOption()
{
	return selOption;
}
