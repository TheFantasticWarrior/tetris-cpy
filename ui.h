
constexpr int PIXEL_SIZE = 40;
constexpr int WIDTH = (PIXEL_SIZE + 1) * 50 / 3+2;
constexpr int HEIGHT = 20.5 * (PIXEL_SIZE + 1);
bool hdrelease = true;


bool open = true;
SDL_Event event;
SDL_Window* window;
SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
int BOARDX = WIDTH / 5;
SDL_Rect bg{ BOARDX,0,WIDTH*3/5, HEIGHT };
SDL_Rect rect{ PIXEL_SIZE,PIXEL_SIZE ,PIXEL_SIZE ,PIXEL_SIZE };
int frameCount, lastFrame, fps;


int colors[9]{ //bg SZJLTOI garbage
	0x000000,
	0x59b101,
	0xd70f37,
	0x2141c6,
	0xe35b02,
	0xaf298a,
	0xe39f02,
	0x0f9bd7,
	0x666666
};