#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <Windows.h>
#include <WinBase.h>
#include "LTexture.h"
#include "Level.h"
#include "GMenu.h"

//Screen dimension constants
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

enum MainMenu {
	BEGIN_BORDER,
	O_START,
	O_EXIT,
	END_BORDER
};

enum ResMenu {
	B_BORDER,
	O_RETRY,
	O_QUIT,
	E_BORDER
};
//Global variables

	//The window we'll be rendering to
SDL_Window* gWindow = NULL;

	//The window renderer
SDL_Renderer* gRenderer = NULL;

LTexture gMenuStart;
LTexture gMenuExit;
LTexture gMenuPointer;

bool init();
bool loadMedia();
int ShowResultScreen(Level::Result r);
void close();


int main(int argc, char* args[])
{
	//Start up SDL and create window
	if (!init())
	{
		printf("Failed to initialize!\n");
	}
	else
	{
		if (!loadMedia())
		{
			printf("Failed to initialize!\n");
		}
		else 
		{		
			//if we are dubugging with breakpoints it would be impossible to switch to VS from fullscreen
			if (IsDebuggerPresent()) {
				SDL_SetWindowBordered(gWindow, SDL_FALSE);
			}
			else {
				SDL_SetWindowFullscreen(gWindow, SDL_WINDOW_FULLSCREEN);
			}

			//SDL_SetWindowBordered(gWindow, SDL_FALSE);
			//SDL_SetWindowFullscreen(gWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);			
			//Main loop flag
			bool quit = false;
			int menuOption = MainMenu::O_START, menuOptionSel = MainMenu::BEGIN_BORDER;

			//Event handler
			SDL_Event e;

			//While application is running
			while (!quit)
			{				

				//Handle events on queue
				/*while (SDL_PollEvent(&e) != 0)
				{
					//User requests quit
					if (e.type == SDL_QUIT || (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE))
					{
						quit = true;
					}
					else if (e.type == SDL_KEYDOWN){
						SDL_Keycode s = e.key.keysym.sym;
						switch (s)
						{
						case SDLK_DOWN:
							if(menuOption<MainMenu::END_BORDER - 1)
								menuOption++;
							break;
						case SDLK_UP:
							if (menuOption>MainMenu::BEGIN_BORDER + 1)
								menuOption--;
							break;
						case SDLK_RETURN:
							menuOptionSel = menuOption;
							break;
						default:
							break;
						}						
					}
				}

				//Clear screen
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
				SDL_RenderClear(gRenderer);
				
				//render main menu
				TTF_Font* tF = TTF_OpenFont("Resources/Fonts/Mf_July_Sky.ttf", 25);//font could be unavailable
				if (gMenuStart.loadFromRenderedText("START", { 0x00,0x00,0x00 }, gRenderer, tF)) {
					SDL_Rect dest = {
						(SCREEN_WIDTH - gMenuStart.getWidth())/2,
						SCREEN_HEIGHT- 180 + MainMenu::O_START*30,
						0,
						0 
					};
					gMenuStart.render(gRenderer, &dest);
				}
				if (gMenuExit.loadFromRenderedText("EXIT", { 0x00,0x00,0x00 }, gRenderer, tF)) {
					SDL_Rect dest = {
						(SCREEN_WIDTH - gMenuExit.getWidth()) / 2,
						SCREEN_HEIGHT - 180 + MainMenu::O_EXIT*30,
						0,
						0
					};
					gMenuExit.render(gRenderer, &dest);
				}
				SDL_Rect dest = {
					(SCREEN_WIDTH) / 2 - 80,
					SCREEN_HEIGHT - 177 + menuOption * 30,
					20,
					20
				};
				gMenuPointer.render(gRenderer,&dest);
				*/
				if ( menuOptionSel == MainMenu::O_START)
				{					
					//create level
					Level currLevel = Level(gRenderer, SCREEN_WIDTH, SCREEN_HEIGHT);

					//load level 
					if (!currLevel.Load("level_1", "dot_1")) {
						printf("Failed to initialize!\n");

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
								if (e.type == SDL_QUIT )
								{
									currLevel.exit();
									menuOptionSel = MainMenu::O_EXIT;
									break;
								}

								//Handle input for the dot
								currLevel.handleDotEvent(e);
							}

							if (currLevel.isLost())
							{
								currLevel.StopTimer();
								//"failure"-animation and GAMEOVER screen
 								if (currLevel.potato->fail_anim() == 1) {
									SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
									SDL_RenderClear(gRenderer);
									SDL_RenderPresent(gRenderer);
									break;
								}
							}
							else
							{
								//Move the dot
								currLevel.potato->move();
							}
							currLevel.UpdateCamera();
							currLevel.Draw();
						}
						currLevel.StopTimer();
						if(menuOptionSel != MainMenu::O_EXIT){
							Level::Result cResult = currLevel.GetResult();
							int retry = ShowResultScreen(cResult);
							if (retry == ResMenu::O_QUIT) {
								menuOptionSel = MainMenu::BEGIN_BORDER;
							}
							else if (retry == ResMenu::O_RETRY) {
								menuOptionSel = MainMenu::O_START;
							}
						}						
					}					
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
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		{
			printf("Warning: Linear texture filtering not enabled!");
		}

		//Create window
		gWindow = SDL_CreateWindow("Ziemniak", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (gWindow == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Create vsynced renderer for window
			gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (gRenderer == NULL)
			{
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
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
					printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
					success = false;
				}
				//Initialize SDL_ttf 
				if( TTF_Init() == -1 ) 
				{
					printf( "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError() );
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
	if (!gMenuPointer.loadFromFile("Resources/pointer.png", gRenderer))
	{
		printf("Failed to load coin block texture!\n");
		success = false;
	}
	//load game main screen textures and lables(fonts,strings etc.)

	return success;
}

int ShowResultScreen(Level::Result r) {
	
	LTexture* rsTimeTexture = new LTexture();
	LTexture* rsCoinsTexture = new LTexture();
	LTexture rsResultTexture;
	LTexture rsRetryOptTexture;
	LTexture rsExitOptTexture;
	
	int menuOption = ResMenu::O_RETRY, menuOptionSel = -1;
	bool quit = false;
	SDL_Event e;
	TTF_Font* tF;

	//result textures
	SDL_Rect rsCoinsDest = {
		100,
		120,
		0,
		0
	};
	SDL_Rect rsTimeDest = {
		100,
		150,
		0,
		0
	};
	SDL_Rect rsResultDest = {
		100,
		50,
		0,
		0
	};

	if (r.rLost) {
		tF = TTF_OpenFont("Resources/Fonts/TheyPerished.ttf", 45);
		rsResultTexture.loadFromRenderedText("YOU LOST...", {255,0,0},gRenderer,tF);
		rsCoinsTexture = NULL;
		rsTimeTexture = NULL;
	}
	else {
		tF = TTF_OpenFont("Resources/Fonts/Flyknit.ttf", 45);
		rsResultTexture.loadFromRenderedText("YOU WON!!!", { 0,255,0 }, gRenderer, tF);
		tF = TTF_OpenFont("Resources/Fonts/Flyknit.ttf", 25);
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

	//menu textures
	tF = TTF_OpenFont("Resources/Fonts/Mf_July_Sky.ttf", 25);//font could be unavailable
	
	rsRetryOptTexture.loadFromRenderedText("TRY AGAIN", { 0x00,0x00,0x00 }, gRenderer, tF);
	SDL_Rect rsRetryOptDest = {
		(SCREEN_WIDTH - gMenuStart.getWidth()) / 2,
		SCREEN_HEIGHT - 180 + MainMenu::O_START * 30,
		0,
		0
	};	
	
	rsExitOptTexture.loadFromRenderedText("EXIT", { 0x00,0x00,0x00 }, gRenderer, tF);
	SDL_Rect rsExitOptDest = {
		(SCREEN_WIDTH - gMenuExit.getWidth()) / 2,
		SCREEN_HEIGHT - 180 + MainMenu::O_EXIT * 30,
		0,
		0
	};
	
	SDL_Rect rsPointerDest = {
		(SCREEN_WIDTH) / 2 - 80,
		SCREEN_HEIGHT - 177 + menuOption * 30,
		20,
		20
	};
	
	while (!quit)
	{
		//Handle events on queue
		while (SDL_PollEvent(&e) != 0)
		{
			//User requests quit
			if (e.type == SDL_QUIT )
			{
				quit = true;
			}
			else if (e.type == SDL_KEYDOWN) {
				SDL_Keycode s = e.key.keysym.sym;
				switch (s)
				{
				case SDLK_DOWN:
					if (menuOption < ResMenu::E_BORDER - 1)
						menuOption++;
					break;
				case SDLK_UP:
					if (menuOption > ResMenu::B_BORDER + 1)
						menuOption--;
					break;
				case SDLK_RETURN:
					menuOptionSel = menuOption;
					break;
				default:
					break;
				}
			}
		}

		//Clear screen
		SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
		SDL_RenderClear(gRenderer);

		if (rsCoinsTexture != NULL)
			rsCoinsTexture->render(gRenderer, &rsCoinsDest);
		if (rsTimeTexture != NULL)
			rsTimeTexture->render(gRenderer, &rsTimeDest);
		rsResultTexture.render(gRenderer, &rsResultDest);

		rsExitOptTexture.render(gRenderer, &rsExitOptDest);
		rsRetryOptTexture.render(gRenderer, &rsRetryOptDest);
		rsPointerDest.y = SCREEN_HEIGHT - 177 + menuOption * 30;
		gMenuPointer.render(gRenderer, &rsPointerDest);
		SDL_RenderPresent(gRenderer);
		if (menuOptionSel != -1)
			quit = true;
	}
	return menuOptionSel;

}

void close()
{
	//Free loaded images
	
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


