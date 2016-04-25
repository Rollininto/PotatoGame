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
	O_CAMPAIGN,
	O_ARCADE,
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

bool init();
bool loadMedia();
void RenderResults(Level::Result r);
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
			int menuOptionSel = -1;

			//Event handler
			SDL_Event e;

			//While application is running
			while (!quit)
			{	
				//Clear screen
				SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
				SDL_RenderClear(gRenderer);
			
				GMenu MainMenu(gRenderer,
					{ "CAMPAIGN","ARCADE","OPTIONS","EXIT" },
					{0, SCREEN_HEIGHT/2, SCREEN_WIDTH, SCREEN_HEIGHT/2},
					"Resources/Fonts/Mf_July_Sky.ttf",
					40,
					{0,0,0},
					&gMenuPointer
				);
				MainMenu.Show();
				menuOptionSel = MainMenu.getSelectedOption();
				if ( menuOptionSel == MainMenu::O_ARCADE)
				{		
					Level::PlayAgain = true;
					while (Level::PlayAgain)
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
									if (e.type == SDL_QUIT)
									{
										currLevel.exit();
										break;
									}
									else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)
									{
										currLevel.pause();
										break;
									}

									//Handle input for the dot
									currLevel.handleDotEvent(e);
								}

								//Move the dot
								currLevel.UpdateDot();
								
								currLevel.UpdateCamera();
								currLevel.Draw();
							}
							if (!Level::PlayAgain) break;

							currLevel.StopTimer();
							Level::Result cResult = currLevel.GetResult();
							RenderResults(cResult);//render to texture
							GMenu ResultMenu(gRenderer,
								{ "TRY AGAIN","EXIT" },
								{ SCREEN_WIDTH / 2, 3*SCREEN_HEIGHT / 4, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 4 },
								"Resources/Fonts/Mf_July_Sky.ttf",
								40,
								{ 0,0,0 },
								&gMenuPointer,
								&gMenuBG
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
				else if (menuOptionSel == MainMenu::O_CAMPAIGN) {
					GMenu CampaignMenu(gRenderer,
						{ "Level 1","Level 2","Level 3" ,"Level 4" },
						{ 0, 0, SCREEN_WIDTH , SCREEN_HEIGHT },
						"Resources/Fonts/Mf_July_Sky.ttf",
						40,
						{ 0,0,0 },
						&gMenuPointer,
						NULL
					);
					CampaignMenu.Show();
					switch (CampaignMenu.getSelectedOption())
					{
					default:
						break;
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

void RenderResults(Level::Result r) {
	
	LTexture* rsTimeTexture = new LTexture();
	LTexture* rsCoinsTexture = new LTexture();
	LTexture rsResultTexture;
	LTexture ret;
	gMenuBG.createBlank(gRenderer, SCREEN_WIDTH, SCREEN_HEIGHT,SDL_TEXTUREACCESS_TARGET);
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
	
	gMenuBG.setAsRenderTarget(gRenderer);
	//Clear screen
	SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
	SDL_RenderClear(gRenderer);

	if (rsCoinsTexture != NULL)
		rsCoinsTexture->render(gRenderer, &rsCoinsDest);
	if (rsTimeTexture != NULL)
		rsTimeTexture->render(gRenderer, &rsTimeDest);
	rsResultTexture.render(gRenderer, &rsResultDest);
	
	SDL_SetRenderTarget(gRenderer, NULL);
//	gMenuBG.render(gRenderer, new SDL_Rect({ 0,0,0,0 }));

//	SDL_RenderPresent(gRenderer);

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


