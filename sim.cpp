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

class game_server;
class game_client;

class game_container {
public:
	class game_server {
	public:
		int stored_attack=0;
		std::vector<int> attack_queue{};
		game_server() { reset(); }
		game_server(const game_server& other) {
			stored_attack = other.stored_attack;

			attack_queue.resize(other.attack_queue.size());
			std::copy(other.attack_queue.begin(), other.attack_queue.end(), attack_queue.begin());
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
					while (new_atk > 0)
					{
						new_atk -= attack_queue[0];
						if (new_atk < 0)
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
		void reset() {
			stored_attack = 0;
			attack_queue.clear();
		}
	};
	class game_client : public game {
	public:
		game_server* server;
		int port=-1;
		int action_count = 0;
		bool last_invalid=false;
		game_client(game_server* s, int p) {
			server = s;
			port = p;
			set_seed(1);
			reset();
		}
		game_client(const game_client& other, game_server* s) : game(other) {
				server = s;
				port = other.port;
				action_count = other.action_count;
				last_invalid = other.last_invalid;
			
		}
		game_client() {
			server = (nullptr);
			port = -1;
		}
		void reset() {
			game::reset();
			action_count = 0;

			game::random_recv(3);
		}
		void harddrop() {
			game::harddrop();
			action_count = 0;

			if (combo)
				server->send(port, attack);
			else recieve(server->recieve(port));
			new_piece();

		}
		void game_step(int action) {
			action_count++;
			if (action_count == 10)
			{
				harddrop();
				last_invalid = 1;
			}
			else {
				switch (action)
				{
				case 0:
					hold();
					break;
				case 1:
					harddrop();
					break;
				case 2:
					rotate(1);
					break;
				case 3:
					rotate(-1);
					break;
				case 4:
					//x = x;
					move(0, -1);
					break;
				case 5:
					//x = x;
					move(0, 1);
					break;
				case 6:
					//x = x;
					move(1, -1);
					break;
				case 7:
					//x = x;
					move(1, 1);
					break;
				case 8:
					if (softdropdist() > 0)
						softdrop();
					break;
				case 9:
					rotate(2);
					break;
				default:
					break;
				}
			}
		}
	};
	PyObject_VAR_HEAD
	game_server *server;
	game_client* clients[2];
	game_container() : server(nullptr), clients{ nullptr, nullptr } {
		server = new game_server();
		clients[0] = new game_client(server, 1);
		clients[1] = new game_client(server, 2);
	}
	game_container(const game_container& other) {
		server = new game_server(*other.server);

		clients[0] = new game_client(*other.clients[0],server);
		clients[1] = new game_client(*other.clients[1],server);
	}
	~game_container() {
		if (server != nullptr) {
			delete server;
		}
		if (clients[0] != nullptr) {
			delete clients[0];
		}
		if (clients[1] != nullptr) {
			delete clients[1];
		}
	}
	static int init(game_container* self, PyObject* args) {
		self->server = new game_server();
		try {
			self->clients[0] = new game_client(self->server, 1);
			self->clients[1] = new game_client(self->server, 2);
		}
		catch (const std::exception& e) {
			// Print or log the exception details
			std::cerr << "Exception during initialization: " << e.what() << std::endl;
			return -1;  // Indicate failure
		}
		return 0;
	}

	static void dealloc(game_container* self) {
		Py_TYPE(self)->tp_free((PyObject*)self);
	}
	static PyObject* seed_reset(game_container* self, PyObject* args) {
		int x = 0;
		if (!PyArg_ParseTuple(args, "i", &x))
			return NULL;
		self->server->reset();

		for (game_client* client : self->clients) {
			client->set_seed(x);
			client->reset();

		}
		Py_RETURN_NONE;
	}
	static PyObject* reset(game_container* self, PyObject* args) {
		self->server->reset();

		for (game_client* client : self->clients) {
			client->reset();
		}
		Py_RETURN_NONE;
	}
	static PyObject* get_state(game_container* self, PyObject * Py_UNUSED) {
		const npy_intp dim = 1477;
		const npy_intp* dims = &dim;
		int8_t* state = new int8_t[dim];
		state[1476] = self->server->stored_attack;
		if (self->clients[0]->game_over|| self->clients[1]->game_over){
		if (self->clients[0]->game_over&&self->clients[1]->game_over)
		{
			state[0] = 127;
			state[738] = 127;
		}
		else if (self->clients[0]->game_over) {
			state[0] = 127;
			state[738] = 126;

		}
		else {
			state[738 + 0] = 127;
			state[0] = 126;
		}
	}
	else {
		if (self->clients[0]->last_invalid) {
			state[0] = 125;
		}
		else {
			state[0] = self->clients[0]->cleared + self->clients[0]->spin;
		}
		if (self->clients[1]->last_invalid) {
			state[738] = 125;
		}
		else {
			state[738 + 0] = self->clients[1]->cleared + self->clients[1]->spin;
		}
	}
		state[0] = self->clients[0]->cleared;
		state[1] = self->clients[0]->action_count;//self->clients[0]->x+2;

		state[2] = self->clients[0]->gheight;//self->clients[0]->y;
		state[3] = self->clients[0]->hold_used;
		state[4] = self->clients[0]->rotation;
		state[5] = self->clients[0]->active + 1;
		state[6] = self->clients[0]->held_piece + 1;
		for (size_t i = 0; i < 5; i++)
		{
			state[i + 7] = self->clients[0]->queue[i] + 1;
		}
		for (size_t i = 0; i < 210; i++)
		{
			state[i + 12] = self->clients[0]->board[9 + i % 21][i / 21] + 1 > 0;
		}
		//active
		for (int i = 0; i < 210; i++)
		{
			state[i + 222] = ((9 + i % 21) >= self->clients[0]->y) && ((6 + i % 21) <= self->clients[0]->y) && ((i / 21) - self->clients[0]->x >= 0) && (self->clients[0]->x + 3 - (i / 21) >= 0) ? (self->clients[0]->piecedefs[self->clients[0]->active][self->clients[0]->rotation][(9 + i % 21) - self->clients[0]->y][((i / 21) - self->clients[0]->x)] + 1) > 0: 0;
		}
		//shadow
		int ny = self->clients[0]->y + self->clients[0]->softdropdist();
		for (int i = 0; i < 210; i++)
		{
			state[i + 432] = ((9 + i % 21) >= (ny)) && ((6 + i % 21) <= (ny)) && ((i / 21) - self->clients[0]->x >= 0) && (self->clients[0]->x + 3 - (i / 21) >= 0) ? (self->clients[0]->piecedefs[self->clients[0]->active][self->clients[0]->rotation][(9 + i % 21) - (ny)][((i / 21) - self->clients[0]->x)] + 1) > 0:0;
		}
		for (size_t j = 0; j < 4; j++)
		{
			for (size_t k = 0; k < 4; k++)
			{
				state[j * 4 + 642 + k] = self->clients[0]->held_piece != -1 ? ((self->clients[0]->piecedefs[self->clients[0]->held_piece][0][k][j] + 1) > 0) : 0;
			}
		}

		for (size_t i = 0; i < 5; i++)
		{
			for (size_t j = 0; j < 4; j++)
			{
				for (size_t k = 0; k < 4; k++)
				{
					state[i * 16 + j * 4 + 658 + k] = ((self->clients[0]->piecedefs[self->clients[0]->queue[i]][0][k][j] + 1) > 0);
				}
			}
		}
		state[738 + 0] = self->clients[1]->cleared;
		state[738 + 1] = self->clients[1]->action_count;//self->clients[1]->x+2;
		state[738 + 2] = self->clients[1]->gheight;//self->clients[1]->y;
		state[738 + 3] = self->clients[1]->hold_used;
		state[738 + 4] = self->clients[1]->rotation;
		state[738 + 5] = self->clients[1]->active + 1;
		state[738 + 6] = self->clients[1]->held_piece + 1;
		for (size_t i = 0; i < 5; i++)
		{
			state[738 + i + 7] = self->clients[1]->queue[i] + 1;
		}
		for (size_t i = 0; i < 210; i++)
		{
			state[738 + i + 12] = self->clients[1]->board[9 + i % 21][i / 21] + 1 > 0;
		}
		//active
		for (int i = 0; i < 210; i++)
		{
			state[738 + i + 222] = ((9 + i % 21) >= self->clients[1]->y) && ((6 + i % 21) <= self->clients[1]->y) && ((i / 21) - self->clients[1]->x >= 0) && (self->clients[1]->x + 3 - (i / 21) >= 0) ? (self->clients[1]->piecedefs[self->clients[1]->active][self->clients[1]->rotation][(9 + i % 21) - self->clients[1]->y][((i / 21) - self->clients[1]->x)] + 1) > 0: 0;
		}
		//shadow
		ny = self->clients[1]->y + self->clients[1]->softdropdist();
		for (int i = 0; i < 210; i++)
		{
			state[738 + i + 432] = ((9 + i % 21) >= (ny)) && ((6 + i % 21) <= (ny)) && ((i / 21) - self->clients[1]->x >= 0) && (self->clients[1]->x + 3 - (i / 21) >= 0) ? (self->clients[1]->piecedefs[self->clients[1]->active][self->clients[1]->rotation][(9 + i % 21) - (ny)][((i / 21) - self->clients[1]->x)] + 1) > 0:0;
		}
		for (size_t j = 0; j < 4; j++)
		{
			for (size_t k = 0; k < 4; k++)
			{
				state[738 + j * 4 + 642 + k] = self->clients[1]->held_piece != -1 ? ((self->clients[1]->piecedefs[self->clients[1]->held_piece][0][k][j] + 1) > 0) : 0;
			}
		}

		for (size_t i = 0; i < 5; i++)
		{
			for (size_t j = 0; j < 4; j++)
			{
				for (size_t k = 0; k < 4; k++)
				{
					state[738 + i * 16 + j * 4 + 658 + k] = ((self->clients[1]->piecedefs[self->clients[1]->queue[i]][0][k][j] + 1) > 0);
				}
			}
		}
		PyObject* ret = PyArray_SimpleNewFromData(1, dims, NPY_INT8, state);
		PyArray_ENABLEFLAGS((PyArrayObject*)ret, NPY_ARRAY_OWNDATA);
		//free(state);
		return ret;
	}
	static PyObject* step(game_container* self, PyObject* args) {
		int x, y;
		if (!PyArg_ParseTuple(args, "ii", &x, &y))
			return NULL;
		self->clients[0]->game_step(x);
		self->clients[1]->game_step(y);
		Py_RETURN_NONE;
	}
	static game_container* copy(game_container* self, PyObject* Py_UNUSED);
};
static PyMethodDef game_container_methods[] = {
	{"seed_reset", (PyCFunction)game_container::seed_reset, METH_VARARGS, "Seed and reset"},
	{"reset", (PyCFunction)game_container::reset, METH_NOARGS, "Reset game"},
	{"get_state", (PyCFunction)game_container::get_state, METH_NOARGS, "Get game state"},
	{"step", (PyCFunction)game_container::step, METH_VARARGS, "Step game, two inputs for each board"},
	{"copy", (PyCFunction)game_container::copy, METH_NOARGS, "Copy game state"},
	{NULL, NULL, 0, NULL} // Sentinel
};
class game_renderer {
public:
	PyObject_VAR_HEAD
	game_renderer() {
		if (!SDL_WasInit(SDL_INIT_EVERYTHING)) {
			if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
				std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
				return;
			}
		}
	}
	game_renderer(int mode,int size) {
		if (!SDL_WasInit(SDL_INIT_EVERYTHING)) {
			if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
				std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
				return;
			}
		}
		c_set_size(mode, size);
		c_create_window();
	}
	~game_renderer() {
		c_close();
		SDL_Quit();
	}
	static void dealloc(game_renderer* self) {
		self->c_close();
		SDL_Quit();
		Py_TYPE(self)->tp_free((PyObject*)self);
		//delete self;
	}

	bool window_opened = false;
	static int init(game_renderer* self, PyObject* args) {
		int render_mode = -1;
		int render_size=-1;

		if (!PyArg_ParseTuple(args, "|ii", &render_mode, &render_size)) {
			return -1;
		}
		if (render_mode == -1 && render_size == -1)
			new (self) game_renderer();
		else
			new (self) game_renderer(render_mode, render_size);

		return 0;
	}
	static PyObject* create_window(game_renderer* self, PyObject* args) {
		int mode = 1;
		int size = 30;
		if (!PyArg_ParseTuple(args, "|ii", &mode, &size)) {
			return NULL;
		}
		self->c_set_size(mode, size);
		self->c_create_window();
		Py_RETURN_NONE;
	}
	static PyObject* close(game_renderer* self, PyObject* Py_UNUSED) {
		self->c_close();

		Py_RETURN_NONE;
	}
	static PyObject* render(game_renderer* self, PyObject* args);
private:
	SDL_Rect bg{};
	SDL_Rect rect{};
	SDL_Rect red_line{};
	SDL_Rect red_line_small{};
	SDL_Event event{};
	SDL_Window* window;
	SDL_Renderer* renderer;
	int block_size=30;
	int BOARDX=0;
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
	void c_set_size(int render_mode = 1, int render_size = 30) {
		switch (render_mode)
		{
		case 1:
			block_size = render_size;
			break;
		case 2:
			block_size = (render_size - 2) * 3 / 100 - 1;
			break;
		case 3:
			block_size = (render_size - 32) / 20.5f - 1;
			break;
		default:
			block_size = 30;
			break;
		}
	}
	void c_create_window() {

		c_close();


		int width = (block_size + 1) * 100 / 3 + 2;
		int height = 20.5f * (block_size + 1);
		BOARDX = width / 10;
		bg.y = 0; bg.w = width * 3 / 10; bg.h = height;
		rect.w = block_size; rect.h = block_size;
		red_line.w = (int)(block_size / 10), red_line.h = block_size + 1;
		red_line_small.w = (int)(block_size / 10), red_line_small.h = block_size;

		
		// Create window and renderer
		this->window = SDL_CreateWindow("Tetris", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
		if (!this->window) {
			std::cerr << "SDL window creation failed: " << SDL_GetError() << std::endl;
			return;
		}

		this->renderer = SDL_CreateRenderer(this->window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
		if (!this->renderer) {
			std::cerr << "S DL renderer creation failed: " << SDL_GetError() << std::endl;
			return;
		}

		SDL_SetRenderDrawBlendMode(this->renderer, SDL_BLENDMODE_BLEND);
		this->window_opened = true;
	}

	void c_close() {
		if (window_opened) {
			if (this->renderer) {
				SDL_DestroyRenderer(this->renderer);
				this->renderer = nullptr;
			}
			if (this->window) {
				SDL_DestroyWindow(this->window);
				this->window = nullptr;
			}
			window_opened = false;
		}
	}
	void color_from_rgb(uint32_t v) {
		SDL_SetRenderDrawColor(renderer, (v >> 16) & 0xFF, (v >> 8) & 0xFF, v & 0xFF, 0xFF);
	}
	void rgba_from_rgb(uint32_t v) {
		SDL_SetRenderDrawColor(renderer, (v >> 16) & 0xFF, (v >> 8) & 0xFF, v & 0xFF, 0x99);
	}
	void c_render(const game_container& g) {
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
		draw(*g.clients[0], 0);
		draw(*g.clients[1], BOARDX * 5);
		if(g.server->stored_attack!=0) draw_atk(g.server->stored_attack>0,g.server->attack_queue);
		SDL_RenderPresent(renderer);
	}
	void draw(const game& g, int xloc) {
		int ghosty;
		color_from_rgb(0x666666);
		bg.x += xloc;
		SDL_RenderFillRect(renderer, &bg);
		for (int i = 0; i < 10; i++)
		{
			for (int j = 0; j < 21; j++)
			{
				rect.x = 1 + BOARDX + i * (block_size + 1) + xloc;
				rect.y = -block_size / 2 + j * (block_size + 1);
				color_from_rgb(colors[g.board[j + 9][i] + 1]);
				SDL_RenderFillRect(renderer, &rect);

			}
		}
		// queue
		for (int n = 0; n < 5; n++)
		{

			color_from_rgb(colors[g.queue[n] + 1]);
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					rect.x = 4 * BOARDX + i * (block_size + 1) + xloc;
					rect.y = block_size * 3 * n + j * (block_size + 1);
					if (g.piecedefs[g.queue[n]][0][j][i] != -1)
					{
						SDL_RenderFillRect(renderer, &rect);
					}

				}
			}
		}
		//hold
		if (g.held_piece != -1)
		{
			color_from_rgb(colors[g.held_piece + 1]);
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					if (g.piecedefs[g.held_piece][0][j][i] != -1) {

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
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					rect.x = BOARDX + (g.x + i) * (block_size + 1) + xloc;
					rect.y = block_size / 2 + (g.y - 10 + j) * (block_size + 1);
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
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
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
		if (ghost) {
			ghosty = g.y + g.softdropdist();
			for (int i = 0; i < 4; i++)
			{
				for (int j = 0; j < 4; j++)
				{
					rect.x = BOARDX + (g.x + i) * (block_size + 1) + xloc;
					rect.y = block_size / 2 + (ghosty - 10 + j) * (block_size + 1);
					if (g.piecedefs[g.active][g.rotation][j][i] != -1)
					{
						rgba_from_rgb(colors[g.piecedefs[g.active][g.rotation][j][i] + 1]);
						SDL_RenderFillRect(renderer, &rect);
					}

				}
			}
		}
	}
	void draw_atk(int side, std::vector<int> attacks) {
		red_line.x = BOARDX + 11 * (block_size + 1) + side * BOARDX * 5;
		int sum = 0;
		for (int i:attacks)
		{
			for (size_t j = sum; j < sum+i-1; j++)
			{
				red_line.y = 21* block_size -block_size / 2 - j * (block_size + 1);
				SDL_RenderFillRect(renderer, &red_line);
			}
			sum += i - 1;
			red_line_small.y = 21 * block_size - block_size / 2 - sum * (block_size + 1);
			sum += 1;
			SDL_RenderFillRect(renderer, &red_line_small);

		}
	}
};
static PyMethodDef game_renderer_methods[] = {
	{"create_window", (PyCFunction)game_renderer::create_window, METH_VARARGS, "Create window, optional mode and size input"},
	{"close", (PyCFunction)game_renderer::close, METH_NOARGS, "Close window"},
	{"render", (PyCFunction)game_renderer::render, METH_VARARGS, "Render game, takes a Container object"},
	{NULL, NULL, 0, NULL} // Sentinel
};


static PyTypeObject game_container_type = {
	PyVarObject_HEAD_INIT(NULL, 0)
	.tp_name = "sim.Container",
	.tp_basicsize = sizeof(game_container),
	.tp_itemsize = 0,
	.tp_dealloc = (destructor)game_container::dealloc,
	.tp_flags = 0,// Py_TPFLAGS_BASETYPE,
	.tp_doc = "game_container objects",
	.tp_methods = game_container_methods,
	.tp_init = (initproc)game_container::init,
	.tp_new = PyType_GenericNew,//game_container::_new,
};
game_container* game_container::copy(game_container* self, PyObject* Py_UNUSED) {
	game_container* container = PyObject_New(game_container, &game_container_type);

	if (container == NULL) {
		PyErr_SetString(PyExc_MemoryError, "Failed to allocate memory for game_container");
		return NULL;
	}

	// Use the copy constructor to initialize the new object
	new (container) game_container(*self);

	return container;
}
PyObject* game_renderer::render(game_renderer* self, PyObject* args) {

	PyObject* pyg;
	if (!PyArg_ParseTuple(args, "O", &pyg)) {
		return NULL;
	}
	const game_container* g = reinterpret_cast<const game_container*>(pyg);
	if (self->window_opened)
	{
		self->c_render(*g);

		SDL_PollEvent(&self->event);
		if (self->event.type == SDL_QUIT) {
			self->c_close();
		}
	}
	else {
		std::cerr << "No Window open!\nUse create_window(render_mode,render_size) to create a window";
	}

	Py_RETURN_NONE;
}
static PyTypeObject game_renderer_type = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"sim.Renderer",  // name of the type
	sizeof(game_renderer),         // size of the type
	0,                             // itemsize, set to 0 for variable-sized objects
	(destructor)game_renderer::dealloc,  // tp_dealloc, destructor function
	0,                             // tp_print
	0,                             // tp_getattr
	0,                             // tp_setattr
	0,                             // tp_reserved
	0,                             // tp_repr
	0,                             // tp_as_number
	0,                             // tp_as_sequence
	0,                             // tp_as_mapping
	0,                             // tp_hash
	0,                             // tp_call
	0,                             // tp_str
	0,                             // tp_getattro
	0,                             // tp_setattro
	0,                             // tp_as_buffer
	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,  // tp_flags
	"Game Renderer",       // tp_doc, documentation string
	0,                             // tp_traverse
	0,                             // tp_clear
	0,                             // tp_richcompare
	0,                             // tp_weaklistoffset
	0,                             // tp_iter
	0,                             // tp_iternext
	game_renderer_methods,        // tp_methods, methods of the type
	0,                             // tp_members
	0,                             // tp_getset
	0,                             // tp_base
	0,                             // tp_dict
	0,                             // tp_descr_get
	0,                             // tp_descr_set
	0,                             // tp_dictoffset
	(initproc)game_renderer::init,  // tp_init, constructor function
	0,                             // tp_alloc
	PyType_GenericNew,             // tp_new, create a new object
};


static PyMethodDef Methods[] = {
	 /*{"make",  make, METH_VARARGS,
	 "Makes and returns the game container. "},
	 {"render",  render, METH_VARARGS,
	 "Creates a renderer, returns renderer. "},*/
	 {NULL, NULL, 0, NULL}
};
static struct PyModuleDef Module = {
	PyModuleDef_HEAD_INIT,
	"sim",   /* name of module */
	NULL,
	-1,
	Methods
};
PyMODINIT_FUNC
PyInit_sim(void)
{
	import_array();
	PyObject* m= PyModule_Create(&Module);
	if (m == NULL)
		return NULL;

	if (PyType_Ready(&game_container_type) != 0)
		return NULL;

	Py_INCREF(&game_container_type);
	PyModule_AddObject(m, "Container", (PyObject*)&game_container_type);

	if (PyType_Ready(&game_renderer_type) < 0)
		return NULL;
	Py_INCREF(&game_renderer_type);
	PyModule_AddObject(m, "Renderer", (PyObject*)&game_renderer_type);

	return m;
}
int main(int argc, char* argv[]) {
	
	return 0;
}