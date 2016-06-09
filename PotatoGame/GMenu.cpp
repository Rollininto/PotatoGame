#include "GMenu.h"


/*GMenu::GMenu(SDL_Renderer * gR, SDL_Rect rendWin, std::vector<std::string> items, std::vector<int> av, SDL_Rect mrect, const char* fPath, int fSz, SDL_Color miCol, LTexture* point, LTexture* bg, std::vector<std::string> infoMsg)
{
	gRenderer = gR;
	renderWindow = rendWin;
	Items = items;
	available = av;
	MenuRect = mrect;
	FontPath = fPath;
	TextColor = miCol;
	PointTexture = point;
	fSize = fSz;
	PointRect = { MenuRect.x + MenuRect.w / 2, 0, 0, 0 };
	bgTexture = bg;
	infoMessage = infoMsg;
	pad = 5;
}*/


GMenu::GMenu(SDL_Renderer * gR, SDL_Rect rendWin, SDL_Rect mrect,  char* fPath, int fSz, LTexture * point, LTexture * bg, std::vector<GMenuOption> items)
{
	gRenderer = gR;
	renderWindow = rendWin;
	MenuRect = mrect;
	Items = items;
	PointTexture = point;
	PointRect = { MenuRect.x + MenuRect.w / 2, 0, 0, 0 };
	bgTexture = bg;
	FontPath = fPath;
	fSize = fSz;
	pad = 5;
	selOption = -1;
}

GMenu::~GMenu()
{
	PointTexture = NULL;
	bgTexture = NULL;

}

void GMenu::setInfos(std::vector<GMenuInfo> infs)
{
	Infos = infs;
}

void GMenu::addHotKeyOption(SDL_Keycode key, void (* func)())
{
	aditionalOpt a_opt;
	a_opt.key = key;
	a_opt.func = func;
	AditionalOptions.push_back(a_opt);
}

void GMenu::LoadTextures()
{	
	SDL_Color TextDisColor = {127,127,127};
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
		TTF_SizeText(tF, Items[i].Text().c_str(), &a, &b);
		while (a > MenuRect.w - 2 * PointRect.w) {
			fSize--;
			TTF_CloseFont(tF);
			tF = TTF_OpenFont(FontPath, fSize);
			TTF_SizeText(tF, Items[i].Text().c_str(), &a, &b);
		}
	}
	for (int i = 0; i < textureCnt; i++) {
		Items[i].setFont(tF);
	}

	itemHeight = 1.25*fSize;
	spacing = itemHeight - fSize;
	PointRect.w = fSize;
	PointRect.h = fSize;
	if (PointTexture == NULL) {
		PointTexture = new LTexture();
		PointTexture->loadFromRenderedText(">>", { 255,0,0 }, gRenderer,tF);
	}
	ItemRects.clear();

	for (int i = 0; i < textureCnt; i++) {
		Items[i].createTexture(gRenderer);
		ItemRects.push_back(SDL_Rect());
		ItemRects[i].w = Items[i].Texture()->getWidth();
		ItemRects[i].h = Items[i].Texture()->getHeight();
		ItemRects[i].y = y;
		y += ItemRects[i].h + spacing;
		ItemRects[i].x = MenuRect.x + (MenuRect.w - ItemRects[i].w) / 2;
		if (ItemRects[i].x - PointRect.w - 10 < PointRect.x)
			PointRect.x = ItemRects[i].x - PointRect.w - 10;
	}
	TTF_CloseFont(tF);
	tF = TTF_OpenFont(GAME_INFO_FONT_PATH, 2*fSize/3);

	InfoContainerRect = { renderWindow.w , renderWindow.h,0,0 }; 
	int step = 0;
	for (int i = 0; i < Infos.size(); i++) {
		Infos[i].setFont(tF);
		Infos[i].createTexture(gRenderer);
		InfoRects.push_back(SDL_Rect());
		InfoRects[i] = { renderWindow.w - Infos[i].Texture()->getWidth() - 2*pad,
			2 * pad + step,
			Infos[i].Texture()->getWidth(),
			Infos[i].Texture()->getHeight()
		};
		step = Infos[i].Texture()->getHeight();
		if (InfoRects[i].x < InfoContainerRect.x) {
			InfoContainerRect.x = InfoRects[i].x - pad;
		}
		if (InfoRects[i].y < InfoContainerRect.y) {
			InfoContainerRect.y = InfoRects[i].y - pad;
		}
		if (InfoRects[i].w > InfoContainerRect.w) {
			InfoContainerRect.w = InfoRects[i].w;
		}
		InfoContainerRect.h += InfoRects[i].h;
	}
	TTF_CloseFont(tF);
	InfoContainerRect.w += 2*pad;
	InfoContainerRect.h += 2*pad;
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
			if (e.type == SDL_QUIT )
			{
				quit = true;
				selOption = Items.size() - 1;
			}
			else if (e.type == SDL_KEYDOWN) {
				SDL_Keycode s = e.key.keysym.sym;
				switch (s)
				{
				case SDLK_ESCAPE:
					selOption = -1;
					quit = true;
					break;
				case SDLK_DOWN:
					if (currOption < Items.size() - 1)
						currOption++;
					break;
				case SDLK_UP:
					if (currOption > 0)
						currOption--;
					break;
				case SDLK_RETURN:
					if (Items[currOption].Available()) {
						selOption = currOption;
						quit = true;
					}
					break;
				default:
					for (aditionalOpt ao : AditionalOptions) {
						if (ao.key == s) {
							ao.func();
							quit = true;
						}
					}
					break;
				}
			}
		}
		SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 0);
		SDL_RenderClear(gRenderer);

		if (bgTexture != NULL) {
			SDL_Rect rc = { 0,0,0,0 };
			bgTexture->render(gRenderer, &rc);
		}
		int iSize = Items.size();
		for (int i = 0; i < iSize; i++) {
			Items[i].Texture()->render(gRenderer, &ItemRects[i]);
		}
		PointRect.y = ItemRects[currOption].y;
		PointTexture->render(gRenderer, &PointRect);
		if (!Infos.empty()) {
			for (int i = 0; i < Infos.size();i++)
				Infos[i].Texture()->render(gRenderer, &InfoRects[i]);
		}
		SDL_RenderPresent(gRenderer);
	}
}

int GMenu::getSelectedOption()
{
	return selOption;
}

GMenuOption::GMenuOption(std::string text, bool avail, SDL_Color col)
{
	itemText = text;
	available = avail;
	itemColor = col;
}

GMenuOption::~GMenuOption()
{
	itemTexture.free();
	itemFont = NULL;
}

void GMenuOption::setFont(TTF_Font * tf)
{
	itemFont = tf;
}

void GMenuOption::createTexture(SDL_Renderer* gRenderer)
{	
	if (available) {
		itemTexture.loadFromRenderedText(itemText, itemColor, gRenderer, itemFont);
	}
	else
	{
		itemTexture.loadFromRenderedText(itemText, {127,127,127}, gRenderer, itemFont);
	}
}

std::string GMenuOption::Text()
{
	return itemText;
}

LTexture * GMenuOption::Texture()
{
	return &itemTexture;
}

bool GMenuOption::Available()
{
	return available;
}

GMenuInfo::GMenuInfo( std::string text, SDL_Rect irect, SDL_Color color)
{
	infoText = text;
	infoRect = irect;
	infoColor = color;
}

void GMenuInfo::createTexture(SDL_Renderer* gRenderer) {
	infoTexture.loadFromRenderedText(infoText, infoColor, gRenderer, infoFont);
}

GMenuInfo::~GMenuInfo()
{
	infoTexture.free();
	infoFont = NULL;
}

void GMenuInfo::setFont(TTF_Font * tf)
{
	infoFont = tf;
}

LTexture * GMenuInfo::Texture()
{
	return &infoTexture;
}
