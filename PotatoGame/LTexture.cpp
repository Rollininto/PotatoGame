#include "LTexture.h"

LTexture::LTexture()
{
	//Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

LTexture::LTexture(SDL_Texture* sdlTexture, int w, int h)
{
	mTexture = sdlTexture;
	mWidth = w;
	mHeight = h;
}

LTexture::LTexture(SDL_Renderer* gRen, SDL_Surface* sdlSurf)
{
	SDL_Texture* newTexture = SDL_CreateTextureFromSurface(gRen, sdlSurf);
	if (newTexture == NULL)
	{
		std::stringstream ss;
		ss << "Unable to create texture from surface! SDL_image Error:" << IMG_GetError();
		OutputDebugString(ss.str().c_str());
	}
	else
	{
		//Get image dimensions
			mWidth = sdlSurf->w;
			mHeight = sdlSurf->h;
	}
	mTexture = newTexture;
}

LTexture::~LTexture()
{
	//Deallocate
	free();
}

bool LTexture::loadFromFile(std::string path, SDL_Renderer* mRenderer, int newWidth, int newHeight)
{
	//Get rid of preexisting texture
	free();

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == NULL)
	{
		std::stringstream ss;
		ss << "Unable to load image "+ path +"! SDL_image Error:"<< IMG_GetError();
		OutputDebugString(ss.str().c_str());
	}
	else
	{
		//Set color key image or use png with alpha?
		//SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));

		//Create texture from surface pixels
		newTexture = SDL_CreateTextureFromSurface(mRenderer, loadedSurface);
		if (newTexture == NULL)
		{
			std::stringstream ss;
			ss << "Unable to create texture from " + path + "! SDL_image Error:" << IMG_GetError();
			OutputDebugString(ss.str().c_str());
		}
		else
		{
			//Get image dimensions
			if (newWidth == 0) {
				mWidth = loadedSurface->w;
			}
			else
			{
				mWidth = newWidth;
			}
			if (newHeight == 0) {
				mHeight = loadedSurface->h;
			}
			else
			{
				mHeight = newHeight;
			}
		}

		//Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}

	//Return success
	mTexture = newTexture;
	return mTexture != NULL;
}

#ifdef _SDL_TTF_H
bool LTexture::loadFromRenderedText(std::string textureText, SDL_Color textColor, SDL_Renderer* mRenderer, TTF_Font* tFont)
{
	//Get rid of preexisting texture
	free();
	if(tFont == NULL)
		tFont = TTF_OpenFont("Resources/Fonts/airstrike.ttf", 16);//font could be unavailable
	if(tFont != NULL){
		//Render text surface
		SDL_Surface* textSurface = TTF_RenderText_Solid(tFont, textureText.c_str(), textColor);
		if (textSurface != NULL)
		{
			//Create texture from surface pixels
			mTexture = SDL_CreateTextureFromSurface(mRenderer, textSurface);
			if (mTexture == NULL)
			{
				std::stringstream ss;
				ss << "Unable to create texture from rendered text! SDL Error: "<< SDL_GetError();
				OutputDebugString(ss.str().c_str());
			}
			else
			{
				//Get image dimensions
				mWidth = textSurface->w;
				mHeight = textSurface->h;
			}

			//Get rid of old surface
			SDL_FreeSurface(textSurface);
		}
		else
		{
			std::stringstream ss;
			ss << "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError();
			OutputDebugString(ss.str().c_str());
		}
	}
	else
	{
		OutputDebugString("Unable to open font file! ( Resourses/Fonts/airstrike.ttf )");
	}
	//Return success
	return mTexture != NULL;
}
bool LTexture::createBlank(SDL_Renderer* gR,int width, int height, SDL_TextureAccess access)
{
	//Create uninitialized texture
	//mTexture = SDL_CreateTexture(gR, SDL_PIXELFORMAT_RGBA8888, access, width, height);
	mTexture = SDL_CreateTexture(gR, SDL_PIXELFORMAT_UNKNOWN, access, width, height);
	if (mTexture == NULL)
	{
		std::stringstream ss;
		ss << "Unable to create blank texture! SDL Error: %s\n", SDL_GetError();
		OutputDebugString(ss.str().c_str()); 
	}
	else
	{
		mWidth = width;
		mHeight = height;
	}

	return mTexture != NULL;
}
#endif

void LTexture::free()
{
	//Free texture if it exists
	if (mTexture != NULL)
	{
		SDL_DestroyTexture(mTexture);
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

void LTexture::setColor(Uint8 red, Uint8 green, Uint8 blue)
{
	//Modulate texture rgb
	SDL_SetTextureColorMod(mTexture, red, green, blue);
}

void LTexture::setBlendMode(SDL_BlendMode blending)
{
	//Set blending function
	SDL_SetTextureBlendMode(mTexture, blending);
}

void LTexture::setAlpha(Uint8 alpha)
{
	//Modulate texture alpha
	SDL_SetTextureAlphaMod(mTexture, alpha);
}

void LTexture::render( SDL_Renderer* mRenderer, SDL_Rect* dest, SDL_Rect* clip, SDL_Rect* tClips, double angle, SDL_Point* center, SDL_RendererFlip flip)
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = *dest;
	if (renderQuad.w == 0)
		renderQuad.w = mWidth;
	if (renderQuad.h == 0)
		renderQuad.h = mHeight;
	//Set clip rendering dimensions
	SDL_Rect new_clip;
	if(tClips != NULL) {
		double kx = mWidth / tClips->w;
		double ky = mHeight/ tClips->h;
		new_clip = {(int)(tClips->x*kx), (int)(tClips->y*ky),(int)kx,(int)ky};
		clip = &new_clip;
	}
	
	//Render to screen
	SDL_RenderCopyEx(mRenderer, mTexture, clip, &renderQuad, angle, center, flip);
}

void LTexture::setAsRenderTarget(SDL_Renderer* gR )
{
	SDL_SetRenderTarget(gR, mTexture);
}

int LTexture::getWidth()
{
	return mWidth;
}

int LTexture::getHeight()
{
	return mHeight;
}

void LTexture::setWidth(int w)
{
	mWidth = w;
}

void LTexture::setHeight(int h)
{
	mHeight = h;
}
