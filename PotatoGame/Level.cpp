#include "Level.h"

SDL_Rect Level::DOOR_CLOSED_CLIPS = { 0,0,4,2 };
SDL_Rect Level::DOOR_OPENED_CLIPS = { 2,2,4,2 };
SDL_Rect Level::COIN_CLIPS = { 0,0,4,2};
SDL_Rect Level::OBST_CLIPS = { 0,0,1,1 };
SDL_Rect Level::GROUND_CLIPS = {0,0,1,1};
int Level::T_ANIMATE = 0;



Level::Level(SDL_Renderer* gR, int w, int h) : lTextures(BlockBox::BL_TYPENUM)
{
	gRenderer = gR;
	
	int size = lTextures.size();
	for (int i = 0; i < size; i++) {
		lTextures[i] = new LTexture();
	}
	
	

	lCoinsTexture = new LTexture();
	lTimeTexture = new LTexture();
	Width = 0;
	Height = 0;
	lCamera = { 0,0, w, h };
	
	BlockBox::DOOR_OPEN = 0;
	Dot::DOT_VEL = (int)(BlockBox::BOX_SIZE*0.1);
}


Level::~Level()
{
	for (LTexture* t : lTextures) {
		t->free();
		t = NULL;	
	}	
}

void Level::handleDotEvent(SDL_Event & e)
{
	potato->handleEvent(e);
}

bool Level::Load(std::string levelName, std::string PotatoStyle)
{
	bool levelLoaded = true;
	//Load background texture
	if (!lTextures[BlockBox::BL_EMPTY]->loadFromFile("Resources/" + levelName + "/bg.png", gRenderer))
	{
		printf("Failed to load background texture!\n");
		levelLoaded = false;
	}

	//Load earth texture
	if (!lTextures[BlockBox::BL_GROUND]->loadFromFile("Resources/" + levelName + "/01.png", gRenderer))
	{
		printf("Failed to load earth block texture!\n");
		levelLoaded = false;
	}

	//Load obstacle texture
	if (!lTextures[BlockBox::BL_OBSTACLE]->loadFromFile("Resources/" + levelName + "/02.png", gRenderer))
	{
		printf("Failed to load obstacle block texture!\n");
		levelLoaded = false;
	}

	//Load coin texture
	if (!lTextures[BlockBox::BL_COLLECTABLE]->loadFromFile("Resources/03.png", gRenderer))
	{
		printf("Failed to load coin block texture!\n");
		levelLoaded = false;
	}
	//Load door tezture
	if (!lTextures[BlockBox::BL_DOOR]->loadFromFile("Resources/" + levelName + "/door.png", gRenderer)) {
		printf("Failed to load door block texture!\n");
		levelLoaded = false;
	}

	//Load dot texture
	LTexture* tmp = new LTexture();
	if (!tmp->loadFromFile("Resources/Dots/"+PotatoStyle+".png", gRenderer))
	{
		printf("Failed to load coin block texture!\n");
		levelLoaded = false;
	}
	

	int x = 0, y = 0;
	
	SDL_Rect init_rect = { 0, 0, BlockBox::BOX_SIZE, BlockBox::BOX_SIZE};

	std::ifstream map("Resources/" + levelName + ".map");
	if (!map.good())
	{
		printf("Unable to load map file!\n");
		levelLoaded = false;
	}
	else
	{
		int door_clips = 0;
		std::string line;
		while (std::getline(map, line)) {
			int blockType = -1;
			x = 0;
			std::istringstream ss(line);
			while (ss >> blockType) {
				if (blockType != 0) {
					init_rect.x = x;
					init_rect.y = y;					
					switch (blockType)
					{
					case BlockBox::BL_DOOR:
						DOOR_CLOSED_CLIPS.x = door_clips % 2;
						DOOR_CLOSED_CLIPS.y = (int)(door_clips / 2);
						lBlocks.push_back(new BlockBox(init_rect, blockType, lTextures[blockType], DOOR_CLOSED_CLIPS));
						door_clips++;
						break;
					case BlockBox::BL_COLLECTABLE:
						lBlocks.push_back(new BlockBox(init_rect, blockType, lTextures[blockType], COIN_CLIPS));
						lCoinsCount++;
						break;
					case BlockBox::BL_OBSTACLE:
						lBlocks.push_back(new BlockBox(init_rect, blockType, lTextures[blockType], OBST_CLIPS));
						break;
					case BlockBox::BL_GROUND:
						lBlocks.push_back(new BlockBox(init_rect, blockType, lTextures[blockType], GROUND_CLIPS));
						break;
					default:
						break;
					}
				}
				x += BlockBox::BOX_SIZE;
			}
			y += BlockBox::BOX_SIZE;
		}
		Width = x;
		Height = y;
	}
	map.close();
	//count background scaling - for bg images smaller then screen
	double kw = (double) lTextures[BlockBox::BL_EMPTY]->getWidth() / Width;
	double kh = (double) lTextures[BlockBox::BL_EMPTY]->getHeight() / Height;
	bg_scale = SDL_min(kw, kh);
	//creating personage
	potato = new Dot(&lBlocks, tmp, gRenderer, Width, Height);

	is_running = true;
	
	Level::T_ANIMATE = 0;
	SDL_TimerID timerID = SDL_AddTimer( 150, CoinAnimate, NULL);
	

	return levelLoaded;
}

void Level::Draw()
{
	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderClear(gRenderer);

	DrawBack();

	//Render score	
	int gotCoins = potato->getCoinCountInt();
	//if(gotCoins == CoinsCount)
	if (gotCoins >= 7)//temporary for tests
		BlockBox::DOOR_OPEN = 1;
	if (lCoinsTexture->loadFromRenderedText("Coins: " + potato->getCoinCountStr() + " / 10", { 0,0,0 }, gRenderer)) {
		SDL_Rect dest = {0,0,0,0};
		lCoinsTexture->render(gRenderer, &dest);
	}
	Uint32 ticks = getTimer();
	std::ostringstream TimeStr;
	TimeStr.str("");
	TimeStr.fill('0');
	TimeStr.width(2);
	TimeStr << (ticks / 1000) / 60 << ":";
	TimeStr.fill('0');
	TimeStr.width(2); 
	TimeStr << (ticks / 1000) % 60 << "." << ticks % 1000;
	if (lTimeTexture->loadFromRenderedText("Time: " + TimeStr.str() , { 0,0,0 }, gRenderer)) {		
		SDL_Rect dest = { 0, lCoinsTexture->getHeight(),0,0 };
		lTimeTexture->render(gRenderer, &dest);
	}

	//Render map
	bool animated = false;
	if (Level::T_ANIMATE == 1) {
		BlockBox::COIN_ANIM_NEXT = 1;
		animated = true;
	}
	int size = lBlocks.size();
	for (int i = 0; i < size; i++) {
		BlockBox* b = lBlocks[i];
		b->render(gRenderer, lCamera);
	}
	if (animated)
		Level::T_ANIMATE = 0;
	BlockBox::COIN_ANIM_NEXT = 0;
	potato->render(lCamera.x, lCamera.y);
	SDL_RenderPresent(gRenderer);
}

void Level::DrawBack()
{
	//Render background
	SDL_Rect new_camera = lCamera;
	new_camera.x *= bg_scale;
	new_camera.y *= bg_scale;
	new_camera.w *= bg_scale;
	new_camera.h *= bg_scale;


	int cam_w = lTextures[BlockBox::BL_EMPTY]->getWidth() / 2;
	if (lCamera.x > cam_w) {
		new_camera.x = lCamera.x % cam_w;
	}
	SDL_Rect dest = { 0, 0, lCamera.w, lCamera.h };
	lTextures[BlockBox::BL_EMPTY]->render(gRenderer, &dest, &new_camera);
}

void Level::UpdateCamera()
{
	if (potato->DOT_VEL>0)
		lCamera.x = potato->getPosX() - 150;
	else
		lCamera.x = potato->getPosX() - lCamera.w + 150;
	int br = potato->getPosY();
	lCamera.y = (potato->getPosY() + Dot::DOT_HEIGHT) - lCamera.h / 2;
	//camera.x = 0;
	//camera.y = 600;
	//Keep the camera in bounds
	if (lCamera.x < 0)
	{
		lCamera.x = 0;
	}
	if (lCamera.y < 0)
	{
		lCamera.y = 0;
	}
	if (lCamera.x > Width - lCamera.w)
	{
		lCamera.x = Width - lCamera.w;
	}
	if (lCamera.y > Height - lCamera.h)
	{
		lCamera.y = Height - lCamera.h;
	}
}

bool Level::isLost()
{
	return  potato->getState() == Dot::States::DOT_DEAD;
}

bool Level::isRunning()
{
	int st = potato->getState();
	if (st == Dot::States::DOT_WON)
		is_running = false;
	return is_running;
}

void Level::exit()
{
	is_running = false;
}

Level::Result Level::GetResult()
{
	Result res;
	res.rGotCoins = potato->getCoinCountInt();
	res.rMaxCoins = lCoinsCount;
	res.rLost = isLost();
	res.rTime = getTimer();
	return res;
}

void Level::StartTimer()
{
	lStartTime = SDL_GetTicks();
}

Uint32 Level::getTimer()
{
	if(lEndTime == 0)
		return SDL_GetTicks() - lStartTime;
	return lEndTime - lStartTime;
}

void Level::StopTimer()
{
	if(lEndTime == 0)
		lEndTime = SDL_GetTicks();
}

Uint32 Level::CoinAnimate(Uint32 interval, void* param)
{
	Level::T_ANIMATE = 1;
	return interval;
}