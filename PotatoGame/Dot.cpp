#include "Dot.h"

int Dot::DOT_WIDTH = (int) (BlockBox::BOX_SIZE*1.5);
int Dot::DOT_HEIGHT = Dot::DOT_WIDTH;
double Dot::LEVEL_GRAVITY = BlockBox::BOX_SIZE*0.05;
int Dot::LEVEL_JUMP_POWER = (int)(BlockBox::BOX_SIZE*0.5);
double Dot::DOT_VEL = (int)(BlockBox::BOX_SIZE*0.1);


Dot::Dot(std::vector<Mix_Chunk*>*s ,std::vector<BlockBox*>* v, LTexture* t, SDL_Renderer* gR, int mapW, int mapH)
{	
	lBlocks = v;
	lSounds = s;
	dTexture = t;
	gRenderer = gR;
	
	//Initialize the offsets
	mPosX = 5;
	mPosY = 5;
	//mPosY = LEVEL_HEIGHT-DOT_HEIGHT-50;
	mCoins = 0;
	//Initialize the velocity
	mVelX = DOT_VEL;
	mVelY = 0;
	State = States::DOT_ALIVE;
	
	tFlip = SDL_FLIP_NONE;

	mapWidth = mapW;
	mapHeight = mapH;
}

Dot::~Dot()
{
	dTexture->free();
}

void Dot::handleEvent(SDL_Event& e)
{
	//If a key was pressed
	//if (e.type == SDL_KEYDOWN && !e.key.repeat)
	if (e.type == SDL_KEYDOWN)
	{
		//Adjust the velocity
		switch (e.key.keysym.sym)
		{
			//case SDLK_UP:  DOT_HEIGHT += 1; DOT_WIDTH += 1; mPosY -= 1; gDotTexture.setWidth(DOT_WIDTH); gDotTexture.setHeight(DOT_HEIGHT);  break;
			//case SDLK_DOWN:  DOT_HEIGHT -= 1; DOT_WIDTH -= 1; mPosY += 1; gDotTexture.setWidth(DOT_WIDTH); gDotTexture.setHeight(DOT_HEIGHT);  break;
		case SDLK_UP: break;
		case SDLK_DOWN: break;
		case SDLK_LEFT: break;
		case SDLK_RIGHT: break;
		case SDLK_SPACE:
			if (_grounded) {
				mVelY = - LEVEL_JUMP_POWER;
				_grounded = false;
				Mix_PlayChannel(-1, lSounds->at(Sounds::SE_JUMP), 0);
			}
			break;
		}
	}
}

void Dot::move()
{
	if(State == States::DOT_ALIVE){
		//Move the dot right
		mPosX += (int)mVelX;
	
		//Check collision with left side of platform - what to do?
		int sz = lBlocks->size();
		bool collides_x = false;
		for (int i = 0; i < sz; i++) {
			SDL_Rect r = lBlocks->at(i)->getRect();
			if (check_collision(r))
			{
				int type = lBlocks->at(i)->getType();
				if (type == BlockBox::BL_GROUND)
				{
					mPosX -= (int)mVelX;
					mVelX = 0;
					collides_x = true;
				}
				else if (type == BlockBox::BL_OBSTACLE)
				{
					State = States::DOT_DIE;
					break;
				}
			}
			else
			{
				mVelX = DOT_VEL;
			}
		}

		if (mPosX < 0)
		{
			mPosX -= (int)mVelX;
			DOT_VEL = -DOT_VEL + 0.2;
			mVelX = DOT_VEL;
			tFlip = SDL_FLIP_NONE;
		}
		//If the dot went too far to the right
		if (mPosX + DOT_WIDTH > mapWidth)
		{
			//Move back
			mPosX -= (int)mVelX;
			DOT_VEL = -DOT_VEL - 0.2;
			mVelX = DOT_VEL;
			tFlip = SDL_FLIP_HORIZONTAL;
		}
	
		
		bool collides_y = false;
		//Move the dot up or down (jump, fall)	
		mVelY += 0.5*LEVEL_GRAVITY;
		mPosY += (int)mVelY;
		int exits = 0;//checking if dot exits level
		for (int i = 0; i < sz; i++) {
			BlockBox* b = lBlocks->at(i);
			SDL_Rect r = b->getRect();
			if (check_collision(r)) {
				int type = b->getType();
				if (type == BlockBox::BL_GROUND)
				{
					collides_y = true;
					mPosY -= (int)mVelY;
					if (mPosY > r.y)
						mPosY = r.y + r.h;
					else
						mPosY = r.y - DOT_HEIGHT;
					mVelY = 1;
					if (r.y > mPosY)
						_grounded = true;
					//break;//???
				}
				else if (type == BlockBox::BL_OBSTACLE)
				{
					if (State == States::DOT_ALIVE) {
						if (mPosY > r.y)
							mPosY = r.y + r.h;
						else
							mPosY = r.y - DOT_HEIGHT;
						Mix_PlayChannel(-1, lSounds->at(Sounds::SE_DEATH), 0);
					}
					mVelY = 0;
					_grounded = false;
					State = States::DOT_DIE;
					
					break;//!!!
				}
				else if (type == BlockBox::BL_COLLECTABLE)
				{
					if (!b->is_collected()) {
						b->collect();
						mCoins++;
						Mix_PlayChannel(-1, lSounds->at(Sounds::SE_COIN), 0);
					}
				}
				else if (type == BlockBox::BL_DOOR && BlockBox::DOOR_OPEN == 1)
				{
					exits++;
					if (exits == 4) {
						State = States::DOT_WON;
						stop();
					}
				}
			}
		}
		if( !collides_y && _grounded )
			_grounded = false;
	
		//If the dot went too far up
		if ((mPosY < 0))
		{
			mPosY -= (int)mVelY;
			mPosY = 0;
			mVelY = 0;
		}

		if (State == States::DOT_DIE) {
			stop();
			mVelY = -8;
		}
	}
	else if (State == States::DOT_DIE) {
		mVelY += 0.5*LEVEL_GRAVITY;
		mPosY += (int)mVelY;
	} 
	if ((mPosY > mapHeight + DOT_HEIGHT)) {
		State = States::DOT_DEAD;
	}
}

bool Dot::check_collision(SDL_Rect B) {
	int leftA, leftB;
	int rightA, rightB;
	int topA, topB;
	int bottomA, bottomB;

	leftA = mPosX;
	rightA = mPosX + DOT_WIDTH;
	topA = mPosY;
	bottomA = mPosY + DOT_HEIGHT;

	leftB = B.x;
	rightB = B.x + B.w;
	topB = B.y;
	bottomB = B.y + B.h;

	if (leftA >= rightB) { return false; }
	if (rightA <= leftB) { return false; }
	if (bottomA <= topB) { return false; }
	if (topA >= bottomB) { return false; }
	return true;
}

void Dot::stop() {
	mVelX = 0;
	mVelY = 0;
}

int Dot::fail_anim()
{
	if (mPosY > mapHeight)
		return 1;
	mVelY += 0.5*LEVEL_GRAVITY;
	mPosY += (int)mVelY;
	return 0;
}

int Dot::getState()
{
	return State;
}

LTexture * Dot::getTexture()
{
	return dTexture;
}


void Dot::render(int camX, int camY)
{
	//Show the dot relative to the camera
	SDL_Rect dest = { mPosX - camX ,mPosY - camY,DOT_WIDTH,DOT_HEIGHT };
	//SDL_SetRenderDrawColor(gRenderer, 255, 0, 0, 0);
	//SDL_RenderDrawRect(gRenderer, &dest);
	dTexture->render(gRenderer, &dest, NULL,NULL,0.0,0,tFlip);
}

int Dot::getPosX()
{
	return mPosX;
}

int Dot::getPosY()
{
	return mPosY;
}

int Dot::getCoinCountInt() {
	return mCoins;
}

std::string Dot::getCoinCountStr()
{
	std::stringstream sstm;
	sstm << mCoins;
	std::string sCoins = sstm.str();
	return sCoins;
}
