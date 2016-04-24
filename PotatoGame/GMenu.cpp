#include "GMenu.h"


GMenu::GMenu(SDL_Renderer * gR, std::vector<std::string> items, SDL_Rect mrect, const char* fPath, SDL_Color miCol, LTexture* point)
{
	gRenderer = gR;
	Items = items;
	MenuRect = mrect;
	FontPath = fPath;
	TextColor = miCol;
	PointTexture = point;
	PointRect = { MenuRect.w / 2, 0, 0, 0 };
	
}


GMenu::~GMenu()
{
}

void GMenu::LoadTextures()
{
	if(PointTexture == NULL)
		PointTexture->loadFromRenderedText(".", {255,0,0},gRenderer);
	PointRect.w = PointTexture->getWidth();
	PointRect.h = PointTexture->getHeight\();
	int x = 0, y = 15;
	int textureCnt = Items.size();
	int itemHeight = (MenuRect.h - 2 * y) / textureCnt;
	int fSize = 0.8*itemHeight;
	int spacing = itemHeight - fSize;
	TTF_Font* tF = TTF_OpenFont(FontPath, fSize);
	for (int i = 0; i < textureCnt; i++) {
		int*a, *b;
		TTF_SizeText(tF, Items[i].c_str(), a, b);
		while (*a > MenuRect.w - 2 * PointRect.w) {
			fSize--;
			tF = TTF_OpenFont(FontPath, fSize);
			TTF_SizeText(tF, Items[i].c_str(), a, b);
		}
	}
	itemHeight = 1.25*fSize;
	spacing = itemHeight - fSize;
	ItemTextures.clear();
	ItemRects.clear();
	for (int i = 0; i < textureCnt; i++) {
		ItemTextures.push_back(LTexture());
		ItemTextures[i].loadFromRenderedText(Items[i], TextColor, gRenderer, tF);
		ItemRects[i].w = ItemTextures[i].getWidth();
		ItemRects[i].h = ItemTextures[i].getHeight();
		ItemRects[i].y = y;
		y += ItemRects[i].h + spacing;
		ItemRects[i].x = (MenuRect.w - ItemRects[i].w) / 2;
		if (ItemRects[i].x - PointRect.w < PointRect.x)
			PointRect.x = ItemRects[i].x - PointRect.w;
	}	
}



void GMenu::SetBackground(LTexture* bgTex)
{
	bgTexture = bgTex;
}

void GMenu::Show()
{
	bool quit = false;
	SDL_Event e;
	currOption = 0;

	LoadTextures();
	while (!quit)
	{
		//Handle events on queue
		while (SDL_PollEvent(&e) != 0)
		{
			//User requests quit
			if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE))
			{
				quit = true;
				currOption = Items.size() - 1;
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
					break;
				default:
					break;
				}
			}
		}
		for (int i = 0; i < Items.size(); i++) {
			ItemTextures[i].render(gRenderer, &ItemRects[i]);
		}
		PointRect.y = ItemRects[currOption].y;
		PointTexture->render(gRenderer, &PointRect);
	}
}

int GMenu::getSelectedOption()
{
	return selOption;
}
