#ifndef boardh
#define boardh

#include <vector>
const int COLUMNS=10;
const int ROWS=30;
const int INVISIBLE_ROWS=10;
int mod(int x, int y);
class game
{
	public:
    
	int board[ROWS][COLUMNS];
	//int active_board[ROWS][COLUMNS];
	int active;
	int rotation;
	int x;
	int y;
    int queue[5];
	int held=-1;
	int hold_used = false;
    void print_row(int index);

	int* check_clear();
	
	void print_board();
	void reset();
	void game_over();
	int softdropdist();
	void sd();
	void harddrop();
	void harddrop2();
	void hold();
	void move(int das,int d);
    int piecedefs [7][4][4][4]= { //piece, rotation, position in board

	{ // S
	{
		{-1, 0, 0, -1},
		{0, 0, -1, -1},
		{-1, -1, -1, -1},
		{-1, -1, -1, -1}
	},
	{
		{-1, 0, -1, -1},
		{-1, 0, 0, -1},
		{-1, -1, 0, -1},
		{-1, -1, -1, -1}
	},
	{
		{-1, -1, -1, -1},
		{-1, 0, 0, -1},
		{0, 0, -1, -1},
		{-1, -1, -1, -1}
	},
	{
		{0, -1, -1, -1},
		{0, 0, -1, -1},
		{-1, 0, -1, -1},
		{-1, -1, -1, -1}
	}
	},
	{ // Z
	{
		{1, 1, -1, -1},
		{-1, 1, 1, -1},
		{-1, -1, -1, -1},
		{-1, -1, -1, -1}
	},
	{
		{-1, -1, 1, -1},
		{-1, 1, 1, -1},
		{-1, 1, -1, -1},
		{-1, -1, -1, -1}
	},
	{
		{-1, -1, -1, -1},
		{1, 1, -1, -1},
		{-1, 1, 1, -1},
		{-1, -1, -1, -1}
	},
	{
		{-1, 1, -1, -1},
		{1, 1, -1, -1},
		{1, -1, -1, -1},
		{-1, -1, -1, -1}
	}
	},
	{ // J
	{
		{2, -1, -1, -1},
		{2, 2, 2, -1},
		{-1, -1, -1, -1},
		{-1, -1, -1, -1}
	},
	{
		{-1, 2, 2, -1},
		{-1, 2, -1, -1},
		{-1, 2, -1, -1},
		{-1, -1, -1, -1}
	},
	{
		{-1, -1, -1, -1},
		{2, 2, 2, -1},
		{-1, -1, 2, -1},
		{-1, -1, -1, -1}
	},
	{
		{-1, 2, -1, -1},
		{-1, 2, -1, -1},
		{2, 2, -1, -1},
		{-1, -1, -1, -1}
	}
	},
	{ // L
	{
		{-1, -1, 3, -1},
		{3, 3, 3, -1},
		{-1, -1, -1, -1},
		{-1, -1, -1, -1}
	},
	{
		{-1, 3, -1, -1},
		{-1, 3, -1, -1},
		{-1, 3, 3, -1},
		{-1, -1, -1, -1}
	},
	{
		{-1, -1, -1, -1},
		{3, 3, 3, -1},
		{3, -1, -1, -1},
		{-1, -1, -1, -1}
	},
	{
		{3, 3, -1, -1},
		{-1, 3, -1, -1},
		{-1, 3, -1, -1},
		{-1, -1, -1, -1}
	}
	},
	{ // T
	{
		{-1, 4, -1, -1},
		{4, 4, 4, -1},
		{-1, -1, -1, -1},
		{-1, -1, -1, -1}
	},
	{
		{-1, 4, -1, -1},
		{-1, 4, 4, -1},
		{-1, 4, -1, -1},
		{-1, -1, -1, -1}
	},
	{
		{-1, -1, -1, -1},
		{4, 4, 4, -1},
		{-1, 4, -1, -1},
		{-1, -1, -1, -1}
	},
	{
		{-1, 4, -1, -1},
		{4, 4, -1, -1},
		{-1, 4, -1, -1},
		{-1, -1, -1, -1}
	}
	},
	{ // O
	{
		{-1, 5, 5, -1},
		{-1, 5, 5, -1},
		{-1, -1, -1, -1},
		{-1, -1, -1, -1}
	},
	{
		{-1, 5, 5, -1},
		{-1, 5, 5, -1},
		{-1, -1, -1, -1},
		{-1, -1, -1, -1}
	},
	{
		{-1, 5, 5, -1},
		{-1, 5, 5, -1},
		{-1, -1, -1, -1},
		{-1, -1, -1, -1}
	},
	{
		{-1, 5, 5, -1},
		{-1, 5, 5, -1},
		{-1, -1, -1, -1},
		{-1, -1, -1, -1}
	}
	},
	{ // I
	{
		{-1, -1, -1, -1},
		{6, 6, 6, 6},
		{-1, -1, -1, -1},
		{-1, -1, -1, -1}
	},
	{
		{-1, -1, 6, -1},
		{-1, -1, 6, -1},
		{-1, -1, 6, -1},
		{-1, -1, 6, -1}
	},
	{
		{-1, -1, -1, -1},
		{-1, -1, -1, -1},
		{6, 6, 6, 6},
		{-1, -1, -1, -1}
	},
	{
		{-1, 6, -1, -1},
		{-1, 6, -1, -1},
		{-1, 6, -1, -1},
		{-1, 6, -1, -1}
	}
	}
};
	int wallkick[4][5][2] = {
		{{ 0, 0},{-1, 0},{-1,+1},{ 0,-2},{-1,-2}},
		{{ 0, 0},{+1, 0},{+1,-1},{ 0,+2},{+1,+2}},
		{{ 0, 0},{+1, 0},{+1,+1},{ 0,-2},{+1,-2}},
		{{ 0, 0},{-1, 0},{-1,-1},{ 0,+2},{-1,+2}},
	};
	int ikick[4][5][2] = {
		{{0, 0},{-2, 0},{+1, 0},{-2,-1},{+1,+2}},
		{{0, 0},{-1, 0},{+2, 0},{-1,+2},{+2,-1}},
		{{0, 0},{+2, 0},{-1, 0},{+2,+1},{-1,-2}},
		{{0, 0},{+1, 0},{-2, 0},{+1,-2},{-2,+1}}
	};
	void rotate(int direction);
	private:
	std::vector<int> hidden_queue;
	int bottom[4];
	void bag_randomizer();
	void place();
	//int place_ghost(int x,int y)

	void new_piece();
	void get_bottom(int rotation);
    
	
};
#endif // !boardh