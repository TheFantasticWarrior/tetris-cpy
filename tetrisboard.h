#ifndef boardh
#define boardh

#include <random>
#include <vector>
const int COLUMNS=10;
const int ROWS=30;
const int INVISIBLE_ROWS=10;
int mod(int x, int y);

class game
{
	public:
		game() {};
		int game_over=0;
		int cleared=0;
		int board[ROWS][COLUMNS] = {};
		int active=0;
		int rotation=0;
		int x=0;
		int y=0;
		int recieved = 0;
		int queue[5] = {};
		int held_piece=-1;
		bool hold_used = false;
		bool b2b=0;
		int attack = 0;
		int combo = 0;
		int gheight=0;
		void set_seed(int seed);

		void random_recv(int max);


		void new_piece();
		void recieve(std::vector<int> list);

		int softdropdist() const;

		void reset();
		void sd();

		void softdrop();
		void harddrop();
		void harddrop2();
		void hold();
		void move(bool das,int d);
		void rotate(int direction);
		game(const game& other) {
			// Copy scalar values
			game_over = other.game_over;
			cleared = other.cleared;
			active = other.active;
			rotation = other.rotation;
			x = other.x;
			y = other.y;
			recieved = other.recieved;
			held_piece = other.held_piece;
			hold_used = other.hold_used;
			b2b = other.b2b;
			attack = other.attack;
			combo = other.combo;
			gheight = other.gheight;
			spin = other.spin;
			kick = other.kick;

			copy_board(board, other.board);
			hidden_queue.assign(other.hidden_queue.begin(), other.hidden_queue.end());

		}
		int piecedefs [7][4][4][4]= { //piece(SZJLTOI), rotation, position in board

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
		int bottom[7][4][4] = {
		{
			{2,2,3,4,},
			{4,2,1,4,},
			{1,1,2,4,},
			{2,1,4,4,},
		},
		{
			{3,2,2,4,},
			{4,1,2,4,},
			{2,1,1,4,},
			{1,2,4,4,},
		},
		{
			{2,2,2,4,},
			{4,1,3,4,},
			{2,2,1,4,},
			{1,1,4,4,},
		},
		{
			{2,2,2,4,},
			{4,1,1,4,},
			{1,2,2,4,},
			{3,1,4,4,},
		},
		{
			{2,2,2,4,},
			{4,1,2,4,},
			{2,1,2,4,},
			{2,1,4,4,},
		},
		{
			{4,2,2,4,},
			{4,2,2,4,},
			{4,2,2,4,},
			{4,2,2,4,},
		},
		{
			{2,2,2,2,},
			{4,4,0,4,},
			{1,1,1,1,},
			{4,0,4,4,},
		},
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

		bool spin = 0;
		bool kick=0;
	private:

		std::mt19937 gen;
		std::mt19937 gen2;
		int next_seed=0;
		bool seeded = false;
		std::vector<int> hidden_queue = {};
		void bag_randomizer();
		void place();

		void check_clear();
		void copy_board(int dest[ROWS][COLUMNS], const int src[ROWS][COLUMNS]);
};
#endif // !boardh