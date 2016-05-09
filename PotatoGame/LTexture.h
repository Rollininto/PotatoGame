#pragma once
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <Windows.h>
#include <WinBase.h>
#include <sstream>


//LTexture class
class LTexture
{
public:
	//Initializes variables
	LTexture();
	LTexture(SDL_Texture * sdlTexture, int w, int h);
	LTexture(SDL_Renderer* gRen, SDL_Surface* sdlSurf);

	//Deallocates memory
	~LTexture();

	//Loads image at specified path
	bool loadFromFile(std::string path, SDL_Renderer* mRenderer, int newWidth = 0, int newHeight = 0);

#ifdef _SDL_TTF_H
	//Creates image from font string
	bool loadFromRenderedText(std::string textureText, SDL_Color textColor, SDL_Renderer* mRenderer, TTF_Font* tFont = NULL);
#endif
	//Creates blank texture
	bool createBlank(SDL_Renderer* gR,int width, int height, SDL_TextureAccess access = SDL_TEXTUREACCESS_STREAMING);

	//Deallocates texture
	void free();

	//Set color modulation
	void setColor(Uint8 red, Uint8 green, Uint8 blue);

	//Set blending
	void setBlendMode(SDL_BlendMode blending);

	//Set alpha modulation
	void setAlpha(Uint8 alpha);

	//Renders texture at given point
	void render( SDL_Renderer* mRenderer, SDL_Rect* dest, SDL_Rect* clip = NULL, SDL_Rect* tClips = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE);
	//Set self as render target
	void setAsRenderTarget(SDL_Renderer * gR);
	//Gets image dimensions
	int getWidth();
	int getHeight();
	void setWidth(int w);
	void setHeight(int h);

private:
	//The actual hardware texture
	SDL_Texture* mTexture;
	//Image dimensions
	int mWidth;
	int mHeight;
};