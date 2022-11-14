#include "inireader.h"
INIReader reader("config.ini");
int DAS = reader.GetInteger("control","DAS",133);
int DROPDAS = reader.GetInteger("control", "SOFTDROPDELAY", 20);
int ARR = reader.GetInteger("control", "ARR", 0);

// https://wiki.libsdl.org/SDL_Keycode
int HARDDROP = reader.GetInteger("control", "HARDDROP", 'v');
int SOFTDROP = reader.GetInteger("control", "SOFTDROP", 0x40000051);
int LEFT = reader.GetInteger("control", "LEFT", 0x40000050);
int RIGHT = reader.GetInteger("control", "RIGHT", 0x4000004F);
int CW = reader.GetInteger("control", "CW", 'x');
int CCW = reader.GetInteger("control", "CCW", 'z');
int HOLD = reader.GetInteger("control", "HOLD", 'c');
int RT = reader.GetInteger("control", "180", 'a'); //180
int RESET = reader.GetInteger("control", "RESET", 0x4000003B);

int t = reader.GetInteger("graphics", "type", 3);
int size = reader.GetInteger("graphics", "value", 1080);
bool ghost = reader.GetBoolean("graphics", "ghost", true);
bool active_piece = reader.GetBoolean("graphics", "active_piece", true);


