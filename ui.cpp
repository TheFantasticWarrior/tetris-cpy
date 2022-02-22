#include "tetrisboard.h"
#ifndef io
#define io
#include <iostream>
#endif // !io
#include <SDL.h>
#include "config.h"
#include "ui.h"

void color_from_rgb(uint32_t v) {
	SDL_SetRenderDrawColor(renderer, (v >> 16) & 0xFF, (v >> 8) & 0xFF, v & 0xFF, 0xFF);
}
void rgba_from_rgb(uint32_t v) {
	SDL_SetRenderDrawColor(renderer, (v >> 16) & 0xFF, (v >> 8) & 0xFF, v & 0xFF, 0x99);
}
int ghosty;
bool updated = true;
int leftdastimer=0;
int rightdastimer=0;
int softdroptimer = 0;
bool sd1 = 0;
static int timer=0;
bool left = 0;
bool right = 0;
bool softdrop = 0;
void update(game &g) {
		timer = SDL_GetTicks();
		if (left && timer - leftdastimer > DAS)
		{
			g.move(1, -1);
			updated = true;
		}
		if (right && timer - rightdastimer > DAS)
		{
			g.move(1, 1);
			updated = true;
		}
	
	if (softdrop) 
	{
		if (timer - softdroptimer > DROPDAS)
		{
			g.y += g.softdropdist();
			updated = true;
		}
		else if(sd1) {
			sd1 = 0;
			g.sd();
			updated = true;
		}
	}
	
}
void input(game &g)
{
	
	while (SDL_PollEvent(&event))
	{
		if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
			open = false;
		}
		if (event.key.repeat == 0 && event.type == SDL_KEYDOWN)
		{
			if (event.key.keysym.sym == HARDDROP)
			{
				updated = true;
				g.harddrop();
			}
			if (event.key.keysym.sym == HOLD)
			{
				updated = true;
				g.hold();
			}
			if (event.key.keysym.sym == SOFTDROP)
			{

				softdroptimer = SDL_GetTicks();
				sd1 = true;
				softdrop = true;
				updated = true;
			}
			if (event.key.keysym.sym == CW)
			{
				g.rotate(1);
				updated = true;
			}
			if (event.key.keysym.sym == CCW)
			{
				g.rotate(-1);
				updated = true;
			}
			if (event.key.keysym.sym == RT)
			{
				g.rotate(2);
				updated = true;

			}
			if (event.key.keysym.sym == LEFT)
			{
				g.move(0, -1);
				leftdastimer = SDL_GetTicks();
				left = 1;
				right = 0;
				updated = true;
			}
			if (event.key.keysym.sym == RIGHT)
			{
				g.move(0, 1);
				rightdastimer = SDL_GetTicks();
				right = 1;
				left = 0;
				updated = true;
			}
			if (event.key.keysym.sym == RESET)
			{
				g.reset();
				left = 0;
				right = 0;
				softdrop = 0;

				leftdastimer = 0;
				rightdastimer = 0;
				timer = 0;
				updated = true;
			}
			
		}
		if(event.type == SDL_KEYUP)
		{
			if (event.key.keysym.sym == LEFT)
			{
				leftdastimer = 0;
				left = 0;
				
			}
			if (event.key.keysym.sym == RIGHT)
			{
				rightdastimer = 0;
				right = 0;
			}
			if (event.key.keysym.sym == SOFTDROP)
			{
				softdrop = false;
			}
		}
		
	}
}

void draw(game &g) {
	color_from_rgb(0x666666);
	SDL_RenderFillRect(renderer,&bg);
	for (int_fast8_t i = 0; i < 10; i++)
	{
		for (int_fast8_t j = 0; j < 21; j++)
		{
			rect.x =1+ BOARDX+i * (PIXEL_SIZE + 1);
			rect.y = -PIXEL_SIZE/2+j * (PIXEL_SIZE + 1);
			color_from_rgb(colors[g.board[j+9][i] + 1]);
			SDL_RenderFillRect(renderer, &rect);
			
		}
	}
	// queue
	for (int_fast8_t n = 0; n < 5; n++)
	{

		color_from_rgb(colors[g.queue[n]+1]);
		for (int_fast8_t i = 0; i < 4; i++)
		{
			for (int_fast8_t j = 0; j < 4; j++)
			{
				rect.x = 4*BOARDX + i * (PIXEL_SIZE + 1);
				rect.y = PIXEL_SIZE*3*n+j * (PIXEL_SIZE + 1);
				if (g.piecedefs[g.queue[n]][0][j][i]!=-1)
				{
					SDL_RenderFillRect(renderer, &rect);
				}

			}
		}
	}
	//hold
	if (g.held != -1)
	{
		color_from_rgb(colors[g.held+1]);
		for (int_fast8_t i = 0; i < 4; i++)
		{
			for (int_fast8_t j = 0; j < 4; j++)
			{
				if (g.piecedefs[g.held][0][j][i] != -1){
					
					rect.x = 0 + i * (PIXEL_SIZE + 1);
					rect.y = j * (PIXEL_SIZE + 1);
					SDL_RenderFillRect(renderer, &rect);
				}

			}
		}
	}
	//active
	for (int_fast8_t i = 0; i < 4; i++)
	{
		for (int_fast8_t j = 0; j < 4; j++)
		{
			rect.x = BOARDX + (g.x + i) * (PIXEL_SIZE + 1);
			rect.y = PIXEL_SIZE/2+(g.y - 10 + j) * (PIXEL_SIZE + 1);
			if (g.piecedefs[g.active][g.rotation][j][i] != -1)
			{
				rgba_from_rgb(colors[g.piecedefs[g.active][g.rotation][j][i] + 1]);
				SDL_RenderFillRect(renderer, &rect);
			}

		}
	}
	//ghost
	if (ghost){
		ghosty= g.y+g.softdropdist();
		for (int_fast8_t i = 0; i < 4; i++)
		{
			for (int_fast8_t j = 0; j < 4; j++)
			{
				rect.x = BOARDX + (g.x + i) * (PIXEL_SIZE + 1);
				rect.y = PIXEL_SIZE/2+(ghosty - 10 + j) * (PIXEL_SIZE + 1);
				if (g.piecedefs[g.active][g.rotation][j][i] != -1)
				{
					rgba_from_rgb(colors[g.piecedefs[g.active][g.rotation][j][i] + 1]);
					SDL_RenderFillRect(renderer, &rect);
				}

			}
		}
	}
}

void render(game &g) {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	frameCount++;
	int timerFPS = SDL_GetTicks() - lastFrame;
	if (timerFPS < (8)) {
		SDL_Delay((8) - timerFPS);
	}
	draw(g);

	SDL_RenderPresent(renderer);
}

int main(int argv,char* argc[]) {
	game g;
	g.reset();
	int lastTime = 0;
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, 0, &window, &renderer);
	SDL_SetWindowTitle(window, "Tetris");
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	while (open) {
		lastFrame = SDL_GetTicks();
		if (lastFrame >= (lastTime + 1000)) {
			lastTime = lastFrame;
			fps = frameCount;
			frameCount = 0;
			//std::cout<<"FPS" << fps << std::endl;
		}
		input(g);
		update(g);
		if (updated){
			render(g);
			updated = false;
		}
		


	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
