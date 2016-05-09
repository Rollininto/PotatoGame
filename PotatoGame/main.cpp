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
#include "Level.h"
#include "GMenu.h"
#include "DataStorage.h"

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

//The music that will be played in main menu
Mix_Music *gMainMusic = NULL;


std::string UserName = "Guest";
int gAvailArcLevelsCnt;
int gAvailCampLevelsCnt;
int gOverCampLevelsCnt;
bool gFullScreen;

bool init();
bool loadMedia();
void GetAvailableGameData();
void RenderResults(Level::Result r);
void ShowCampaignMenu();
void ShowLogInForm();
void ChangeUser(std::string log);
void PlayLevel(int lvlId = -1);
void ShowSettings();
void close();


int main(int argc, char* args[])
{
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
			//SDL_SetWindowFullscreen(gWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);			
			//Main loop flag
			bool quit = false;
			int menuOptionSel = -1;

			//Event handler
			//SDL_Event e;

			//While application is running
			while (!quit)
			{	
				if (Mix_PlayingMusic() == 0) { 
					//Play the music 
					Mix_PlayMusic( gMainMusic, -1 ); 
				}
				//Clear screen
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
				SDL_RenderClear(gRenderer);
			
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
					GMenuInfo("Logged in as: "+UserName),
					GMenuInfo("To log in press F3!")
				}
				);
				MainMenu.addOption(SDLK_F3, &ShowLogInForm);
				MainMenu.Show();
				menuOptionSel = MainMenu.getSelectedOption();
				if ( menuOptionSel == MainMenu::O_ARCADE)
				{		
					PlayLevel();
				}
				else if (menuOptionSel == MainMenu::O_CAMPAIGN) {
					GMenu CampaignMenu(gRenderer,
						{0,0,SCREEN_WIDTH,SCREEN_HEIGHT},
						//{ "Level 1","Level 2","Level 3" ,"Level 4","EXIT" },
						//{ 1, 0, 0, 0, 1},
						{ 0, SCREEN_HEIGHT/5, SCREEN_WIDTH , 4*SCREEN_HEIGHT/5 },
						"Resources/Fonts/Mf_July_Sky.ttf",
						40,
						//{ 0,0,0 },
						&gMenuPointer,
						&gMainMenuBg,
						{
							GMenuOption("Level 1",true,{ 0,0,0 }),
							GMenuOption("Level 2",false,{ 0,0,0 }),
							GMenuOption("Level 3",false,{ 0,0,0 }),
							GMenuOption("Level 4",false,{ 0,0,0 }),
							GMenuOption("EXIT",true,{ 0,0,0 })
						}
					);
					CampaignMenu.Show();
					switch (CampaignMenu.getSelectedOption())
					{
					case 0:
						PlayLevel(1);
						break;
					case 1:
						PlayLevel(2);
						break;
					case 2:
						PlayLevel(3);
						break;
					default:
						break;
					}
				}
				else if (menuOptionSel == MainMenu::O_SETTINGS) {
					ShowSettings();
				}
				else if (menuOptionSel == MainMenu::O_EXIT) {
					quit = true;
				}

				//Update screen
				SDL_RenderPresent(gRenderer);
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
	return success;
}

void GetAvailableGameData() {
	gAvailArcLevelsCnt = 3;
	gAvailCampLevelsCnt = 1;
	gOverCampLevelsCnt = 3;
}

void RenderResults(Level::Result r) {
	
	LTexture* rsTimeTexture = new LTexture();
	LTexture* rsCoinsTexture = new LTexture();
	LTexture rsResultTexture;
	LTexture ret;
	gMenuBG.createBlank(gRenderer, SCREEN_WIDTH, SCREEN_HEIGHT,SDL_TEXTUREACCESS_TARGET);
	TTF_Font* tF;

	//result textures
	if (r.rLost) {
		tF = TTF_OpenFont("Resources/Fonts/TheyPerished.ttf", 65);
		rsResultTexture.loadFromRenderedText("YOU LOST...", {255,0,0},gRenderer,tF);
		rsCoinsTexture = NULL;
		rsTimeTexture = NULL;
	}
	else {
		tF = TTF_OpenFont("Resources/Fonts/Flyknit.ttf", 65);
		rsResultTexture.loadFromRenderedText("YOU WON!!!", { 0,255,0 }, gRenderer, tF);
		tF = TTF_OpenFont("Resources/Fonts/Flyknit.ttf", 40);
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
	
	SDL_Rect rsResultDest = {
		(SCREEN_WIDTH - rsResultTexture.getWidth())/2 ,
		50,
		0,
		0
	};
	SDL_Rect rsCoinsDest = {
		rsResultDest.x,
		120,
		0,
		0
	};
	SDL_Rect rsTimeDest = {
		rsResultDest.x,
		150,
		0,
		0
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
	
	SDL_SetRenderTarget(gRenderer, NULL);
//	gMenuBG.render(gRenderer, new SDL_Rect({ 0,0,0,0 }));

//	SDL_RenderPresent(gRenderer);

}

void ShowCampaignMenu()
{
	//campaign menu
	//probably shold do map-like
}

void ShowLogInForm()
{
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

	std::string login = "";
	TTF_Font * tf = TTF_OpenFont("Resources/Fonts/Mf_July_Sky.ttf", 35);
	LTexture gPromptTextTexture, gInputTextTexture;
	gPromptTextTexture.loadFromRenderedText("Login:", { 0,0,0 }, gRenderer, tf);
	tf = TTF_OpenFont("Resources/Fonts/airstrike.ttf", 35);
	gInputTextTexture.loadFromRenderedText(login, { 0,0,0 }, gRenderer, tf);
	bool quit = false;
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
				switch (s)
				{
				case SDLK_ESCAPE:
					quit = true;
					break;
				case SDLK_BACKSPACE:
					if(login.length()>0){
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
						ChangeUser(login);
						quit = true;
					}
					break;
				case SDLK_SPACE:
					//spaces not allowed
					break;
				default:
					break;
				}
			}
			else if (e.type == SDL_TEXTINPUT) {
				login += e.text.text;
				updateText = true;
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
		SDL_SetRenderDrawColor(gRenderer, 0x00, 0xFF, 0x100, 0xFF);
		SDL_RenderFillRect(gRenderer, &Form);
		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderFillRect(gRenderer, &Input);
		gPromptTextTexture.render(gRenderer, &PromptTextureDest);

		gInputTextTexture.render(gRenderer, &InputTextureDest,&InputTextureClip);
		SDL_RenderPresent(gRenderer);
	}
}

void ChangeUser(std::string log)
{
	UserName = log;
}

void PlayLevel(int lvlId) {
	srand(time(NULL));
	int arcLevInd;
	if (lvlId==-1){
		//int arcLevInd = 1 + rand() % gAvailArcLevelsCnt;
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
		if (!currLevel.Load(arcLevInd, "dot_1")) {
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
						currLevel.exit();
						break;
					}
					else if (e.type == SDL_KEYDOWN) {
						if (e.key.keysym.sym == SDLK_ESCAPE)
						{
							currLevel.pause();
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
				currLevel.Draw();
			}

			Mix_HaltMusic();
			Mix_PlayMusic(gMainMusic, -1);

			currLevel.StopTimer();
			if (Level::ShowResult) {
				Level::Result cResult = currLevel.GetResult();
				RenderResults(cResult);//render to texture
				GMenu ResultMenu(gRenderer,
					{0,0,SCREEN_WIDTH,SCREEN_HEIGHT},
					//{ "TRY AGAIN","EXIT" },
					//{ 1, 1, 1 },
					{ SCREEN_WIDTH / 2, 3 * SCREEN_HEIGHT / 4, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 4 },
					"Resources/Fonts/Mf_July_Sky.ttf",
					40,
					//{ 0,0,0 },
					&gMenuPointer,
					&gMenuBG,
					{
						GMenuOption("TRY AGAIN",true,{ 0,0,0 }),
						GMenuOption("EXIT",true,{ 0,0,0 })
					}
					);
				ResultMenu.Show();
				switch (ResultMenu.getSelectedOption()) {
				case ResMenu::O_RETRY:
					break;
				case ResMenu::O_QUIT:
					Level::PlayAgain = false;
					break;
				}
			}
		}
	}
	
}

void ShowSettings()
{
	//settings window


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


