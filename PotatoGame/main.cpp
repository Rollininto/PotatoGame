#include "GameConstants.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <time.h>
#include <Windows.h>
#include <WinBase.h>
#include "LTexture.h"
#include "DataStorage.h"
#include "Level.h"
#include "GMenu.h"

//Screen dimension constants
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

enum MainMenu {
	O_CAMPAIGN,
	O_ARCADE,
	O_CUSTOMIZE,
	O_SETTINGS,
	O_EXIT
};

enum ResMenu {
	O_RETRY,
	O_NEXTLVL,
	O_QUIT
};
//Global variables


//The window we'll be rendering to
SDL_Window* gWindow = NULL;

	//The window renderer
SDL_Renderer* gRenderer = NULL;

LTexture gMenuBG;
LTexture gMenuPointer;
LTexture gMainMenuBg;
LTexture gResMenuBgOk;
LTexture gResMenuBgFail;


Mix_Music *gMainMusic = NULL;//The music that will be played in main menu
Mix_Chunk* gSFXSample = NULL;//Sample sound for SFX volume preview
//volume
double MusicVolume = 0.6;
double SFXVolume = 0.6;

sUserData gUserData;

int gAvailArcLevelsCnt;
int gOverCampLevelsCnt;
bool gFullScreen;

bool init();
bool loadMedia();
void GetAvailableGameData();
void RenderResults(Level* lvl);
void ShowCampaignMenu();
void ShowLogInForm();
void ChangeUser(std::string log);
void PlayLevel(int lvlId = -1);
void UpdateData(int lId, Level* currLevel);
void ShowSettings();
void ShowCustomize();
void close();


int main(int argc, char* args[])
{
	gUserData.login = "Guest";
	gUserData.coins = 0;
	gUserData.levelsPassed = 0;
	gUserData.persId = 1;
	//Start up SDL and create window
	if (!init())
	{
		OutputDebugString("Failed to initialize!\n");
	}
	else
	{
		if (!loadMedia())
		{
			OutputDebugString("Failed to initialize!\n");
		}
		else 
		{		
			//if we are dubugging with breakpoints it would be impossible to switch to VS from fullscreen
			//SDL_SetWindowFullscreen(gWindow, SDL_WINDOW_FULLSCREEN);
			SDL_SetWindowBordered(gWindow, SDL_FALSE);
			gFullScreen = false;
			GetAvailableGameData();

			//Main loop flag
			bool quit = false;
			int menuOptionSel = -1;

					
			//While application is running
			while (!quit)
			{	
				if (Mix_PlayingMusic() == 0) { 
					//Play the music 
					Mix_PlayMusic( gMainMusic, -1 ); 
				}
							
				GMenu MainMenu(gRenderer,
					{0,0,SCREEN_WIDTH,SCREEN_HEIGHT},
					//{ "CAMPAIGN","ARCADE","OPTIONS","EXIT" },
					//{ 1, 1, 1, 1 },
					{0, SCREEN_HEIGHT/2, SCREEN_WIDTH, SCREEN_HEIGHT/2},
					"Resources/Fonts/Mf_July_Sky.ttf",
					40,
					//{0,0,0},
					&gMenuPointer, 
					&gMainMenuBg,
					{
						GMenuOption("CAMPAIGN",true,{ 0,0,0 }),
						GMenuOption("ARCADE",true,{ 0,0,0 }),
						GMenuOption("CUSTOMIZE",true,{ 0,0,0 }),
						GMenuOption("OPTIONS",true,{ 0,0,0 }),
						GMenuOption("EXIT",true,{ 0,0,0 })
					}
				);
				MainMenu.setInfos(
				{
					GMenuInfo("Logged in as: "+gUserData.login),
					GMenuInfo("To log in press F3!")
				}
				);
				MainMenu.addHotKeyOption(SDLK_F3, &ShowLogInForm);
				MainMenu.Show();
				menuOptionSel = MainMenu.getSelectedOption();
				if ( menuOptionSel == MainMenu::O_ARCADE)
				{		
					PlayLevel();
				}
				else if (menuOptionSel == MainMenu::O_CAMPAIGN) {
					std::vector<GMenuOption> campOptions;
					for (int i = 1; i <= gOverCampLevelsCnt;i++) {
						std::ostringstream lname("");
						lname << "Level " << i;
						if(i<=gUserData.levelsPassed+1)
							campOptions.push_back(GMenuOption(lname.str(), true, { 0,0,0 }) );
						else
							campOptions.push_back(GMenuOption(lname.str(), false, { 0,0,0 }));
					}
					campOptions.push_back(GMenuOption("EXIT", true, { 0,0,0 }));
					GMenu CampaignMenu(gRenderer,
						{0,0,SCREEN_WIDTH,SCREEN_HEIGHT},
						{ 0, SCREEN_HEIGHT/5, SCREEN_WIDTH , 4*SCREEN_HEIGHT/5 },
						"Resources/Fonts/Mf_July_Sky.ttf",
						40,
						&gMenuPointer,
						&gMainMenuBg,
						campOptions
					);
					CampaignMenu.Show();
					int selOpt = CampaignMenu.getSelectedOption();
					if (selOpt >= 0 && selOpt < gOverCampLevelsCnt) {
						PlayLevel(selOpt + 1);
					}
					else if(selOpt!=gOverCampLevelsCnt) {
						OutputDebugString("Campaign menu: bad option!");
					}
				}
				else if (menuOptionSel == MainMenu::O_CUSTOMIZE) {
					ShowCustomize();
				}
				else if (menuOptionSel == MainMenu::O_SETTINGS) {
					ShowSettings();
				}
				else if (menuOptionSel == MainMenu::O_EXIT) {
					quit = true;
				}				
			}
		}
	
	}
	close();
	return 0;
}

bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0 )
	{
		OutputDebugString("SDL could not initialize!");
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		{
			OutputDebugString("Warning: Linear texture filtering not enabled!\n");
		}

		//Create window
		gWindow = SDL_CreateWindow("Ziemniak", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (gWindow == NULL)
		{
			OutputDebugString("Window could not be created!\n");
			success = false;
		}
		else
		{
			//Create vsynced renderer for window
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (gRenderer == NULL)
			{
				OutputDebugString("Renderer could not be created!\n");
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if (!(IMG_Init(imgFlags) & imgFlags))
				{
					OutputDebugString("SDL_image could not initialize!\n");
					success = false;
				}
				//Initialize SDL_ttf 
				if( TTF_Init() == -1 ) 
				{
					OutputDebugString( "SDL_ttf could not initialize!\n");
					success = false; 
				}
				//Initialize SDL_mixer 
				if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 ) { 
					std::stringstream ss;
					ss << "SDL_mixer could not initialize! SDL_mixer Error: "<< Mix_GetError()<<"\n";
					OutputDebugString(ss.str().c_str());
					success = false; 
				}
			}
		}
	}

	return success;
}

bool loadMedia()
{
	//Loading success flag
	bool success = true;
	if (!gMenuPointer.loadFromFile("Resources/Common/pointer.png", gRenderer))
	{
		OutputDebugString("Failed to load pointer texture!\n");
		success = false;
	}
	if (!gMainMenuBg.loadFromFile("Resources/Common/main_menu.png", gRenderer))
	{
		OutputDebugString("Failed to load main menu background texture!\n");
		success = false;
	}
	if (!gResMenuBgOk.loadFromFile("Resources/Common/win.png", gRenderer))
	{
		OutputDebugString("Failed to load succes result menu background texture!\n");
		success = false;
	}
	if (!gResMenuBgFail.loadFromFile("Resources/Common/loose.png", gRenderer))
	{
		OutputDebugString("Failed to load fail result menu background texture!\n");
		success = false;
	}
	//load game main screen textures and lables(fonts,strings etc.)

	//Load music 
	gMainMusic = Mix_LoadMUS( "Resources/Sounds/menu.wav" );
	if( gMainMusic == NULL ) { 
		std::stringstream ss;
		ss << "Failed to load /menu.wav! SDL_mixer Error: " << Mix_GetError() << "\n";
		OutputDebugString(ss.str().c_str());
		success = false; 
	}
	else {
		Mix_VolumeMusic((int)(MusicVolume*MIX_MAX_VOLUME));
		Mix_Volume(-1, (int)(SFXVolume*MIX_MAX_VOLUME));
	}

	gSFXSample = Mix_LoadWAV("Resources/Sounds/jump.wav");
	if (gSFXSample == NULL) {
		std::stringstream ss;
		ss << "Failed to load /jump.wav! SDL_mixer Error: " << Mix_GetError() << "\n";
		OutputDebugString(ss.str().c_str());
		success = false;
	}
	return success;
}

void GetAvailableGameData() {
	//std::vector<sLevelData> lvls = DataStorage::
	gAvailArcLevelsCnt = 3;
	gOverCampLevelsCnt = 5;
}

void RenderResults(Level* lvl) {
	Level::Result r = lvl->GetResult();
	LTexture* rsTimeTexture = new LTexture();
	LTexture* rsCoinsTexture = new LTexture();
	LTexture bestLvlTimeTexture, bestLvlTimePrompt;
	LTexture bestUserTime,bestUserCoins, bestResPrompt;
	LTexture rsResultTexture;
	LTexture ret;
	gMenuBG.createBlank(gRenderer, SCREEN_WIDTH, SCREEN_HEIGHT,SDL_TEXTUREACCESS_TARGET);
	TTF_Font* tF;

	//result textures
	if (r.rLost) {
		tF = TTF_OpenFont(GAME_LOST_FONT_PATH, 70);
		rsResultTexture.loadFromRenderedText("YOU LOST...", {255,0,0},gRenderer,tF);
		rsCoinsTexture = NULL;
		rsTimeTexture = NULL;
	}
	else {
		tF = TTF_OpenFont(GAME_WON_FONT_PATH, 70);
		rsResultTexture.loadFromRenderedText("YOU WON!!!", { 0,255,0 }, gRenderer, tF);
		TTF_CloseFont(tF);
		tF = TTF_OpenFont(GAME_RESULT_FONT_PATH, 40);
		std::stringstream cc;
		cc.str("");
		cc << "Coins: "<< r.rGotCoins<<" / "<<r.rMaxCoins;
		rsCoinsTexture->loadFromRenderedText(cc.str(), { 0,55,0 }, gRenderer, tF);
		cc.str("");
		cc << "Time: ";
		cc.fill('0');
		cc.width(2);
		cc<< (r.rTime / 1000) / 60 << ":";
		cc.fill('0');
		cc.width(2);
		cc << (r.rTime / 1000) % 60 << "." << r.rTime % 1000;
		rsTimeTexture->loadFromRenderedText(cc.str(), { 0,55,0 }, gRenderer, tF);
	}
	TTF_CloseFont(tF);
	sLevelData ld = lvl->lLevelData;
	sUsersLevelResult ulres = DataStorage::getUsersLevelResult(gUserData, ld);

	tF = TTF_OpenFont(GAME_BEST_RESULT_FONT_PATH, 20);
	bestResPrompt.loadFromRenderedText("Best user result:", {0,0,0}, gRenderer,tF);
	std::stringstream format;
	format.str("");
	format << "Bonus coins: " << ulres.userBestBonusCoins;//??
	bestUserCoins.loadFromRenderedText(format.str(), { 0,0,0 }, gRenderer);
	format.str("");
	format << "Time: ";
	format.fill('0');
	format.width(2);
	format << (ulres.userBestTime / 1000) / 60 << ":";
	format.fill('0');
	format.width(2);
	format << (ulres.userBestTime/ 1000) % 60 << "." << ulres.userBestTime% 1000;
	bestUserTime.loadFromRenderedText(format.str(), { 0,0,0 }, gRenderer, tF);

	bestLvlTimePrompt.loadFromRenderedText("Best level time:", {0,0,0},gRenderer,tF);
	format.str("");
	format << "  ";
	format.fill('0');
	format.width(2);
	format << ( ld.bestTime / 1000) / 60 << ":";
	format.fill('0');
	format.width(2);
	format << (ld.bestTime / 1000) % 60 << "." << ld.bestTime % 1000 ;
	if (ld.bestTime != 0) {
		format << " by " << ld.bestUserLogin;
	}
	bestLvlTimeTexture.loadFromRenderedText(format.str(), { 0,0,0 }, gRenderer, tF);

	SDL_Rect rsResultDest = {
		(SCREEN_WIDTH - rsResultTexture.getWidth()) / 2 ,
		50,
		rsResultTexture.getWidth(),
		rsResultTexture.getHeight()
	};
	SDL_Rect rsCoinsDest = {
		rsResultDest.x,
		rsResultDest.y + rsResultDest.h + 15,
		0,
		0
	};
	if (rsCoinsTexture != NULL) {
		rsCoinsDest.w = rsCoinsTexture->getWidth();
		rsCoinsDest.h = rsCoinsTexture->getHeight();
	}
	SDL_Rect rsTimeDest = {
		rsResultDest.x,
		rsCoinsDest.y + rsCoinsDest.h + 15,
		0,
		0
	};
	if (rsTimeTexture != NULL) {
		rsTimeDest.w = rsTimeTexture->getWidth();
		rsTimeDest.h = rsTimeTexture->getHeight();
	}

	SDL_Rect bestLvlTimePromptDest = {
		50,
		SCREEN_HEIGHT/3,
		bestLvlTimePrompt.getWidth(),
		bestLvlTimePrompt.getHeight()
	};	
	SDL_Rect bestLvlTimeTextureDest = {
		bestLvlTimePromptDest.x + 25,
		bestLvlTimePromptDest.y + bestLvlTimePromptDest.h + 5,
		bestLvlTimeTexture.getWidth(),
		bestLvlTimeTexture.getHeight()
	};

	SDL_Rect bestResPromptDest = {
		bestLvlTimePromptDest.x,
		bestLvlTimeTextureDest.y + bestLvlTimeTextureDest.h + 25,
		bestResPrompt.getWidth(),
		bestResPrompt.getHeight()
	};
	SDL_Rect bestUserTimeDest = {
		bestResPromptDest.x + 25,
		bestResPromptDest.y + bestResPromptDest.h + 5,
		bestUserTime.getWidth(),
		bestUserTime.getHeight()
	};
	SDL_Rect bestUserCoinsDest = {
		bestUserTimeDest.x,
		bestUserTimeDest.y + bestUserTimeDest.h + 5,
		bestUserCoins.getWidth(),
		bestUserCoins.getHeight()
	};
	

	gMenuBG.setAsRenderTarget(gRenderer);
	//Clear screen
	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderClear(gRenderer);
	if (r.rLost) {
		gResMenuBgFail.render(gRenderer, new SDL_Rect({ 0,0,SCREEN_WIDTH,SCREEN_HEIGHT }));
	}
	else {
		gResMenuBgOk.render(gRenderer, new SDL_Rect({ 0,0,SCREEN_WIDTH,SCREEN_HEIGHT }));
	}
	if (rsCoinsTexture != NULL)
		rsCoinsTexture->render(gRenderer, &rsCoinsDest);
	if (rsTimeTexture != NULL)
		rsTimeTexture->render(gRenderer, &rsTimeDest);

	rsResultTexture.render(gRenderer, &rsResultDest);
	bestLvlTimePrompt.render(gRenderer, &bestLvlTimePromptDest);
	bestLvlTimeTexture.render(gRenderer, &bestLvlTimeTextureDest);
	
	if (gUserData.login != "Guest") {
		bestResPrompt.render(gRenderer, &bestResPromptDest);
		bestUserTime.render(gRenderer, &bestUserTimeDest);
		bestUserCoins.render(gRenderer, &bestUserCoinsDest);
	}
	SDL_SetRenderTarget(gRenderer, NULL);
}

void ShowCampaignMenu()
{
	//campaign menu
	//probably should do map-like
}

void ShowLogInForm()
{
	//memory leak???
	SDL_Surface* saveSurface = NULL;
	SDL_Surface* surf = SDL_GetWindowSurface(gWindow);
	unsigned char * pixels = new (std::nothrow) unsigned char[surf->w * surf->h * surf->format->BytesPerPixel];
	SDL_RenderReadPixels(gRenderer, &surf->clip_rect, surf->format->format, pixels, surf->w * surf->format->BytesPerPixel);
	saveSurface = SDL_CreateRGBSurfaceFrom(pixels, surf->w, surf->h, surf->format->BitsPerPixel, surf->w * surf->format->BytesPerPixel, surf->format->Rmask, surf->format->Gmask, surf->format->Bmask, surf->format->Amask);
	SDL_Texture* loginBG = SDL_CreateTextureFromSurface(gRenderer, saveSurface);
	SDL_Delay(100);
	SDL_FreeSurface(surf);
	SDL_FreeSurface(saveSurface);
	surf = NULL;
	saveSurface = NULL;
	delete[] pixels;
	pixels = NULL;
	SDL_Delay(100);
	SDL_Rect bgDest = {
		0,
		0,
		SCREEN_WIDTH,
		SCREEN_HEIGHT
	};
	SDL_Rect Form = {
		(SCREEN_WIDTH -  SCREEN_WIDTH / 4) / 2,
		SCREEN_HEIGHT / 4,
		SCREEN_WIDTH / 4,
		SCREEN_HEIGHT / 5,
	};
	SDL_Rect Input = {
		Form.x + 20,
		Form.y + 3*Form.h/5 - 20,
		Form.w - 40,
		2*Form.h / 5
	};
	SDL_Rect InputTextureDest = {
		Input.x + 5,
		Input.y + Input.h,
		0,
		0
	};
	
	SDL_Rect InputTextureClip = {
		0,
		0,
		Input.w - 10,
		Input.h - 20
	};
	SDL_Rect PromptTextureDest = {
		Input.x + 5,
		Form.y + 20,
		0,
		0
	};
	SDL_Rect AskCreateTextureDest, AskCreateNotifyTextureDest,AskCreateForm;

	std::string login = "";
	LTexture gPromptTextTexture, gInputTextTexture, gAskCreateTexture, gAskCreateNotifyTexture;
	TTF_Font * tf = TTF_OpenFont(GAME_MENU_FONT_PATH, 40);	
	gPromptTextTexture.loadFromRenderedText("Login:", { 0,0,0 }, gRenderer, tf);
	TTF_CloseFont(tf);
	tf = TTF_OpenFont(GAME_INPUT_FONT_PATH, 35);
	bool quit = false;
	bool ask_create = false;
	SDL_Event e;
	
	while (!quit)
	{
		SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 0);
		SDL_RenderClear(gRenderer);
		bool updateText = false;
		//Handle events on queue
		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_QUIT)
			{
				quit = true;				
			}
			else if (e.type == SDL_KEYDOWN) {
				SDL_Keycode s = e.key.keysym.sym;
				if (!ask_create) {
					switch (s)
					{
					case SDLK_ESCAPE:
						quit = true;
						break;
					case SDLK_BACKSPACE:
						if (login.length() > 0) {
							login.pop_back();
							updateText = true;
						}
						break;
					case SDLK_RETURN:
						if (login.length() == 0) {
							//empty login not allowed or log in as Guest
						}
						else if (login.length() > 16) {
							//too long login
						}
						else {
							if (DataStorage::userExsists(login)) {
								ChangeUser(login);
								quit = true; 
							}
							else {
								TTF_Font*tf_ask = TTF_OpenFont(GAME_OUTPUT_FONT_PATH, 35);
								gAskCreateNotifyTexture.loadFromRenderedText("User \"" + login + "\" doesn't exsit!", { 0,0,0 }, gRenderer, tf_ask);
								gAskCreateTexture.loadFromRenderedText("Create \"" + login + "\"?", { 0,0,0 }, gRenderer, tf_ask);
								TTF_CloseFont(tf_ask);
								AskCreateNotifyTextureDest = {
									(SCREEN_WIDTH - gAskCreateNotifyTexture.getWidth())/2,
									Form.y + 20,
									0,
									0
								};
								AskCreateTextureDest = {
									(SCREEN_WIDTH - gAskCreateTexture.getWidth()) / 2,
									AskCreateNotifyTextureDest.y + gAskCreateNotifyTexture.getHeight() + 15,
									0,
									0
								};
								AskCreateForm = {
									AskCreateNotifyTextureDest.x - 10,
									Form.y,
									gAskCreateNotifyTexture.getWidth() + 20,
									AskCreateTextureDest.y + gAskCreateTexture.getHeight() + 20 - Form.y
								};
								ask_create = true;
							}
						}
						break;
					case SDLK_SPACE:
						//spaces not allowed
						break;
					default:
						break;
					}
				}
				else {
					switch (s)
					{
					case SDLK_ESCAPE:
						ask_create = false;
						break;
					case SDLK_RETURN:
						ChangeUser(login);
						quit = true;
						break;
					default:
						break;
					}
				}
			}
			else if (e.type == SDL_TEXTINPUT) {
				if (!ask_create) {
					login += e.text.text;
					updateText = true;
				}
			}
		}
		if (updateText) {
			gInputTextTexture.free();
			gInputTextTexture.loadFromRenderedText(login, { 0,0,0 }, gRenderer,tf);
			InputTextureDest.y = Input.y + Input.h - gInputTextTexture.getHeight();
			if (gInputTextTexture.getWidth() > InputTextureClip.w) {
				InputTextureClip.x = gInputTextTexture.getWidth() - InputTextureClip.w;
				InputTextureDest.w = InputTextureClip.w;
			}else {
				InputTextureClip.x = 0;
				InputTextureDest.w = 0;
			}
		}
		SDL_RenderCopy(gRenderer, loginBG, NULL, NULL);
		if (ask_create) {
			SDL_SetRenderDrawColor(gRenderer, 0xFF, 0x00, 0x64, 0xFF);
			SDL_RenderFillRect(gRenderer, &AskCreateForm);
			gAskCreateNotifyTexture.render(gRenderer, &AskCreateNotifyTextureDest);
			gAskCreateTexture.render(gRenderer, &AskCreateTextureDest);
		}
		else {
			SDL_SetRenderDrawColor(gRenderer, 0x00, 0xFF, 0x64, 0xFF);
			SDL_RenderFillRect(gRenderer, &Form);
			SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
			SDL_RenderFillRect(gRenderer, &Input);
			gPromptTextTexture.render(gRenderer, &PromptTextureDest);
			gInputTextTexture.render(gRenderer, &InputTextureDest, &InputTextureClip);
		}
		SDL_RenderPresent(gRenderer);
	}
	SDL_DestroyTexture(loginBG);
	TTF_CloseFont(tf);
	gPromptTextTexture.free();
	gInputTextTexture.free();
}

void ChangeUser(std::string log)
{
	gUserData = DataStorage::getUserData(log);
}

void PlayLevel(int lvlId) {
	srand(time(NULL));	
	int arcLevInd;
	if (lvlId==-1){
		arcLevInd = 1 + rand() % (gAvailArcLevelsCnt);
	}
	else {
		arcLevInd = lvlId;
	}
	
	Level::PlayAgain = true;
	while (Level::PlayAgain)
	{
		SDL_Event e;
		Level::ShowResult = true;
		//create level
		Level currLevel = Level(gRenderer, SCREEN_WIDTH, SCREEN_HEIGHT);		
		//load level 
		if (!currLevel.Load(arcLevInd, DataStorage::getCharById(gUserData.persId))) {
			OutputDebugString("Failed to load level!\n");
			Level::PlayAgain = false;
		}
		else
		{
			currLevel.StartTimer();
			while (currLevel.isRunning())
			{
				//start game
				while (SDL_PollEvent(&e) != 0)
				{
					//User requests quit
					if (e.type == SDL_QUIT)
					{
						currLevel.ExitPrompt();
						break;
					}
					else if (e.type == SDL_KEYDOWN) {
						if (e.key.keysym.sym == SDLK_ESCAPE)
						{
							currLevel.PauseMenu();
							break;
						}
						else if (e.key.keysym.sym == SDLK_F11) {
							if (gFullScreen) {
								//OutputDebugString("Fullscreen ON");
								SDL_SetWindowFullscreen(gWindow, 0);
							}else {
								//OutputDebugString("Fullscreen OFF");
								SDL_SetWindowFullscreen(gWindow, SDL_WINDOW_FULLSCREEN);								
							}			
							gFullScreen = !gFullScreen;
						}
					}
					//Handle input for the dot
					currLevel.handleDotEvent(e);
				}
				//Move the dot
				currLevel.UpdateDot();
				currLevel.UpdateCamera();
				currLevel.UpdateDoor();
				currLevel.Draw();	
			}
			currLevel.StopTimer();

			Mix_HaltMusic();
			Mix_PlayMusic(gMainMusic, -1);
			
			if (Level::ShowResult) {
				std::vector<GMenuOption> resOpt = {
					GMenuOption("TRY AGAIN", true, { 0,0,0 }),
					GMenuOption("NEXT", false, { 0,0,0 }),
					GMenuOption("EXIT", true, { 0,0,0 })
				};
				Level::Result cResult = currLevel.GetResult();
				if (!cResult.rLost){
					UpdateData(lvlId, &currLevel);
				}
				if (lvlId <= gUserData.levelsPassed && lvlId != gOverCampLevelsCnt) {
					resOpt[1] = GMenuOption("NEXT", true, { 0,0,0 });
				}
				
				RenderResults(&currLevel);//render to texture
				GMenu ResultMenu(gRenderer,
					{0,0,SCREEN_WIDTH,SCREEN_HEIGHT},
					{ SCREEN_WIDTH / 2, 3 * SCREEN_HEIGHT / 4, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 4 },
					"Resources/Fonts/Mf_July_Sky.ttf",
					40,
					&gMenuPointer,
					&gMenuBG,					
					resOpt
				);
				ResultMenu.Show();
				switch (ResultMenu.getSelectedOption()) {
				case ResMenu::O_RETRY:
					break;
				case ResMenu::O_NEXTLVL:
					if (lvlId == -1) {
						currLevel.lLevelData.id = 1 + rand() % (gAvailArcLevelsCnt);
					}
					else {
						currLevel.lLevelData.id++;
						lvlId++;
						arcLevInd++;
					}
					break;
				case ResMenu::O_QUIT:
					Level::PlayAgain = false;
					break;
				case -1:
					Level::PlayAgain = false;
					break;
				}
			}
			if (!gUserData.UpToDate) {
				DataStorage::updateUserData(gUserData);
			}
		}
	}
	
}

void UpdateData(int lId, Level* currLevel)
{
	Level::Result res = currLevel->GetResult();
	bool updres = false;

	if (lId > gUserData.levelsPassed) {
		gUserData.levelsPassed++;
		if (gUserData.login != "Guest") {
			gUserData.UpToDate = false;
		}
	}
	if ( gUserData.login != "Guest" &&
		(currLevel->lLevelData.bestTime == 0 || (int)res.rTime < currLevel->lLevelData.bestTime)) {
		currLevel->lLevelData.bestTime = (int)res.rTime;
		currLevel->lLevelData.bestUserLogin = gUserData.login;
		DataStorage::updateBestResult(currLevel->lLevelData);
	}

	sUsersLevelResult lvlResult = DataStorage::getUsersLevelResult(gUserData, currLevel->lLevelData);
	int gotBonusCoins = res.rGotCoins - currLevel->lLevelData.reqCoins;
	if ( gotBonusCoins > lvlResult.userBestBonusCoins) {
		gUserData.coins += gotBonusCoins - lvlResult.userBestBonusCoins;
		lvlResult.userBestBonusCoins = gotBonusCoins;
		if (gUserData.login != "Guest") {
			gUserData.UpToDate = false;
			updres = true;
		}
	}
	if (lvlResult.userBestTime == 0 || (int)res.rTime < lvlResult.userBestTime) {
		lvlResult.userBestTime = res.rTime;
		if (gUserData.login != "Guest") {
			updres = true;
		}
	}
	if(updres){
		DataStorage::updateUsersLevelResult(gUserData, currLevel->lLevelData, lvlResult);
	}
}

void ShowSettings()
{
	enum SettOptions {
		SO_FIRST,
		SO_MUSIC,
		SO_SFX,
		SO_LAST
	};
	int selOpt = SettOptions::SO_MUSIC;

	bool quit = false;
	SDL_Event e;

	int VolumeSteps = 10;
	int MusicVolumeStep = (int)(VolumeSteps*MusicVolume);
	int SFXVolumeStep = (int)(VolumeSteps*SFXVolume);

	LTexture BgTexture;
	BgTexture.loadFromFile("Resources/Common/cust_back.png", gRenderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_Rect BgDest = { 0,0,0,0 };

	LTexture MusicVolumePrompt;
	TTF_Font* tf = TTF_OpenFont(GAME_MENU_FONT_PATH, 40);
	MusicVolumePrompt.loadFromRenderedText("Music volume", { 0,0,0 }, gRenderer, tf);
	SDL_Rect MusicVolumePromptDest = {
		(SCREEN_WIDTH - MusicVolumePrompt.getWidth()) / 2,
		(int)(0.4*SCREEN_HEIGHT) - MusicVolumePrompt.getHeight(),
		MusicVolumePrompt.getWidth(),
		MusicVolumePrompt.getHeight()
	};
	SDL_Rect MusicVolumeControlDest{
		SCREEN_WIDTH/3,
		MusicVolumePromptDest.y + MusicVolumePromptDest.h,
		SCREEN_WIDTH/3,
		(int)(0.1*SCREEN_HEIGHT)
	};
	SDL_Rect MusicVolumeControlPartDest = {
		MusicVolumeControlDest.x,
		MusicVolumeControlDest.y + (int)(0.2*MusicVolumeControlDest.h),
		(int)(MusicVolumeControlDest.w/VolumeSteps*0.6),
		(int)(0.9*MusicVolumeControlDest.h)
	};
	LTexture SFXVolumePrompt;//sound effects
	SFXVolumePrompt.loadFromRenderedText("Sound effects volume", { 0,0,0 }, gRenderer, tf);
	SDL_Rect SFXVolumePromptDest = {
		(SCREEN_WIDTH - SFXVolumePrompt.getWidth()) / 2,
		MusicVolumeControlDest.y + (int)(1.5*MusicVolumeControlDest.h),
		SFXVolumePrompt.getWidth(),
		SFXVolumePrompt.getHeight()
	};
	SDL_Rect SFXVolumeControlDest{
		SCREEN_WIDTH/3,
		SFXVolumePromptDest.y + SFXVolumePromptDest.h,
		SCREEN_WIDTH/3,
		(int)(0.1*SCREEN_HEIGHT)
	};
	SDL_Rect SFXVolumeControlPartDest = {
		SFXVolumeControlDest.x,
		SFXVolumeControlDest.y + (int)(0.2*SFXVolumeControlDest.h),
		(int)(SFXVolumeControlDest.w / VolumeSteps*0.6),
		(int)(0.9*SFXVolumeControlDest.h)
	};

	SDL_Rect gMenuPointerDest = {
		0,
		0,
		MusicVolumePromptDest.h,
		MusicVolumePromptDest.h
	};
	while(!quit) {
		SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 0);
		SDL_RenderClear(gRenderer);
		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_KEYDOWN) {
				SDL_Keycode s = e.key.keysym.sym;
				switch (s)
				{
				case SDLK_ESCAPE:
					quit = true;
					break;
				case SDLK_RETURN:
					break;
				case SDLK_LEFT:
					if (selOpt == SettOptions::SO_MUSIC && MusicVolumeStep>0) {
						MusicVolumeStep--;
						MusicVolume = (double)MusicVolumeStep / VolumeSteps;
						Mix_VolumeMusic((int)(MusicVolume*MIX_MAX_VOLUME));
						
					}else if (selOpt == SettOptions::SO_SFX && SFXVolumeStep>0) {
						SFXVolumeStep--;
						SFXVolume = (double)SFXVolumeStep / VolumeSteps;
						Mix_Volume(-1, (int)(SFXVolume*MIX_MAX_VOLUME));
						Mix_PlayChannel(-1, gSFXSample, 0);
					}
					break;
				case SDLK_RIGHT:
					if (selOpt == SettOptions::SO_MUSIC && MusicVolumeStep<VolumeSteps) {
						MusicVolumeStep++;
						MusicVolume = (double)MusicVolumeStep / VolumeSteps;
						Mix_VolumeMusic((int)(MusicVolume*MIX_MAX_VOLUME));
					}
					else if (selOpt == SettOptions::SO_SFX && SFXVolumeStep<VolumeSteps) {
						SFXVolumeStep++;
						SFXVolume = (double)SFXVolumeStep / VolumeSteps;
						Mix_Volume(-1, (int)(SFXVolume*MIX_MAX_VOLUME));
						Mix_PlayChannel(-1, gSFXSample, 0);
					}
					break;
				case SDLK_UP:
					if (selOpt > SettOptions::SO_FIRST + 1) {
						selOpt--;
					}
					break;
				case SDLK_DOWN:
					if (selOpt < SettOptions::SO_LAST - 1) {
						selOpt++;
					}
					break;
				default:
					break;
				}
			}
		}
		if (selOpt == SettOptions::SO_MUSIC) {
			gMenuPointerDest.x = MusicVolumePromptDest.x - gMenuPointerDest.w-20;
			gMenuPointerDest.y = MusicVolumePromptDest.y;
		}
		else if (selOpt == SettOptions::SO_SFX) {
			gMenuPointerDest.x = SFXVolumePromptDest.x - gMenuPointerDest.w-20;
			gMenuPointerDest.y = SFXVolumePromptDest.y;
		}

		BgTexture.render(gRenderer, &BgDest);
		gMenuPointer.render(gRenderer, &gMenuPointerDest);
		MusicVolumePrompt.render(gRenderer, &MusicVolumePromptDest);
		SFXVolumePrompt.render(gRenderer, &SFXVolumePromptDest);
		SDL_SetRenderDrawColor(gRenderer, 0x00, 0xFF, 0x00, 0x00);
		MusicVolumeControlPartDest.x = MusicVolumeControlDest.x;
		for (int i = 0; i < MusicVolumeStep; i++) {
			SDL_RenderFillRect(gRenderer, &MusicVolumeControlPartDest);
			MusicVolumeControlPartDest.x += MusicVolumeControlDest.w/VolumeSteps;
		}
		SFXVolumeControlPartDest.x = SFXVolumeControlDest.x;
		for (int i = 0; i < SFXVolumeStep; i++) {
			SDL_RenderFillRect(gRenderer, &SFXVolumeControlPartDest);
			SFXVolumeControlPartDest.x += SFXVolumeControlDest.w / VolumeSteps;
		}
		SDL_RenderPresent(gRenderer);
	}

}

void ShowCustomize()
{
	enum CustOptions {
		CO_FIRST,
		CO_CHAR,
		CO_STORE,
		CO_SAVE,
		CO_CANCEL,
		CO_LAST
	};
	int selOpt = CustOptions::CO_CHAR;
	const int SAMPLE_SIZE = 80;
	int selChar = 0;
	bool quit = false;
	SDL_Event e;

	LTexture BgTexture;
	BgTexture.loadFromFile("Resources/Common/cust_back.png", gRenderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_Rect BgDest = { 0,0,0,0 };
	std::vector <Character> Chars = DataStorage::getChars();
	LTexture CharLabelTexture,StoreLabelTexture,SaveLabelTexture,CancelLabelTexture;	
	CharLabelTexture.loadFromRenderedText("Character", { 0,0,0 }, gRenderer);
	CharLabelTexture.setColor(0x00, 0xFF, 0x00);
	SDL_Rect CharLabelDest = { 20, 20, CharLabelTexture.getWidth(), CharLabelTexture.getHeight() };
	SDL_Rect CharsForm = {
		CharLabelDest.x,
		CharLabelDest.y + CharLabelDest.h + 20,
		SCREEN_WIDTH - 2*CharLabelDest.x,
		SCREEN_HEIGHT/2
	};
	int charsCnt = Chars.size();
	std::vector<LTexture> CharTextures(charsCnt, LTexture());
	std::vector<SDL_Rect> CharDests(charsCnt);
	for (int i = 0; i < charsCnt; i++) {
		CharTextures[i].loadFromFile(Chars[i].path, gRenderer, SAMPLE_SIZE, SAMPLE_SIZE);
		CharDests[i] = { CharsForm.x + (int)(i*1.5*SAMPLE_SIZE), CharsForm.y, SAMPLE_SIZE, SAMPLE_SIZE };
		if (Chars[i].id == gUserData.persId) {
			selChar = i;
		}
	}

	StoreLabelTexture.loadFromRenderedText("Store", { 0,0,0 }, gRenderer);
	StoreLabelTexture.setColor(0x00, 0xFF, 0x00);
	SaveLabelTexture.loadFromRenderedText("SAVE", { 0,0,0 }, gRenderer);
	SaveLabelTexture.setColor(0x00, 0xFF, 0x00);
	CancelLabelTexture.loadFromRenderedText("CANCEL", { 0,0,0 }, gRenderer);
	CancelLabelTexture.setColor(0x00, 0xFF, 0x00);

	SDL_Rect StoreLabelDest = { 20, SCREEN_HEIGHT / 2, 0, 0 };
	SDL_Rect CancelLabelDest = { 
		20,
		SCREEN_HEIGHT - CancelLabelTexture.getHeight() - 5,
		CancelLabelTexture.getWidth(),
		CancelLabelTexture.getHeight()
	};
	SDL_Rect SaveLabelDest = {
		20,
		CancelLabelDest.y - CancelLabelDest.h,
		SaveLabelTexture.getWidth(),
		SaveLabelTexture.getHeight()
	};
	while (!quit)
	{
		SDL_SetRenderDrawColor(gRenderer, 255, 255, 255, 0);
		SDL_RenderClear(gRenderer);
		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_QUIT)
			{
				quit = true;
			}
			else if (e.type == SDL_KEYDOWN) {
				SDL_Keycode s = e.key.keysym.sym;
				switch (s)
				{
				case SDLK_ESCAPE:
					quit = true;
					break;
				case SDLK_RETURN:
					if (selOpt == CustOptions::CO_SAVE) {
						gUserData.persId = Chars[selChar].id;
						if (gUserData.login != "Guest") {
							gUserData.UpToDate = false;
						}
						quit = true;
					}
					else if (selOpt == CustOptions::CO_CANCEL) {
						quit = true;
					}
					break;
				case SDLK_LEFT:					
					if (selOpt == CustOptions::CO_CHAR && selChar > 0) {
						selChar--;
					}
					break;
				case SDLK_RIGHT:
					if (selOpt == CustOptions::CO_CHAR && selChar < charsCnt - 1) {
						selChar++;
					}
					break;
				case SDLK_UP:
					if (selOpt > CustOptions::CO_FIRST + 1) {
						selOpt--;
					}
					break;
				case SDLK_DOWN:
					if (selOpt < CustOptions::CO_LAST - 1) {
						selOpt++;
					}
					break;
				default:
					break;
				}
			}
		}
		

		BgTexture.render(gRenderer, &BgDest);
		if (selOpt == CustOptions::CO_CHAR) {
			//SDL_RenderDrawRect(gRenderer, &SaveLabelDest);
			CharLabelTexture.setBlendMode(SDL_BLENDMODE_MOD);
		} else {
			CharLabelTexture.setBlendMode(SDL_BLENDMODE_BLEND);
		}
		CharLabelTexture.render(gRenderer, &CharLabelDest);
		for (int i = 0; i < charsCnt; i++) {
			if ( i == selChar) {
				SDL_SetRenderDrawColor(gRenderer, 0x00, 0xFF, 0x00, 0xFF);
				SDL_RenderDrawRect(gRenderer, &CharDests[i]);
			}
			CharTextures[i].render(gRenderer, &CharDests[i]);
		}

		if (selOpt == CustOptions::CO_STORE) {
			//SDL_RenderDrawRect(gRenderer, &SaveLabelDest);
			StoreLabelTexture.setBlendMode(SDL_BLENDMODE_MOD);
		}
		else {
			StoreLabelTexture.setBlendMode(SDL_BLENDMODE_BLEND);
		}
		StoreLabelTexture.render(gRenderer, &StoreLabelDest);
		
		if (selOpt == CustOptions::CO_SAVE) {
			//SDL_RenderDrawRect(gRenderer, &SaveLabelDest);
			SaveLabelTexture.setBlendMode(SDL_BLENDMODE_MOD);
		}
		else {
			SaveLabelTexture.setBlendMode(SDL_BLENDMODE_BLEND);
		}
		SaveLabelTexture.render(gRenderer, &SaveLabelDest);

		if (selOpt == CustOptions::CO_CANCEL) {
			//SDL_RenderDrawRect(gRenderer, &CancelLabelDest);
			CancelLabelTexture.setBlendMode(SDL_BLENDMODE_MOD);
		}
		else {
			CancelLabelTexture.setBlendMode(SDL_BLENDMODE_BLEND);
		}
		CancelLabelTexture.render(gRenderer, &CancelLabelDest);
		SDL_RenderPresent(gRenderer);
	}
}

void close()
{
	//Free loaded images
	gMenuBG.free();
	gMenuPointer.free();
	//Free the music 
	Mix_FreeMusic( gMainMusic ); 
	gMainMusic = NULL;	
	//Destroy window	
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gWindow = NULL;
	gRenderer = NULL;

	//Quit SDL subsystems
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}


