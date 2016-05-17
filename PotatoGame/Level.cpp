#include "Level.h"

SDL_Rect Level::DOOR_CLOSED_CLIPS = { 0,0,4,2 };
SDL_Rect Level::DOOR_OPENED_CLIPS = { 2,2,4,2 };
SDL_Rect Level::COIN_CLIPS = { 0,0,4,2};
SDL_Rect Level::OBST_UP_CLIPS = { 0,0,2,1 };
SDL_Rect Level::OBST_DOWN_CLIPS = { 1,0,2,1 };
SDL_Rect Level::GROUND_CLIPS = {0,0,1,1};
int Level::T_ANIMATE = 0;
bool Level::PlayAgain = true;
bool Level::ShowResult = true;


Level::Level(SDL_Renderer* gR, int w, int h) : lTextures(BlockBox::BL_TYPENUM),lSounds(Sounds::SE_TOTAL)
{
	gRenderer = gR;
	
	int tsize = lTextures.size();
	for (int i = 0; i < tsize; i++) {
		lTextures[i] = new LTexture();
	}
	
	lCoinsTexture = new LTexture();
	lTimeTexture = new LTexture();
	Width = 0;
	Height = 0;
	lCamera = { 0,0, w, h };
	
	BlockBox::DOOR_OPEN = 0;
	Dot::DOT_VEL = (int)(BlockBox::BOX_SIZE*0.1);
	int ssize = lSounds.size();
	for (int i = 0; i < ssize; i++) {
		lSounds[i] = NULL;
	}
}


Level::~Level()
{
	for (LTexture* t : lTextures) {
		if (t != NULL) {
			t->free();
			t = NULL;
		}
		
	}
	for (Mix_Chunk* mc : lSounds) {
		if (mc != NULL) {
			Mix_FreeChunk(mc);
		}
	}
	for (BlockBox* b : lBlocks) {
		b = NULL;
	}
	if (lLevelMusic != NULL) {
		Mix_FreeMusic(lLevelMusic);
	}
	SDL_RemoveTimer(timerCoinRot);
	
}

void Level::handleDotEvent(SDL_Event & e)
{
	potato->handleEvent(e);
}

bool Level::Load(int lvlNum, Character userPotato)
{
	lvlId = lvlNum;
	std::ostringstream ss("");
	ss << "level_" << lvlNum;
	std::string levelName = ss.str();
	ss.str("");
	std::string PotatoPath = userPotato.path;

	bool levelLoaded = true;
	//Load background texture
	if (!lTextures[BlockBox::BL_EMPTY]->loadFromFile("Resources/" + levelName + "/bg.png", gRenderer))
	{
		OutputDebugString("Failed to load background texture!\n");
		levelLoaded = false;
	}

	//Load earth texture
	if (!lTextures[BlockBox::BL_GROUND]->loadFromFile("Resources/" + levelName + "/01.png", gRenderer))
	{
		OutputDebugString("Failed to load earth block texture!\n");
		levelLoaded = false;
	}

	//Load obstacle texture
	if (!lTextures[BlockBox::BL_OBSTACLE]->loadFromFile("Resources/" + levelName + "/02.png", gRenderer))
	{
		OutputDebugString("Failed to load obstacle block texture!\n");
		levelLoaded = false;
	}

	//Load coin texture
	if (!lTextures[BlockBox::BL_COLLECTABLE]->loadFromFile("Resources/Common/03.png", gRenderer))
	{
		OutputDebugString("Failed to load coin block texture!\n");
		levelLoaded = false;
	}
	//Load door tezture
	if (!lTextures[BlockBox::BL_DOOR]->loadFromFile("Resources/" + levelName + "/door.png", gRenderer)) {
		OutputDebugString("Failed to load door block texture!\n");
		levelLoaded = false;
	}

	//Load dot texture
	LTexture* tmp = new LTexture();
	if (!tmp->loadFromFile(PotatoPath, gRenderer))
	{
		OutputDebugString("Failed to load coin block texture!\n");
		levelLoaded = false;
	}
	lLevelMusic = Mix_LoadMUS("Resources/Sounds/game.wav");
	if (lLevelMusic == NULL) {
		std::stringstream ss;
		ss << "Failed to load /game.wav! SDL_mixer Error: "<< Mix_GetError()<<"\n";
		OutputDebugString(ss.str().c_str());
		levelLoaded= false;
	}
	
	lSounds[Sounds::SE_COIN] = Mix_LoadWAV("Resources/Sounds/coin.wav");
	if (lSounds[Sounds::SE_COIN] == NULL) {
		std::stringstream ss;
		ss << "Failed to load /coin.wav! SDL_mixer Error: "<< Mix_GetError()<<"\n";
		OutputDebugString(ss.str().c_str());
		levelLoaded = false;
	}

	lSounds[Sounds::SE_DEATH] = Mix_LoadWAV("Resources/Sounds/death.wav");
	if (lSounds[Sounds::SE_DEATH] == NULL) {
		std::stringstream ss;
		ss << "Failed to load /death.wav! SDL_mixer Error: "<< Mix_GetError()<<"\n";
		OutputDebugString(ss.str().c_str());
		levelLoaded = false;
	}
	
	lSounds[Sounds::SE_DOOR] = Mix_LoadWAV("Resources/Sounds/door.wav");
	if (lSounds[Sounds::SE_DOOR] == NULL) {
		std::stringstream ss;
		ss << "Failed to load /door.wav! SDL_mixer Error: "<< Mix_GetError()<<"\n";
		OutputDebugString(ss.str().c_str());
		levelLoaded = false;
	}
	
	lSounds[Sounds::SE_JUMP] = Mix_LoadWAV("Resources/Sounds/jump.wav");
	if (lSounds[Sounds::SE_JUMP] == NULL) {
		std::stringstream ss;
		ss << "Failed to load /jump.wav! SDL_mixer Error: "<< Mix_GetError()<<"\n";
		OutputDebugString(ss.str().c_str()); 
		levelLoaded = false;
	}
	int x = 0, y = 0;
	
	SDL_Rect init_rect = { 0, 0, BlockBox::BOX_SIZE, BlockBox::BOX_SIZE};

	std::ifstream map("Resources/" + levelName + ".map");
	if (!map.good())
	{
		OutputDebugString("Unable to load map file!\n");
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
						lBlocks.push_back(new BlockBox(init_rect, BlockBox::BL_DOOR, lTextures[BlockBox::BL_DOOR], DOOR_CLOSED_CLIPS));
						door_clips++;
						break;
					case BlockBox::BL_COLLECTABLE:
						lBlocks.push_back(new BlockBox(init_rect, BlockBox::BL_COLLECTABLE, lTextures[BlockBox::BL_COLLECTABLE], COIN_CLIPS));
						lCoinsCount++;
						break;
					case BlockBox::BL_OBSTACLE:
						lBlocks.push_back(new BlockBox(init_rect, BlockBox::BL_OBSTACLE, lTextures[BlockBox::BL_OBSTACLE], OBST_UP_CLIPS));
						break;
					case BlockBox::BL_OBSTACLE + 10:
						lBlocks.push_back(new BlockBox(init_rect, BlockBox::BL_OBSTACLE, lTextures[BlockBox::BL_OBSTACLE], OBST_DOWN_CLIPS));
						break;
					case BlockBox::BL_GROUND:
						lBlocks.push_back(new BlockBox(init_rect, BlockBox::BL_GROUND, lTextures[BlockBox::BL_GROUND], GROUND_CLIPS));
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
	if (levelLoaded) {
		lLevelData = DataStorage::getLevelData(lvlId);

		//count background scaling - for bg images smaller then screen
		double kw = (double)lTextures[BlockBox::BL_EMPTY]->getWidth() / Width;
		double kh = (double)lTextures[BlockBox::BL_EMPTY]->getHeight() / Height;
		bg_scale = SDL_min(kw, kh);
		//creating personage
		potato = new Dot(&lSounds, &lBlocks, tmp, gRenderer, Width, Height);

		is_running = true;
		Level::T_ANIMATE = 0;
		timerCoinRot = SDL_AddTimer(250, CoinAnimate, NULL);

		//Play the music 
		Mix_PlayMusic(lLevelMusic, -1);
	}
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
	if (gotCoins >= 7 && BlockBox::DOOR_OPEN == 0) {//temporary for tests
		BlockBox::DOOR_OPEN = 1;
		Mix_PlayChannel(-1, lSounds[Sounds::SE_DOOR], 0);
	}

	std::ostringstream CoinsStr("");
	CoinsStr << lLevelData.reqCoins;

	lCoinsTexture->loadFromRenderedText("Coins: " + potato->getCoinCountStr() + " / " + CoinsStr.str(), { 0,0,0 }, gRenderer);
	Uint32 ticks = getTimer();
	std::ostringstream TimeStr;
	TimeStr.str("");
	TimeStr.fill('0');
	TimeStr.width(2);
	TimeStr << (ticks / 1000) / 60 << ":";
	TimeStr.fill('0');
	TimeStr.width(2); 
	TimeStr << (ticks / 1000) % 60 << "." << ticks % 1000;
	lTimeTexture->loadFromRenderedText("Time: " + TimeStr.str(), { 0,0,0 }, gRenderer);
	TimeStr.clear();
	
	//Render map
	bool animated = false;
	if (Level::T_ANIMATE == 1) {
		BlockBox::COIN_ANIM_NEXT = 1;
		animated = true;
	}
	int size = lBlocks.size();
	for (int i = 0; i < size; i++) {		
		if (lBlocks[i]->is_remowed()) {
			lBlocks.erase(lBlocks.begin()+i);
			i--;
			size--;
		}else{ 
			lBlocks[i]->render(gRenderer, lCamera); 
		}
	}
	if (animated)
		Level::T_ANIMATE = 0;
	BlockBox::COIN_ANIM_NEXT = 0;
	potato->render(lCamera.x, lCamera.y);
	lCoinsTexture->render(gRenderer, new SDL_Rect({ 0, 0, 0, 0 }));
	
	lTimeTexture->render(gRenderer, new SDL_Rect({ 0, lCoinsTexture->getHeight(),0,0 }));

 	SDL_RenderPresent(gRenderer);
	lTimeTexture->free();
	lCoinsTexture->free();
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
		//new_camera.x = lCamera.x % cam_w;
	}
	SDL_Rect dest = { 0, 0, lCamera.w, lCamera.h };
	lTextures[BlockBox::BL_EMPTY]->render(gRenderer, &dest, &new_camera);
}

void Level::UpdateCamera()
{
	int shift = 200;
	if (potato->DOT_VEL>0)
		lCamera.x = potato->getPosX() - shift;
	else
		lCamera.x = potato->getPosX() - lCamera.w + shift;
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

void Level::UpdateDot() {
	potato->move();
	if (potato->getState() == Dot::States::DOT_DIE)
		StopTimer();
}

void Level::UpdateDoor()
{
	int gotCoins = potato->getCoinCountInt();
	//if(gotCoins == CoinsCount)
	if (gotCoins >= lLevelData.reqCoins && BlockBox::DOOR_OPEN == 0) {//temporary for tests
		BlockBox::DOOR_OPEN = 1;
		Mix_PlayChannel(-1, lSounds[Sounds::SE_DOOR], 0);
	}
}

bool Level::isLost()
{
	return  potato->getState() == Dot::States::DOT_DEAD || potato->getState() != Dot::States::DOT_WON;
}

bool Level::isRunning()
{
	int st = potato->getState();
	if (st == Dot::States::DOT_WON || st == Dot::States::DOT_DEAD)
		is_running = false;
	return is_running;
}

void Level::exit()
{
	StopTimer();
	LTexture bgFromLevel;
	bgFromLevel.createBlank(gRenderer, lCamera.w, lCamera.h, SDL_TEXTUREACCESS_TARGET);
	bgFromLevel.setAsRenderTarget(gRenderer);
	Draw();
	SDL_SetRenderDrawBlendMode(gRenderer, SDL_BlendMode::SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(gRenderer,0,0,0,150);
	SDL_RenderFillRect(gRenderer, new SDL_Rect({ 0,0,lCamera.w,lCamera.h }));
	LTexture title;
	title.loadFromRenderedText("Quit current game?", { 55,0,0 }, gRenderer, TTF_OpenFont("Resources/Fonts/Mf_July_Sky.ttf", 50));
	title.render(gRenderer, new SDL_Rect({ ( lCamera.w - title.getWidth() ) / 2, lCamera.h/4 - title.getHeight(), 0, 0}));
	SDL_SetRenderTarget(gRenderer, NULL);
	GMenu QuitMenu(gRenderer,
		{0,0,lCamera.w, lCamera.h},
		{ 0, lCamera.h / 4, lCamera.w, lCamera.h / 2 },
		"Resources/Fonts/Mf_July_Sky.ttf",
		40,
		NULL,
		&bgFromLevel, 
		{
			GMenuOption("QUIT",true,{ 125,0,0 }),
			GMenuOption("CANCEL",true,{ 125,0,0 })
		}
		);
	QuitMenu.Show();
	switch (QuitMenu.getSelectedOption())
	{
	case 0:
		Level::ShowResult = false;
		is_running = false;
		Level::PlayAgain = false;
		break;
	case 1:
		ResumeTimer();
		break;
	default:
		break;
	}	
}

void Level::pause()
{
	StopTimer();
	LTexture bgFromLevel;
	bgFromLevel.createBlank(gRenderer, lCamera.w, lCamera.h, SDL_TEXTUREACCESS_TARGET);
	bgFromLevel.setAsRenderTarget(gRenderer);
	Draw();
	SDL_SetRenderDrawBlendMode(gRenderer, SDL_BlendMode::SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 150);
	SDL_RenderFillRect(gRenderer, new SDL_Rect({ 0,0,lCamera.w,lCamera.h }));
	SDL_SetRenderDrawColor(gRenderer, 25, 125, 25, 255);
	SDL_RenderFillRect(gRenderer, new SDL_Rect({ 4 * lCamera.w / 5 - 20, 0, lCamera.w / 5 + 20, lCamera.h } ));
	LTexture title;
	title.loadFromRenderedText("PAUSE", { 0,0,0 }, gRenderer, TTF_OpenFont("Resources/Fonts/Mf_July_Sky.ttf", 50));
	title.render(gRenderer, new SDL_Rect({ (9 * lCamera.w / 5 - title.getWidth())/ 2, 10, 0, 0 }));
	SDL_SetRenderTarget(gRenderer, NULL);
	LTexture* point = potato->getTexture();
	GMenu PauseMenu(gRenderer,
		{ 0,0,lCamera.w, lCamera.h },
		//{ "Resume", "Restart","Quit" },
		//{ 1, 1, 1},
		{ 4*lCamera.w/5, 80, lCamera.w/5, lCamera.h - 80 },
		"Resources/Fonts/Mf_July_Sky.ttf",
		40,
		//{ 0,0,0 },
		point,
		&bgFromLevel,
		{
			GMenuOption("Resume",true,{ 0,0,0 }),
			GMenuOption("Restart",true,{ 0,0,0 }),
			GMenuOption("Quit",true,{ 0,0,0 })
		}
		);
	PauseMenu.Show();
	switch (PauseMenu.getSelectedOption())
	{
	case 0:
		ResumeTimer();
		break;
	case 1:
		is_running = false;
		Level::ShowResult = false;
		break;
	case 2:
		is_running = false;
		Level::PlayAgain = false;
		Level::ShowResult = false;
		break;
	default:
		break;
	}
}

Level::Result Level::GetResult()
{
	Result res;
	res.lvlId = lvlId;
	res.rGotCoins = potato->getCoinCountInt();
	res.rMaxCoins = lLevelData.reqCoins;
	res.rLost = isLost();
	res.rTime = getTimer();
	return res;
}

void Level::StartTimer()
{
	lStartTime = SDL_GetTicks();
}


void Level::ResumeTimer()
{
	lStartTime += SDL_GetTicks() - lEndTime;
	lEndTime = 0;
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