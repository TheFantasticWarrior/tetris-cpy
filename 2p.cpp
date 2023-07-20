#define PY_SSIZE_T_CLEAN
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <Python.h>
#include <numpy/arrayobject.h>

#ifndef tb
#define tb
#include "tetrisboard.h"
#endif // !tb


#ifndef io
#define io
#include <iostream>
#endif // !io
#include <SDL.h>
#include <tuple>
#include "config.h"


int block_size;
int BOARDX;
bool window_open = true;
int lastTime;
SDL_Rect bg;
SDL_Rect rect;
SDL_Event event;
SDL_Window* window;
SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
class game_server {
public:
	int stored_attack;
	void send(int port, int attack) {
		stored_attack += (port == 1 ? 1 : -1) * attack;

	}
	int recieve(int port) {
		//std::cout << port << " ";
		if (port == 1 && stored_attack < 0) {
			int atk = -stored_attack;
			stored_attack = 0;
			return atk;
		}
		if (port == 2 && stored_attack > 0)
		{
			int atk = stored_attack;
			stored_attack = 0;
			return atk;
		}
		return 0;
	}
};
class game_client : public game {
public:
	game_server *server;
	int port;
	game_client(game_server &s, int p) {
		server = &s;
		port = p;
	}
	void harddrop() {
		game::harddrop();

		if (combo)
			server->send(port,attack);
		else recieve(server->recieve(port));
		new_piece();

	}
};

int ghosty;
bool updated = true;
game_server server;
game_client g(server,1);
game_client g2(server,2);

//modify board with timed input
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
			g.softdrop();
			updated = true;
		}
		else if(sd1) {
			sd1 = 0;
			g.sd();
			updated = true;
		}
	}
	
}

//get input
void input(game &g)
{
	
	while (SDL_PollEvent(&event))
	{
		if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
			window_open = false;
		}
		if (event.key.repeat == 0 && event.type == SDL_KEYDOWN)
		{
			int a = int(event.key.keysym.sym);
			if (a == HARDDROP)
			{
				updated = true;
				g.harddrop();
			}
			if (a == HOLD)
			{
				updated = true;
				g.hold();
			}
			if (a == SOFTDROP)
			{

				softdroptimer = SDL_GetTicks();
				sd1 = true;
				softdrop = true;
				updated = true;
			}
			if (a == CW)
			{
				g.rotate(1);
				updated = true;
			}
			if (a == CCW)
			{
				g.rotate(-1);
				updated = true;
			}
			if (a == RT)
			{
				g.rotate(2);
				updated = true;

			}
			if (a == LEFT)
			{
				g.move(0, -1);
				leftdastimer = SDL_GetTicks();
				left = 1;
				right = 0;
				updated = true;
			}
			if (a == RIGHT)
			{
				g.move(0, 1);
				rightdastimer = SDL_GetTicks();
				right = 1;
				left = 0;
				updated = true;
			}
			if (a == RESET)
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
			int a = int(event.key.keysym.sym);
			if (a == LEFT)
			{
				leftdastimer = 0;
				left = 0;
				
			}
			if (a == RIGHT)
			{
				rightdastimer = 0;
				right = 0;
			}
			if (a == SOFTDROP)
			{
				softdrop = false;
			}
		}
		
	}
}


int colors[9]{ //bg SZJLTOI garbage
	0x000000,
	0x59b101,
	0xd70f37,
	0x2141c6,
	0xe35b02,
	0xaf298a,
	0xe39f02,
	0x0f9bd7,
	0x777777
};
void color_from_rgb(uint32_t v) {
	SDL_SetRenderDrawColor(renderer, (v >> 16) & 0xFF, (v >> 8) & 0xFF, v & 0xFF, 0xFF);
}
void rgba_from_rgb(uint32_t v) {
	SDL_SetRenderDrawColor(renderer, (v >> 16) & 0xFF, (v >> 8) & 0xFF, v & 0xFF, 0x99);
}
void draw(game &g,int xloc) {
	color_from_rgb(0x666666);
	bg.x += xloc;
	SDL_RenderFillRect(renderer,&bg);
	for (int_fast8_t i = 0; i < 10; i++)
	{
		for (int_fast8_t j = 0; j < 21; j++)
		{
			rect.x =1+ BOARDX+i * (block_size + 1)+xloc;
			rect.y = -block_size/2+j * (block_size + 1);
			//std::cout << g.board[j + 9][i] + 1 << " " << colors[g.board[j + 9][i] + 1]<<" ";
			color_from_rgb(colors[g.board[j+9][i] + 1]);
			SDL_RenderFillRect(renderer, &rect);
			
		}
		//std::cout << "\n";
	}
	// queue
	for (int_fast8_t n = 0; n < 5; n++)
	{

		color_from_rgb(colors[g.queue[n]+1]);
		for (int_fast8_t i = 0; i < 4; i++)
		{
			for (int_fast8_t j = 0; j < 4; j++)
			{
				rect.x = 4*BOARDX + i * (block_size + 1) + xloc;
				rect.y = block_size*3*n+j * (block_size + 1);
				if (g.piecedefs[g.queue[n]][0][j][i]!=-1)
				{
					SDL_RenderFillRect(renderer, &rect);
				}

			}
		}
	}
	//hold
	if (g.held_piece != -1)
	{
		color_from_rgb(colors[g.held_piece+1]);
		for (int_fast8_t i = 0; i < 4; i++)
		{
			for (int_fast8_t j = 0; j < 4; j++)
			{
				if (g.piecedefs[g.held_piece][0][j][i] != -1){
					
					rect.x = 0 + i * (block_size + 1) + xloc;
					rect.y = j * (block_size + 1);
					SDL_RenderFillRect(renderer, &rect);
				}

			}
		}
	}
	//active
	if (active_piece)
	{
	for (int_fast8_t i = 0; i < 4; i++)
	{
		for (int_fast8_t j = 0; j < 4; j++)
		{
			rect.x = BOARDX + (g.x + i) * (block_size + 1) + xloc;
			rect.y = block_size/2+(g.y - 10 + j) * (block_size + 1);
			if (g.piecedefs[g.active][g.rotation][j][i] != -1)
			{
				rgba_from_rgb(colors[g.piecedefs[g.active][g.rotation][j][i] + 1]);
				SDL_RenderFillRect(renderer, &rect);
			}

			}
		}
	}
	else
	{
		for (int_fast8_t i = 0; i < 4; i++)
		{
			for (int_fast8_t j = 0; j < 4; j++)
			{
				rect.x = BOARDX + (3 + i) * (block_size + 1) + xloc;
				rect.y = block_size / 2 + (-1 + j) * (block_size + 1);
				if (g.piecedefs[g.active][0][j][i] != -1)
				{
					rgba_from_rgb(colors[g.piecedefs[g.active][0][j][i] + 1]);
					SDL_RenderFillRect(renderer, &rect);
				}

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
				rect.x = BOARDX + (g.x + i) * (block_size + 1) + xloc;
				rect.y = block_size/2+(ghosty - 10 + j) * (block_size + 1);
				if (g.piecedefs[g.active][g.rotation][j][i] != -1)
				{
					rgba_from_rgb(colors[g.piecedefs[g.active][g.rotation][j][i] + 1]);
					SDL_RenderFillRect(renderer, &rect);
				}

			}
		}
	}
}


int frameCount, lastFrame, fps;
int exited = 0;
void c_render(game &g,game &g2) {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	bg.x = BOARDX;
	/*
	frameCount++;
	int timerFPS = SDL_GetTicks() - lastFrame;
	if (timerFPS < (8)) {
		SDL_Delay((8) - timerFPS);
	}
	*/
	draw(g,0);
	draw(g2, BOARDX * 5);
	SDL_RenderPresent(renderer);
}


void cinit(int pwh,int val) {

	g.reset();
	g2.reset();
	if (pwh != 0) {
		switch (pwh)
		{
		case 1:
			block_size = val;
			break;
		case 2:
			block_size = (val - 2) * 3 / 100 - 1;
		case 3:
			block_size = (val - 32) / 20.5f - 1;
			break;
		default:
			block_size = 40;
			break;
		}

		int width = (block_size + 1) * 100 / 3 + 2;
		int height = 20.5f * (block_size + 1);
		//std::cout << width << "\n";
		BOARDX = width / 10;
		bg.x = BOARDX; bg.y = 0; bg.w = width * 3 / 10; bg.h = height;
		rect.x = block_size; rect.y = block_size; rect.w = block_size; rect.h = block_size;
		lastTime = 0;
		SDL_Init(SDL_INIT_EVERYTHING);
		SDL_CreateWindowAndRenderer(width, height, 0, &window, &renderer);
		SDL_SetWindowTitle(window, "Tetris");
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
	}
}
PyObject* seed(PyObject*, PyObject* args) {
	int x=0;
	if (!PyArg_ParseTuple(args, "|i", &x))
		return NULL;
	g.set_seed(x);
	g2.set_seed(x);
	Py_RETURN_NONE;
}
PyObject* reset(PyObject* self, PyObject* args) {
	int pwh, val;
	if (!PyArg_ParseTuple(args, "i|i", &pwh, &val))
		return NULL;
	cinit(pwh, val);
	const npy_intp dim = 1476;
	const npy_intp* dims = &dim;
	int8_t* state = (int8_t*)malloc(sizeof(int8_t) * 1476);
	state[0] = g.cleared;
	state[1] = g.x+2;
	state[2] = g.y;
	state[3] = g.rotation;
	state[4] = g.hold_used;
	state[5] = g.active+1;
	state[6] = g.held_piece+1;
	for (size_t i = 0; i < 5; i++)
	{
		state[i + 7] = g.queue[i]+1;
	}
	for (size_t i = 0; i < 210; i++)
	{
		state[i + 12] = g.board[9+i % 21][ i / 21]+1;
		//std::cout << state[i + 12] << " ";
	}
	//active
	for (size_t i = 0; i < 210; i++)
	{
		state[i + 222] = ((9 + i % 21) > g.y) && ((4 + i % 21) < g.y) && ((i / 21) > g.x) && (g.x + 5 > (i / 21)) ? (g.piecedefs[g.active][g.rotation][(8 + i % 21) - g.y][((i / 21) - 1 - g.x)] + 1) > 0:0;
	}
	//shadow
	for (size_t i = 0; i < 210; i++)
	{
		state[i + 432] = ((10 + i % 21) > (g.y + g.softdropdist())) && ((5 + i % 21) < (g.y + g.softdropdist())) && ((i / 21) > g.x) && (g.x + 5 > (i / 21)) ? (g.piecedefs[g.active][g.rotation][(9 + i % 21) - (g.y + g.softdropdist())][((i / 21) - 1 - g.x)] + 1) > 0:0;
	}
	for (size_t j = 0; j < 4; j++)
	{
		for (size_t k = 0; k < 4; k++)
		{
			state[j * 4 + 642 + k] = g.held_piece != -1 ? ((g.piecedefs[g.held_piece][0][k][j] + 1) > 0) : 0;
		}
	}

	for (size_t i = 0; i < 5; i++)
	{
		for (size_t j = 0; j < 4; j++)
		{
			for (size_t k = 0; k < 4; k++)
			{
				state[i * 16 + j * 4 + 658 + k] = ((g.piecedefs[g.queue[i]][0][k][j] + 1) > 0);
			}
		}
	}
	state[738+0] = g2.cleared;
	state[738+1] = g2.x + 2;
	state[738+2] = g2.y;
	state[738+3] = g2.rotation;
	state[738+4] = g2.hold_used;
	state[738+5] = g2.active + 1;
	state[738+6] = g2.held_piece + 1;
	for (size_t i = 0; i < 5; i++)
	{
		state[738+i + 7] = g2.queue[i] + 1;
	}
	for (size_t i = 0; i < 210; i++)
	{
		state[738+i + 12] = g2.board[9 + i % 21][i / 21] + 1;
		//std::cout << state[738+i + 12] << " ";
	}
	//active
	for (size_t i = 0; i < 210; i++)
	{
		state[738+i + 222] = ((9 + i % 21) > g2.y) && ((4 + i % 21) < g2.y) && ((i / 21) > g2.x) && (g2.x + 5 > (i / 21)) ? (g2.piecedefs[g2.active][g2.rotation][(8 + i % 21) - g2.y][((i / 21) - 1 - g2.x)] + 1) > 0:0;
	}
	//shadow
	for (size_t i = 0; i < 210; i++)
	{
		state[738+i + 432] = ((10 + i % 21) > (g2.y + g2.softdropdist())) && ((5 + i % 21) < (g2.y + g2.softdropdist())) && ((i / 21) > g2.x) && (g2.x + 5 > (i / 21)) ? (g2.piecedefs[g2.active][g2.rotation][(9 + i % 21) - (g2.y + g2.softdropdist())][((i / 21) - 1 - g2.x)] + 1) > 0:0;
	}
	for (size_t j = 0; j < 4; j++)
	{
		for (size_t k = 0; k < 4; k++)
		{
			state[738+j * 4 + 642 + k] = g2.held_piece != -1 ? ((g2.piecedefs[g2.held_piece][0][k][j] + 1) > 0) : 0;
		}
	}

	for (size_t i = 0; i < 5; i++)
	{
		for (size_t j = 0; j < 4; j++)
		{
			for (size_t k = 0; k < 4; k++)
			{
				state[738+i * 16 + j * 4 + 658 + k] = ((g2.piecedefs[g2.queue[i]][0][k][j] + 1) > 0);
			}
		}
	}
	PyObject* ret = PyArray_SimpleNewFromData(1, dims, NPY_UINT8, state);
	PyArray_ENABLEFLAGS((PyArrayObject*)ret, NPY_ARRAY_OWNDATA);
	//free(state);
	return ret;
}
PyObject* step(PyObject* ,PyObject *args) {
	int x,y;
	if (!PyArg_ParseTuple(args, "ii", &x,&y))
		return NULL;
	int reward = 0;
	int reward2 = 0;
	switch (x)
	{
	case 0:
		if (g.hold_used) {
			reward = 254;
		}
		else
		{
			g.hold();
		}
		break;
	case 1:
		g.harddrop();
		break;
	case 2:
		g.rotate(1);
		break;
	case 3:
		g.rotate(-1);
		break;
	case 4:
		x = g.x;
		g.move(0, -1);
		if (x == g.x) {
			reward = 254;
		}
		break;
	case 5:
		x = g.x;
		g.move(0, 1);
		if (x == g.x) {
			reward = 254;
		}
		break;
	case 6:
		x = g.x;
		g.move(1, -1);
		if (x == g.x) {
			reward = 254;
		}
		break;
	case 7:
		x = g.x;
		g.move(1, 1);
		if (x == g.x) {
			reward = 254;
		}
		break;
	case 8:
		if (g.softdropdist() > 0)
			g.softdrop();
		else reward = 254;
		break;
	case 9:
		g.rotate(2);
		break;
	default:
		break;
	}

	switch (y)
	{
	case 0:
		if (g2.hold_used) {
			reward2 = 254;
		}
		else
		{
			g2.hold();
		}
		break;
	case 1:
		g2.harddrop();
		break;
	case 2:
		g2.rotate(1);
		break;
	case 3:
		g2.rotate(-1);
		break;
	case 4:
		x = g2.x;
		g2.move(0, -1);
		if (x == g2.x) {
			reward2 = 254;
		}
		break;
	case 5:
		x = g2.x;
		g2.move(0, 1);
		if (x == g2.x) {
			reward2 = 254;
		}
		break;
	case 6:
		x = g2.x;
		g2.move(1, -1);
		if (x == g2.x) {
			reward2 = 254;
		}
		break;
	case 7:
		x = g2.x;
		g2.move(1, 1);
		if (x == g2.x) {
			reward2 = 254;
		}
		break;
	case 8:
		if (g2.softdropdist() > 0)
			g2.softdrop();
		else reward2 = 254;
		break;
	case 9:
		g2.rotate(2);
		break;
	default:
		break;
	}
	const npy_intp dim = 1476;
	const npy_intp* dims = &dim;
	int8_t* state = (int8_t*)malloc(sizeof(int8_t) * dim);
	if (g.game_over){
		state[0] = 255;
		reward = 253;
	}
	else if (reward) {
		state[0] = reward;
	}else {
		state[0] = g.cleared;
	}

	state[1] = g.x+2;
	state[2] = g.y;
	state[3] = g.rotation;
	state[4] = g.hold_used;
	state[5] = g.active+1;
	state[6] = g.held_piece+1;
	for (size_t i = 0; i < 5; i++)
	{
		state[i + 7] = g.queue[i]+1;
	}
	for (size_t i = 0; i < 210; i++)
	{
		state[i + 12] = (g.board[9 + i % 21][i / 21] + 1) > 0;
	}
	//active
	for (size_t i = 0; i < 210; i++)
	{
		state[i + 222] = ((9 + i % 21) > g.y) && ((4 + i % 21) < g.y) && ((i / 21) > g.x) && (g.x + 5 > (i / 21))?(g.piecedefs[g.active][g.rotation][(8 + i % 21) - g.y][((i / 21)-1 - g.x)] + 1) > 0:0;
	}
	//shadow
	for (size_t i = 0; i < 210; i++)
	{
		state[i + 432] = ((10 + i % 21) > (g.y + g.softdropdist())) && ((5 + i % 21) < (g.y + g.softdropdist())) && ((i / 21) > g.x) && (g.x + 5 > (i / 21)) ? (g.piecedefs[g.active][g.rotation][(9 + i % 21) - (g.y+g.softdropdist())][((i / 21) - 1 - g.x)] + 1) > 0:0;
	}
		for (size_t j = 0; j < 4; j++)
		{
			for (size_t k = 0; k < 4; k++)
			{
				state[j * 4 + 642 + k] = g.held_piece!=-1?((g.piecedefs[g.held_piece][0][k][j] + 1) > 0):0;
			}
		}
	
	for (size_t i = 0; i < 5; i++)
	{
		for (size_t j = 0; j < 4; j++)
		{
			for (size_t k = 0; k < 4; k++)
			{
				state[i*16+j*4+658+k] = ((g.piecedefs[g.queue[i]][0][k][j] + 1) > 0);
			}
		}
	}
	if (g2.game_over) {
		state[738+0] = 255;
		state[0] = 253;
	}else if (reward2) {
		state[738] = reward2;
	}else {
		state[738+0] = g2.cleared;
	}

	state[738+1] = g2.x + 2;
	state[738+2] = g2.y;
	state[738+3] = g2.rotation;
	state[738+4] = g2.hold_used;
	state[738+5] = g2.active + 1;
	state[738+6] = g2.held_piece + 1;
	for (size_t i = 0; i < 5; i++)
	{
		state[738+i + 7] = g2.queue[i] + 1;
	}
	for (size_t i = 0; i < 210; i++)
	{
		state[738+i + 12] = (g2.board[9 + i % 21][i / 21] + 1) > 0;
	}
	//active
	for (size_t i = 0; i < 210; i++)
	{
		state[738+i + 222] = ((9 + i % 21) > g2.y) && ((4 + i % 21) < g2.y) && ((i / 21) > g2.x) && (g2.x + 5 > (i / 21)) ? (g2.piecedefs[g2.active][g2.rotation][(8 + i % 21) - g2.y][((i / 21) - 1 - g2.x)] + 1) > 0:0;
	}
	//shadow
	for (size_t i = 0; i < 210; i++)
	{
		state[738+i + 432] = ((10 + i % 21) > (g2.y + g2.softdropdist())) && ((5 + i % 21) < (g2.y + g2.softdropdist())) && ((i / 21) > g2.x) && (g2.x + 5 > (i / 21)) ? (g2.piecedefs[g2.active][g2.rotation][(9 + i % 21) - (g2.y + g2.softdropdist())][((i / 21) - 1 - g2.x)] + 1) > 0:0;
	}
	for (size_t j = 0; j < 4; j++)
	{
		for (size_t k = 0; k < 4; k++)
		{
			state[738+j * 4 + 642 + k] = g2.held_piece != -1 ? ((g2.piecedefs[g2.held_piece][0][k][j] + 1) > 0) : 0;
		}
	}

	for (size_t i = 0; i < 5; i++)
	{
		for (size_t j = 0; j < 4; j++)
		{
			for (size_t k = 0; k < 4; k++)
			{
				state[738+i * 16 + j * 4 + 658 + k] = ((g2.piecedefs[g2.queue[i]][0][k][j] + 1) > 0);
			}
		}
	}
	PyObject* ret=PyArray_SimpleNewFromData(1, dims, NPY_UINT8, state);
	PyArray_ENABLEFLAGS((PyArrayObject*)ret, NPY_ARRAY_OWNDATA);
	return ret;
	
	
}
PyObject* close(PyObject* self, PyObject* Py_UNUSED) {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	Py_RETURN_NONE;
}
PyObject* render(PyObject* self, PyObject* Py_UNUSED) {
	if(!exited)
	{
		c_render(g, g2);
		SDL_PollEvent(&event);
		if (event.type == SDL_QUIT) {
			exited = 1;
			SDL_DestroyRenderer(renderer);
			SDL_DestroyWindow(window);
		}
	}
	
	Py_RETURN_NONE;
}


static PyMethodDef Methods[] = {

	 {"seed",  seed, METH_VARARGS,
	 "seed the rng"},
	{"reset",  reset, METH_VARARGS,
	 "Reset everything"},
	 {"step",  step, METH_VARARGS,
	 "Take action and return reward&state"},
	 {"close",  close, METH_NOARGS,
	 "close everything"},
	 {"render",  render, METH_NOARGS,
	 "Render window, must be called often so window doesn't freeze"},
	{NULL, NULL, 0, NULL}
};
static struct PyModuleDef Module = {
	PyModuleDef_HEAD_INIT,
	"env2p",   /* name of module */
	NULL,
	-1,
	Methods
};
PyMODINIT_FUNC
PyInit_env2p(void)
{

	import_array();
	return PyModule_Create(&Module);
}
int main(int argc, char* argv[]) {
	
	return 0;
}