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
#include <vector>

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
	std::vector<int> attack_queue;
	game_server() {}
	game_server(const game_server& other) {
		stored_attack = other.stored_attack;
		attack_queue.assign(other.attack_queue.begin(), other.attack_queue.end());

	}
	void send(int port, int attack) {
		int side = (port == 1 ? 1 : -1);
		if (side * attack * stored_attack > 0) {
			attack_queue.push_back(attack);
		}
		else
		{
			int new_atk = attack;
			if (stored_attack > attack) {
				while (new_atk >0)
				{
					new_atk -= attack_queue[0];
					if (new_atk<0)
					{
						break;
					}
					attack_queue.erase(attack_queue.begin());
				}
				attack_queue[0] = -new_atk;
			}
			else
			{
				while (!attack_queue.empty())
				{
					new_atk -= attack_queue[0];
					attack_queue.erase(attack_queue.begin());
				}
				attack_queue.push_back(new_atk);
			}
			
		}

		stored_attack += side * attack;
	}
	std::vector<int> recieve(int port) {
		//std::cout << port << " ";
		if (port == 1 && stored_attack < 0) {
			stored_attack = 0;
			return attack_queue;
		}
		if (port == 2 && stored_attack > 0)
		{
			stored_attack = 0;
			return attack_queue;
		}
		return {};
	}
};
class game_client : public game {
public:
	game_server *server;
	int port;
	int action_count=0;
	game_client(game_server &s, int p) {
		server = &s;
		port = p;
	}
	void reset() {
		game::reset();
		game::random_recv(3);
	}
	void harddrop() {
		game::harddrop();
		action_count = 0;

		if (combo)
			server->send(port,attack);
		else recieve(server->recieve(port));
		new_piece();

	}
};

int ghosty;
bool updated = true;
game_server server;
game_server servercp;
game_client g1(server,1);
game_client g2(server,2);
game_client g1cp(servercp, 1);;
game_client g2cp(servercp, 2);;
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
void c_render(game &g1,game &g2) {
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
	draw(g1,0);
	draw(g2, BOARDX * 5);
	SDL_RenderPresent(renderer);
}


void cinit(int pwh,int val) {
	g1.reset();
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
	g1.set_seed(x);
	g2.set_seed(x);
	Py_RETURN_NONE;
}
PyObject* reset(PyObject* self, PyObject* args) {
	int pwh, val;
	if (!PyArg_ParseTuple(args, "i|i", &pwh, &val))
		return NULL;
	cinit(pwh, val);

	server.stored_attack = 0;
	g1.action_count = 0;
	g2.action_count = 0;
	const npy_intp dim = 1477;
	const npy_intp* dims = &dim;
	int8_t* state = (int8_t*)malloc(sizeof(int8_t) * dim);
	state[1476] = server.stored_attack;
	state[0] = g1.cleared;
	state[1] = g1.action_count;//g1.x+2;

	state[2] = g1.gheight;//g1.y;
	state[3] = g1.hold_used;
	state[4] = g1.rotation;
	state[5] = g1.active+1;
	state[6] = g1.held_piece+1;
	for (size_t i = 0; i < 5; i++)
	{
		state[i + 7] = g1.queue[i]+1;
	}
	for (size_t i = 0; i < 210; i++)
	{
		state[i + 12] = g1.board[9+i % 21][ i / 21]+1>0;
		//std::cout << state[i + 12] << " ";
	}
	//active
	for (int i = 0; i < 210; i++)
	{
		state[i + 222] = ((9 + i % 21) >= g1.y) && ((6 + i % 21) <= g1.y) && ((i / 21) - g1.x >= 0) && (g1.x + 3 -(i / 21)>= 0) ? (g1.piecedefs[g1.active][g1.rotation][(9 + i % 21) - g1.y][((i / 21) - g1.x)] + 1) > 0: 0;
	}
	//shadow
	int ny = g1.y + g1.softdropdist();
	for (int i = 0; i < 210; i++)
	{
		state[i + 432] = ((9 + i % 21) >= (ny)) && ((6 + i % 21) <= (ny)) && ((i / 21) - g1.x >= 0) && (g1.x + 3 -(i / 21)>= 0) ? (g1.piecedefs[g1.active][g1.rotation][(9 + i % 21) - (ny)][((i / 21) - g1.x)] + 1) > 0:0;
	}
	for (size_t j = 0; j < 4; j++)
	{
		for (size_t k = 0; k < 4; k++)
		{
			state[j * 4 + 642 + k] = g1.held_piece != -1 ? ((g1.piecedefs[g1.held_piece][0][k][j] + 1) > 0) : 0;
		}
	}

	for (size_t i = 0; i < 5; i++)
	{
		for (size_t j = 0; j < 4; j++)
		{
			for (size_t k = 0; k < 4; k++)
			{
				state[i * 16 + j * 4 + 658 + k] = ((g1.piecedefs[g1.queue[i]][0][k][j] + 1) > 0);
			}
		}
	}
	state[738+0] = g2.cleared;
	state[738+1] = g2.action_count;//g2.x+2;
	state[738+2] = g2.gheight;//g2.y;
	state[738+3] = g2.hold_used;
	state[738+4] = g2.rotation;
	state[738+5] = g2.active + 1;
	state[738+6] = g2.held_piece + 1;
	for (size_t i = 0; i < 5; i++)
	{
		state[738+i + 7] = g2.queue[i] + 1;
	}
	for (size_t i = 0; i < 210; i++)
	{
		state[738+i + 12] = g2.board[9 + i % 21][i / 21] + 1>0;
		//std::cout << state[738+i + 12] << " ";
	}
	//active
	for (int i = 0; i < 210; i++)
	{
		state[738 + i + 222] = ((9 + i % 21) >= g2.y) && ((6 + i % 21) <= g2.y) && ((i / 21) - g2.x >= 0) && (g2.x + 3 - (i / 21) >= 0) ? (g2.piecedefs[g2.active][g2.rotation][(9 + i % 21) - g2.y][((i / 21) - g2.x)] + 1) > 0: 0;
	}
	//shadow
	ny = g2.y + g2.softdropdist();
	for (int i = 0; i < 210; i++)
	{
		state[738 + i + 432] = ((9 + i % 21) >= (ny)) && ((6 + i % 21) <= (ny)) && ((i / 21) - g2.x >= 0) && (g2.x + 3 - (i / 21) >= 0) ? (g2.piecedefs[g2.active][g2.rotation][(9 + i % 21) - (ny)][((i / 21) - g2.x)] + 1) > 0:0;
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
	PyObject* ret = PyArray_SimpleNewFromData(1, dims, NPY_INT8, state);
	PyArray_ENABLEFLAGS((PyArrayObject*)ret, NPY_ARRAY_OWNDATA);
	//free(state);
	return ret;
}
int game_step(game_client &g, int action) {
	g.action_count++;
	if (g.action_count == 10)
	{
		g.harddrop();
		return 1;
	}
	else {
		switch (action)
		{
		case 0:
			g.hold();
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
			//x = g.x;
			g.move(0, -1);
			break;
		case 5:
			//x = g.x;
			g.move(0, 1);
			break;
		case 6:
			//x = g.x;
			g.move(1, -1);
			break;
		case 7:
			//x = g.x;
			g.move(1, 1);
			break;
		case 8:
			if (g.softdropdist() > 0)
				g.softdrop();
			break;
		case 9:
			g.rotate(2);
			break;
		default:
			break;
		}
	}
	return 0;
}
PyObject* copy(PyObject* self, PyObject* Py_UNUSED) {
	g1cp=g1;
	g2cp=g2;
	servercp = server;
	Py_RETURN_NONE;
}
PyObject* step(PyObject* ,PyObject *args) {
	int x,y,copy;
	game_client *gobj1;
	game_client *gobj2;
	if (!PyArg_ParseTuple(args, "iip", &x,&y,&copy))
		return NULL;
	if (copy) {
		gobj1 = &g1cp;
		gobj2 = &g2cp;
	}
	else
	{
		gobj1 = &g1;
		gobj2 = &g2;
	}
	int reward = game_step(*gobj1, x);
	int reward2 = game_step(*gobj2, y);
	

	const npy_intp dim = 1477;
	const npy_intp* dims = &dim;
	int8_t* state = (int8_t*)malloc(sizeof(int8_t) * dim);
	state[1476] = server.stored_attack;
	if (gobj1->game_over||gobj2->game_over){
		if (gobj1->game_over&&gobj2->game_over)
		{
			state[0] = 127;
			state[738] = 127;
		}
		else if (gobj1->game_over) {
			state[0] = 127;
			state[738] = 126;

		}
		else {
			state[738 + 0] = 127;
			state[0] = 126;
		}
	}
	else {
		if (reward != 0) {
			state[0] = reward;
		}
		else {
			state[0] = gobj1->cleared + gobj1->spin;
		}
		if (reward2 != 0) {
			state[738] = reward2;
		}
		else {
			state[738 + 0] = gobj2->cleared + gobj2->spin;
		}
	}
	state[1] = gobj1->action_count;//gobj1->x+2;

	state[2] = gobj1->gheight;//gobj1->y;
	state[3] = gobj1->hold_used;
	state[4] = gobj1->rotation;
	state[5] = gobj1->active+1;
	state[6] = gobj1->held_piece+1;
	for (size_t i = 0; i < 5; i++)
	{
		state[i + 7] = gobj1->queue[i] + 1;
	}
	for (size_t i = 0; i < 210; i++)
	{
		state[i + 12] = gobj1->board[9 + i % 21][i / 21] + 1>0;
		//std::cout << state[i + 12] << " ";
	}
	//active
	for (int i = 0; i < 210; i++)
	{
		state[i + 222] = ((9 + i % 21) >= gobj1->y) && ((6 + i % 21) <= gobj1->y) && ((i / 21) - gobj1->x >= 0) && (gobj1->x + 3 - (i / 21) >= 0) ? (gobj1->piecedefs[gobj1->active][gobj1->rotation][(9 + i % 21) - gobj1->y][((i / 21) - gobj1->x)] + 1) > 0: 0;
	}
	//shadow
	int ny = gobj1->y + gobj1->softdropdist();
	for (int i = 0; i < 210; i++)
	{
		state[i + 432] = ((9 + i % 21) >= (ny)) && ((6 + i % 21) <= (ny)) && ((i / 21) - gobj1->x >= 0) && (gobj1->x + 3 - (i / 21) >= 0) ? (gobj1->piecedefs[gobj1->active][gobj1->rotation][(9 + i % 21) - (ny)][((i / 21) - gobj1->x)] + 1) > 0:0;
	}
	for (size_t j = 0; j < 4; j++)
	{
		for (size_t k = 0; k < 4; k++)
		{
			state[j * 4 + 642 + k] = gobj1->held_piece != -1 ? ((gobj1->piecedefs[gobj1->held_piece][0][k][j] + 1) > 0) : 0;
		}
	}

	for (size_t i = 0; i < 5; i++)
	{
		for (size_t j = 0; j < 4; j++)
		{
			for (size_t k = 0; k < 4; k++)
			{
				state[i * 16 + j * 4 + 658 + k] = ((gobj1->piecedefs[gobj1->queue[i]][0][k][j] + 1) > 0);
			}
		}
	}
	

	state[738+1] = gobj2->action_count;//gobj2->x+2;
	state[738+2] = gobj2->gheight;//gobj2->y;
	state[738+3] = gobj2->hold_used;
	state[738+4] = gobj2->rotation;
	state[738+5] = gobj2->active + 1;
	state[738+6] = gobj2->held_piece + 1;
	for (size_t i = 0; i < 5; i++)
	{
		state[738 + i + 7] = gobj2->queue[i] + 1;
	}
	for (size_t i = 0; i < 210; i++)
	{
		state[738 + i + 12] = gobj2->board[9 + i % 21][i / 21] + 1>0;
		//std::cout << state[738+i + 12] << " ";
	}
	//active
	for (int i = 0; i < 210; i++)
	{
		state[738 + i + 222] = ((9 + i % 21) >= gobj2->y) && ((6 + i % 21) <= gobj2->y) && ((i / 21) - gobj2->x >= 0) && (gobj2->x + 3 - (i / 21) >= 0) ? (gobj2->piecedefs[gobj2->active][gobj2->rotation][(9 + i % 21) - gobj2->y][((i / 21) - gobj2->x)] + 1) > 0: 0;
	}
	//shadow
	ny = gobj2->y + gobj2->softdropdist();
	for (int i = 0; i < 210; i++)
	{
		state[738 + i + 432] = ((9 + i % 21) >= (ny)) && ((6 + i % 21) <= (ny)) && ((i / 21) - gobj2->x >= 0) && (gobj2->x + 3 - (i / 21) >= 0) ? (gobj2->piecedefs[gobj2->active][gobj2->rotation][(9 + i % 21) - (ny)][((i / 21) - gobj2->x)] + 1) > 0:0;
	}
	for (size_t j = 0; j < 4; j++)
	{
		for (size_t k = 0; k < 4; k++)
		{
			state[738 + j * 4 + 642 + k] = gobj2->held_piece != -1 ? ((gobj2->piecedefs[gobj2->held_piece][0][k][j] + 1) > 0) : 0;
		}
	}

	for (size_t i = 0; i < 5; i++)
	{
		for (size_t j = 0; j < 4; j++)
		{
			for (size_t k = 0; k < 4; k++)
			{
				state[738 + i * 16 + j * 4 + 658 + k] = ((gobj2->piecedefs[gobj2->queue[i]][0][k][j] + 1) > 0);
			}
		}
	}
	PyObject* ret=PyArray_SimpleNewFromData(1, dims, NPY_INT8, state);
	PyArray_ENABLEFLAGS((PyArrayObject*)ret, NPY_ARRAY_OWNDATA);
	return ret;
	
	
}
/*PyObject* set(PyObject*, PyObject* args) {
	PyArrayObject* x;

	if (!PyArg_ParseTuple(args, "O!", &PyArray_Type, &x)) {
		return NULL;
	}

	npy_intp* shape = PyArray_SHAPE(x);

	int8_t* arr = (int8_t*)PyArray_DATA(x);
	if (arr == NULL)  return NULL;
	if (shape[0] != 444) {
		for (size_t i = 0; i < shape[0]; i++)
		{
			std::cout << (int)arr[i] << " ";
		}
		std::cout << "\n"; 
		PyErr_SetString(PyExc_ValueError, "wrong length");
		return NULL;
	}
	g1.reset();
	g2.reset();
	for (size_t i = 0; i < shape[0]; i++)
	{
		std::cout << (int)arr[i] << " ";
	}
	std::cout << "\n";
	server.stored_attack = arr[0];
	g1.cleared = 0;
	
	g1.hold_used = arr[3];
	g1.rotation = arr[4];
	g1.active = 6;
	g1.held_piece = arr[6]-1;
	for (size_t i = 0; i < 5; i++)
	{
		g1.queue[i]= arr[i + 7]-1;
	}
	for (size_t i = 0; i < 210; i++)
	{
		g1.board[9 + i % 21][i / 21] = arr[i + 12]-1;
	}
	g2.hold_used = arr[222+3];
	g2.rotation = arr[222+4];
	g2.active = 6;
	g2.held_piece = arr[222+6] - 1;
	for (size_t i = 0; i < 5; i++)
	{
		g2.queue[i] = arr[222+i + 7] - 1;
	}
	for (size_t i = 0; i < 210; i++)
	{
		g2.board[9 + i % 21][i / 21] = arr[222+i + 12] - 1;
	}
	Py_RETURN_NONE;

}*/
PyObject* close(PyObject* self, PyObject* Py_UNUSED) {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	Py_RETURN_NONE;
}
PyObject* render(PyObject* self, PyObject* Py_UNUSED) {
	if(!exited)
	{
		c_render(g1, g2);
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
	 //{"set",  set, METH_VARARGS,
	 //"Set board state"},
	 {"copy",  copy, METH_VARARGS,
	 "Copy board state"},
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