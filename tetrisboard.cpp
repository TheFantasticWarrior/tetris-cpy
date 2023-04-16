#ifndef io
#define io
#include <iostream>
#endif // !io


#include <algorithm>
#include <string.h>

#ifndef tb
#define tb
#include "tetrisboard.h"
#endif // !tb




int game_over = 0;
int cleared = 0;
int game::check_clear() {
	int clear[4]={-1,-1,-1,-1};
	int lines = 0;
	int invalid = 0;
	for (int_fast8_t i = 0; i < 4; i++)
	{
		int row = y + 3 - i;
		if (row < 30)
		{
			for (int_fast8_t j = 0; j < COLUMNS; j++)
			{
				if (board[row][j] == -1) {
					invalid++;
					goto end;
				}
			}
		}
		else
		{
			invalid++;
			goto end;
		}
		clear[i - invalid] = row;
		lines++;
	end:;
	}
	int type = 0; //normal,t spin, t spin mini
	if (active == 4) { //T
		
		int block = 0;
		int oob = 0; //out of bounds
		int frontcount = 0;
		int front[4] = { 0b00,0b01,0b11,0b10 };
		for (int_fast8_t i:{0, 1})
		{
			for (int_fast8_t j : {0, 1}) {
				if (y + 1 + i > 29 || x + 1 + j < 0 || x + 1 + j>10) {
					oob++;
				}
				else if (board[y +2* i][x + 2* j]!=-1)
				{
					if ((i << 1) + j == front[rotation] || (i << 1) + j == front[mod(rotation + 1, 4)])
					{
						frontcount++;
					}
					block++;
				}
			}

		}
		if (block+oob >= 3) {
			if ((frontcount == 2) || kick) {
				type = 1;
			}
			else
			{
				type = 2;
			}
		}
	}
	

	if (lines)
	{
		combo++;
		//board clear
		int j = 29;
		int i = 0;
		int offset = 0;
		while (j > 0) {
			int ny = j - offset;
			if (ny==clear[i])
			{
				i++;
				offset++;
			}
			else
			{
				if (ny<0)
				{
					for (int_fast8_t i = 0; i < 10; i++)
					{
						board[j][i] = -1;
					}

				}else
				{
					std::copy(board[ny], board[ny] + 10, board[j]);
				}
				j--;
			}
		}
		if (lines == 4) {
			attack = 4+b2b;
			b2b = 1;
		}
		else if (type == 1){
			attack = lines * 2 + b2b;
			
			b2b = 1;
		}
		else if (type == 2) {
			attack = lines - 1+b2b;
			b2b = 1;
		}
		else
		{
			attack = lines - 1;
			b2b = 0;
		}
	}
	else
	{
		attack = 0;
		combo = 0;
	}
	
	return lines;
}

void game::set_seed(int *seed) {
	if(seed!=NULL)
	{
		std::random_device rd;
		std::mt19937 gen(rd());
	}
	else
	{
		std::mt19937 gen(*seed);
	}
}
void game::reset(){
	memset(board,-1,sizeof(board));
	game_over = 0;
	hidden_queue.clear();
	held_piece = -1;
	hold_used = false;
	kick = 0;
	new_piece();
}


void game::bag_randomizer()
{
	int a[7]={0,1,2,3,4,5,6};
	std::shuffle(a,a+7,gen);
	hidden_queue.insert(hidden_queue.end(), std::begin(a),std::end(a) );
}
void game::place(){
	int count = 0;
	for (int_fast8_t i=0;i<4;i++){
		for (int_fast8_t j=0;j<4;j++){
			if(piecedefs[active][rotation][j][i]!=-1){
				if (board[y+j][x+i]==-1){
					board[y+j][x+i]=piecedefs[active][rotation][j][i];
					if (y + j < 10)
					{
						count++;
					}
				} else{
					//std::cout<<y+j<<" "<<x+i<<"error\n";
					game_over = 1;
				}
			}
		}
	}
	if (count == 4)
	{
		game_over = 1;
	}
	cleared=check_clear();
	new_piece();
	hold_used = false;
	kick = 0;
}


void game::new_piece(){
	y = 9;
	x = 3;
	if (hidden_queue.size()<7){
		bag_randomizer();
	}
	active=hidden_queue[0];
	rotation=0;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			if (piecedefs[active][rotation][j][i] != -1) {
				if (board[y + j][x + i] != -1) {
					//std::cout << y + j << " " << x + i << "collide\n";
					game_over=1;
				}
			}
		}
	}
	hidden_queue.erase(hidden_queue.begin());
	std::copy(hidden_queue.begin(),hidden_queue.begin()+5,queue);
	
}
void game::hold() {
	if (!hold_used) {
		if (held_piece == -1) {
			held_piece = active;
			new_piece();
		}
		else {
			hidden_queue.insert(hidden_queue.begin(),held_piece);
			held_piece = active;
			new_piece();
		}
		hold_used = true;
		kick = 0;
	}
}
int game::softdropdist(){
	int height[4]={30,30,30,30};
	for (int_fast8_t i = 0; i < 4; i++){
		if (bottom[active][rotation][i]!=4){
			int j = 0;
			while (board[j + y+3-bottom[active][rotation][i]][x+i] == -1&& j + y - bottom[active][rotation][i]<27)
			{
				j++;
			}
			height[i]=j-1;
		}
	}
	return *std::min_element(height,height+4);
}
void game::sd() {

	for (int_fast8_t i = 0; i < 4; i++) {
		if (bottom[active][rotation][i] != 4 && !(board[1 + y + 3 - bottom[active][rotation][i]][x + i] == -1 && 1 + y - bottom[active][rotation][i] < 27)){
			goto end;
			
		}
	}
	y += 1;
	kick = 0;
end:;
}
void game::softdrop() {
	y += softdropdist();
}
void game::harddrop(){
	y+=softdropdist();
	place();
}
void game::harddrop2() { //slower
	int ny = y;
	bool allowed = true;
	while (allowed) {
		ny++;
		for (int_fast8_t i = 0; i < 4; i++) {
			for (int_fast8_t j = 0; j < 4; j++) {
				if (piecedefs[active][rotation][j][i] != -1) {
					if (board[ny + j][x + i] != -1) {
						allowed = false;
						goto end;
					}
				}
			}
		}
	}
	end:
	for (int_fast8_t i = 0; i < 4; i++) {
		for (int_fast8_t j = 0; j < 4; j++) {
			if (piecedefs[active][rotation][j][i] != -1) {
				board[ny-1 + j][x + i] = piecedefs[active][rotation][j][i];
			}
		}
	}
	new_piece();
	hold_used = false;
}
int mod(int x,int y) {
	return ((x % y) + y) % y;
}
void game::rotate(int direction)
{
	int ny, nx, nr;
	if (direction != 2) {

		for (int_fast8_t n = 0; n < 5; n++)
		{
			nr = mod(rotation + direction, 4);
			if (active==6)
			{
				ny = y - direction * ikick[direction==-1?nr:rotation][n][1];
				nx = x + direction * ikick[direction == -1 ? nr : rotation][n][0];
			}
			else
			{
				ny = y - direction * wallkick[direction == -1 ? nr : rotation][n][1];
				nx = x + direction * wallkick[direction == -1 ? nr : rotation][n][0];
			}
			for (int_fast8_t i = 0; i < 4; i++) {
				
				for (int_fast8_t j = 0; j < 4; j++) {
					
					if (piecedefs[active][nr][j][i] != -1) {
						if ((board[ny + j][nx + i] != -1) || (ny + j > 30) || !(0 <= nx + i &&nx+i< COLUMNS)) {
							goto end;
						}
					}
				}
			}
			
			y = ny;
			x = nx;
			rotation = nr;
			if (n==4) {
				kick = 1;
			}
			goto end2;
		end:;
		}
	}
	else
	{
		nr = mod(rotation + direction, 4);
		for (int_fast8_t n = 0; n < 2; n++)
		{
			
			ny = y - n*((rotation == 0) ? 1 : (rotation == 2) ? -1 : 0);
			nx = x + n*((rotation == 1) ? 1 : (rotation == 3) ? -1 : 0);
			for (int_fast8_t i = 0; i < 4; i++) {

				for (int_fast8_t j = 0; j < 4; j++) {

					if (piecedefs[active][nr][j][i] != -1) {
						if ((board[ny + j][nx + i] != -1) || (ny + j > 30) || !(0 <= nx + i && nx + i < COLUMNS)) {
							goto end3;
						}
					}
				}
			}
			y = ny;
			x = nx;
			rotation = nr;
			goto end2;
		end3:;
		}
			
	}
end2:;
}
void game::move(bool das,int d) {
	int nx = x+d;
	bool allowed = 1;
	do {
		for (int_fast8_t i = 0; i < 4; i++) {
			for (int_fast8_t j = 0; j < 4; j++) {
				if (piecedefs[active][rotation][j][i] != -1) {
					if (board[y + j][nx + i] != -1|| !(0<=nx+i&&nx+i<COLUMNS)) {
						
						allowed = 0;
						goto end;
					}
				}
			}
		}
		x = nx;
		nx += d;
		kick = 0;
	} while (das&&allowed);
end:;
	//
}