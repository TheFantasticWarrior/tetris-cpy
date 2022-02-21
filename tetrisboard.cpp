#ifndef io
#define io
#include <iostream>
#endif // !io


#include <random>
#include <algorithm>
#include <string.h>
#include "tetrisboard.h"

std::random_device rd;
std::mt19937 gen(rd());


void game::print_row(int index){
	for (int i=0;i<COLUMNS;i++)
		{
			int val=board[index][i];
			if (val!=-1)
			{
				std::cout<<" "<<val;
			} else
			{
				std::cout<<val;
			}
			
		}
		std::cout<<"\n";
	
	
}
int* game::check_clear() {
	int clear[4]={-1,-1,-1,-1};
	int lines = 0;
	int invalid = 0;
	for (size_t i = 0; i < 4; i++)
	{
		int row = y + 3 - i;
		if (row < 30)
		{
			std::cout << row << "\n";
			for (size_t j = 0; j < COLUMNS; j++)
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
		clear[i-invalid] =row;
		std::cout << row << " clear \n";
		lines++;
	end:;
	}
	if (lines)
	{
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
					for (int i = 0; i < 10; i++)
					{
						board[j][i] = -1;
					}

				}else
				{
					std::copy(board[ny], board[ny] + 10, board[j]);
				}
				j--;
			}
			//std::cout << i << j << offset;
		}
		std::cout << "done\n";
	}
	int clears[2] = {lines,0};
	return clears;
}
void game::print_board()
{
	for (int i=INVISIBLE_ROWS;i<ROWS;i++)
		{
			std::cout<<i<<" ";
			print_row(i);
		}
	std::cout<<"\n";
	/*for (size_t i = 0; i < 5; i++)
	{
		std::cout<<hidden_queue[i]<<"\n";
	}
	std::cout<<"\n";*/
}

void game::reset(){
	memset(board,-1,sizeof(board));

	hidden_queue.clear();
	held = -1;
	hold_used = false;
	new_piece();
}
void game::game_over(){
	std::cout<<"game over\n";
	exit(0);
}

void game::bag_randomizer()
{
	int a[7]={0,1,2,3,4,5,6};
	std::shuffle(a,a+7,gen);
	hidden_queue.insert(hidden_queue.end(), std::begin(a),std::end(a) );
}
void game::place(){
	for (int i=0;i<4;i++){
		for (int j=0;j<4;j++){
			if(piecedefs[active][rotation][j][i]!=-1){
				if (board[y+j][x+i]==-1){
					board[y+j][x+i]=piecedefs[active][rotation][j][i];
				} else{
					std::cout<<y+j<<" "<<x+i<<"error\n";
					game_over();
				}
			}
		}
	}
	check_clear();
	new_piece();
	hold_used = false;
}


void game::new_piece(){
	y = 9;
	x = 3;
	if (hidden_queue.size()<7){
		bag_randomizer();
	}
	active=hidden_queue[0];
	rotation=0;
	hidden_queue.erase(hidden_queue.begin());
	std::copy(hidden_queue.begin(),hidden_queue.begin()+5,queue);
	
}
void game::hold() {
	if (!hold_used) {
		if (held == -1) {
			held = active;
			new_piece();
		}
		else {
			hidden_queue.insert(hidden_queue.begin(),held);
			held = active;
			new_piece();
		}
		hold_used = true;
	}
}
void game::get_bottom(int rotation){
	for (size_t i = 0; i < 4; i++)
	{
		bottom[i] = 4;
		for (int j = 0; j <4; j++)
		{
			
			if(piecedefs[active][rotation][3-j][i]!=-1){
				bottom[i] = j;
				//std::cout << j << std::endl;
				break;
			}
			
			
		}
	}
}
int game::softdropdist(){
	get_bottom(rotation);
	int height[4]={30,30,30,30};
	for (size_t i = 0; i < 4; i++){
		if (bottom[i]!=4){
			int j = 0;
			while (board[j + y+3-bottom[i]][x+i] == -1)
			{
				j++;
			}
			height[i]=j-1;
		}
	}
	return *std::min_element(height,height+4);
}
void game::sd() {
	get_bottom(rotation);
	for (size_t i = 0; i < 4; i++) {
		for (size_t j = 0; j < 4; j++)
		{
			if (y + j <29&&board[y + j + 1][x + i] != -1 && piecedefs[active][rotation][j][i] != -1) {
				goto end;
			}
		}
	}
	y += 1;
end:;
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
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
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
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
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

		for (size_t n = 0; n < 5; n++)
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
			for (int i = 0; i < 4; i++) {
				
				for (int j = 0; j < 4; j++) {
					
					//std::cout << ny+j << nx+i << nr<<std::endl;
					if (piecedefs[active][nr][j][i] != -1) {
						if ((board[ny + j][nx + i] != -1) || (ny + j > 30) || !(0 <= nx + i &&nx+i< COLUMNS)) {
							//std::cout << "fail\n\n";
							goto end;
						}
						else
						{
							
						}
					}
				}
			}
			
			y = ny;
			x = nx;
			rotation = nr;
			//std::cout <<y<<std::endl;
			goto end2;
		end:;
		}
	}
	else
	{
		nr = mod(rotation + direction, 4);
		for (size_t n = 0; n < 2; n++)
		{
			
			ny = y - n*((rotation == 0) ? 1 : (rotation == 2) ? -1 : 0);
			nx = x + n*((rotation == 1) ? 1 : (rotation == 3) ? -1 : 0);
			for (int i = 0; i < 4; i++) {

				for (int j = 0; j < 4; j++) {

					//std::cout << ny+j << nx+i << nr<<std::endl;
					if (piecedefs[active][nr][j][i] != -1) {
						if ((board[ny + j][nx + i] != -1) || (ny + j > 30) || !(0 <= nx + i && nx + i < COLUMNS)) {
							//std::cout << "fail\n\n";
							goto end3;
						}
					}
				}
			}
			y = ny;
			x = nx;
			rotation = nr;
			//std::cout <<y<<std::endl;
			goto end2;
		end3:;
		}
			
	}
end2:;
}
void game::move(int das,int d) {
	int nx = x+d;
	bool allowed = 1;
	do {
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
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
	} while (das&&allowed);
end:;
	//
}