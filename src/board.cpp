#ifndef io
#define io
#include <iostream>
#endif // !io


#include <algorithm>
#include <random>
#include <string.h>

#ifndef tb
#define tb
#include "board.h"
#endif // !tb



/*
   int8_t game::check_tetris_hole(int8_t y,int8_t *filled,int8_t* bottom_filled,int8_t *empty)
   {
   int8_t* filled = 0;
   int8_t nempty=0;
   int8_t col_filled;
   for (size_t j = 0; j < 10; j++)
   {
   col_filled = 0;
   for (size_t i = 0; i < 4; i++)
   {
   if (board[y + i][j] >=0)
   {
 *filled++;
 col_filled++;
 }
 }
 if (col_filled == 0) {
 empty[nempty] = j;
 bottom_filled[nempty] = (y + 4 > 29 || board[y + 4][j] >= 0);
 nempty++;
 }
 }
 return nempty;
 }

 int8_t game::check_tsd_hole(int8_t y, int8_t* nfilled, int8_t* xloc)
 {
 int8_t valid;
 int8_t nholes=0;
 for (size_t i = 1; i < 9; i++)
 {
 if (board[y][i] >= 0)(*nfilled)++;
 else {
 valid = 0;
 for (size_t j = 0; j < 3; j++)
 {
 if (board[y - 1][i - 1 + j] >= 0)valid++;
 else break;
 }
 if (valid == 3) {
 xloc[nholes] = i;
 nholes++;
 }
 }
 }
 if (nholes)
 {
 if (board[y][0] >= 0)(*nfilled)++;
 if (board[y][9] >= 0)(*nfilled)++;
 for (size_t i = 0; i < 10; i++)
 {
 if (board[y-1][i]>=0)
 {
 (*nfilled)++;
 }
 }
 }
 return nholes;
 }

 int8_t game::check_tst_hole(int8_t y, int8_t x, int8_t* nfilled)
 {
 int8_t must_be_empty[4][2] = { {-1,-1},{-1,-3}, {-1,-4} }; // x,y
 int8_t corners[4][2] = { { -1,0 }, { 1,0 }, { -1,-2 }, { 1,-2 } };
 int8_t corners_filled = 0;
 float completeness = 0;
 if (x >= 2) {
 for (size_t i = 0; i < 4; i++)
 {
 if (board[y+must_be_empty[i][2]][x + must_be_empty[i][1]]!=-1)
{
    goto p2;
}
}
for (size_t i = 0; i < 4; i++)
{
    if (board[y + corners[i][2]][x + corners[i][1]] != -1)
    {
        corners_filled++;
    }
}
if (x==2)
{
    completeness =corners_filled / 3.;
}
else {
    completeness = corners_filled / 3. * (board[y - 4][x - 2] == -1 ? 1 : board[y - 5][x - 2] != -1 ? 1 : 0.5);
}
if (completeness)
{
    return completeness;
}
}
p2:
if (x <= 7) {
    for (size_t i = 0; i < 4; i++)
    {
        if (board[y + must_be_empty[i][2]][x - must_be_empty[i][1]] != -1)
        {
            return 0;
        }
    }
    corners_filled = 0;
    for (size_t i = 0; i < 4; i++)
    {
        if (board[y + corners[i][2]][x + corners[i][1]] != -1)
        {
            corners_filled++;
        }
    }
    if (x == 2)
    {
        completeness = corners_filled>2?1: corners_filled / 3.;
    }
    else {
        completeness = (corners_filled > 2 ? 1 : corners_filled / 3.) * (board[y - 4][x - 2] == -1 ? 1 : board[y - 5][x - 2] != -1 ? 1 : 0.5);
    }
    return completeness;

}
}
int8_t game::check_stsd_hole(int8_t y, int8_t x, int8_t* nfilled)
{
    int8_t must_be_empty[4][2] = { {-1,-1},{-1,-3}, {-1,-4} }; // x,y
    int8_t corners[3][2] = { { -1,0 }, { 1,0 }, { 1,-2 } };
    int8_t corners_filled = 0;
    float completeness = 0;
    if (x >= 2) {
        for (size_t i = 0; i < 4; i++)
        {
            if (board[y + must_be_empty[i][2]][x + must_be_empty[i][1]] != -1)
            {
                goto p2;
            }
        }
        for (size_t i = 0; i < 4; i++)
        {
            if (board[y + corners[i][2]][x + corners[i][1]] != -1)
            {
                corners_filled++;
            }
        }
        if (x == 2)
        {
            completeness = corners_filled / 3.;
        }
        else {
            completeness = corners_filled / 3. * (board[y - 4][x - 2] == -1 ? 1 : board[y - 5][x - 2] != -1 ? 1 : 0.5);
        }
        if (completeness)
        {
            return completeness;
        }
    }
p2:
    if (x <= 7) {
        for (size_t i = 0; i < 4; i++)
        {
            if (board[y + must_be_empty[i][2]][x - must_be_empty[i][1]] != -1)
            {
                return 0;
            }
        }
        corners_filled = 0;
        for (size_t i = 0; i < 4; i++)
        {
            if (board[y + corners[i][2]][x -corners[i][1]] != -1)
            {
                corners_filled++;
            }
        }
        if (x == 2)
        {
            completeness = corners_filled > 2 ? 1 : corners_filled / 3.;
        }
        else {
            completeness = (corners_filled > 2 ? 1 : corners_filled / 3.) * (board[y - 4][x - 2] == -1 ? 1 : board[y - 5][x - 2] != -1 ? 1 : 0.5);
        }
        return completeness;

    }
}
void game::eval_board()
{
    int8_t top = 10;
    int8_t i = 27;
    float score=0;
    int8_t filled;
    int8_t hole[10] = { 0 };
    int8_t bottom_filled[10] = {};
    int8_t nempty;
    int8_t x;
    bool tetris_hole = false;
    while(i>10){
        nempty=check_tetris_hole(i,&filled,bottom_filled,hole);
        if (nempty == 10) {
            top = i+4;
            break;
        }
        else if (nempty > 0)
        {
            for (size_t j = 0; j < nempty; j++)
            {
                if (bottom_filled[j]) {
                    check_stsd(i, hole[j], &filled);
                    tetris_hole = true;
                }
                if ((x=check_tst_hole(i, hole[j], &filled)))score += filled * x / 260.;
            }

            if(tetris_hole)score += filled * 2 / 360.;
        }
        i--;
    }
    while (i<30)
    {
        nempty = check_tsd_hole(i, &filled, hole);
        if (nempty)
        {
            score += filled*2 / 160.;
        }
        i++;
    }

}*/

void game::check_clear() {
    int8_t clear[4]={-1,-1,-1,-1};
    int8_t lines = 0;
    int8_t invalid = 0;
    garbage = 0;
    for (int8_t i = 0; i < 4; i++)
    {
        int8_t row = y + 3 - i;
        if (row < 30)
        {
            for (int8_t j = 0; j < COLUMNS; j++)
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
        for (size_t j = 0; j < 2; j++)
        {
            if (board[row][j*5] == 7)
            {
                garbage++;
                gheight--;
                goto end;
            }
        }
end:;
    }
    int8_t type = 0; //normal,t spin, t spin mini
    if (active == 4) { //T

        int8_t block = 0;
        int8_t oob = 0; //out of bounds
        int8_t frontcount = 0;
        int8_t front[4] = { 0b00,0b01,0b11,0b10 };
        for (int8_t i:{0, 1})
        {
            for (int8_t j : {0, 1}) {
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
        int8_t j = 29;
        int8_t idx = 0;
        int8_t offset = 0;
        while (j > 0) {
            int8_t ny = j - offset;
            if (idx < 4&&ny==clear[idx])
            {
                idx++;
                offset++;
            }
            else
            {
                if (ny<0)
                {
                    for (int8_t i = 0; i < 10; i++)
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

    cleared = (10 * (attack+ b2b *2* (lines > 0)) + 2 * lines+b2b);
}

void game::receive(std::vector<int8_t> list) {
    while (!list.empty()) {
        int8_t incoming = list[0];
        list.erase(list.begin());
        if (incoming > 0)
        {
            std::copy(board[incoming], board[30], board[0]);
            std::uniform_int_distribution<>dis(0, 9);
            int8_t x = dis(gen);
            bool thole = dis(gen2) < 1;
            for (size_t j = 0; j < incoming; j++)
            {
                for (size_t i = 0; i < 10; i++)
                {
                    board[29 - j][i] = (i == x || (incoming >= 2 && thole && (j == incoming - 1) && (i == x - 1 || i == x + 1))) ? -1 : 7;
                }
            }
            gheight += incoming;
        }
    }


}
void game::set_seed(int8_t seed) {

    std::random_device rd;
    if(seed==0)
    {
        int8_t seed = rd();
        gen.seed(seed);
    }
    else
    {
        gen.seed(seed);
    }
    seeded = true;
    next_seed = gen();

    gen2.seed(rd());
}

void game::random_recv(int8_t max) {
    int8_t recv;
    int8_t curr = 0;
    std::uniform_int_distribution<>dis(0, max);
    recv = dis(gen2);
    if (recv == 0) {
        receive(std::vector<int8_t>{1,1,1,1});
        return;
    }
    std::uniform_int_distribution<>dis2(0, 1);
    int8_t b2bon;
    while (curr < recv) {
        b2bon = dis2(gen2);
        receive(std::vector<int8_t>{(int8_t)(b2bon + 4)});
        curr += b2bon + 4;
    }
    return;
}
void game::reset(){
    if (!seeded)set_seed(0);
    else set_seed(next_seed);
    memset(board,-1,sizeof(board));
    gheight = 0;
    game_over = 0;
    cleared = 0;
    received = 0;
    hidden_queue.clear();
    held_piece = -1;
    hold_used = false;
    kick = 0;
    spin = 0;
    
    bag_randomizer();
    std::copy(hidden_queue.begin(),hidden_queue.begin()+5,queue);
    for(int i=0;i<5;i++){
    }
    hidden_queue.erase(hidden_queue.begin(),hidden_queue.begin()+5);
    new_piece();
}


void game::bag_randomizer()
{
    int8_t a[7]={0,1,2,3,4,5,6};
    std::shuffle(a,a+7,gen);
    hidden_queue.insert(hidden_queue.end(), std::begin(a),std::end(a) );
}
void game::place(){
    int8_t count = 0;
    for (int8_t i=0;i<4;i++){
        for (int8_t j=0;j<4;j++){
            if(piecedefs[active][rotation][j][i]!=-1){
                if (board[y+j][x+i]==-1){
                    board[y+j][x+i]=piecedefs[active][rotation][j][i];
                    if (y + j < 10)
                    {
                        count++;
                    }
                } else{
                    game_over = 1;
                }
            }
        }
    }
    if (count == 4)
    {
        game_over = 1;
    }
    check_clear();
    hold_used = false;
    spin = 0;
    kick = 0;
}


void game::new_piece(){
    y = 9;
    x = 3;
    active=queue[0];
    std::copy(queue+1,queue+5,queue);
    queue[4]=hidden_queue[0];
    for(int i=0;i<5;i++){
    }
    hidden_queue.erase(hidden_queue.begin());
    if (hidden_queue.size()==0){
        bag_randomizer();
    }
    rotation=0;
    spawn_game_over();
}
void game::spawn_game_over(){
start:
    for (int8_t i = 0; i < 4; i++) {
        for (int8_t j = 0; j < 4; j++) {
            if (piecedefs[active][rotation][j][i] != -1) {
                if (board[y + j][x + i] != -1) {
                    if(y==8)game_over=1;
                    else {
                        y = 8;
                        goto start;
                    }
                }
            }
        }
    }
    
}
void game::hold() {
    spin = 0;
    kick = 0;
    cleared = 0;
    if (!hold_used) {
        if (held_piece == -1) {
            held_piece = active;
            new_piece();
        }
        else {
            int temp=held_piece;
            held_piece = active;
            active=temp;
            
            spawn_game_over();

        }
        hold_used = true;
    }
}
int8_t game::softdropdist() const{
    int8_t height[4]={30,30,30,30};
    for (int8_t i = 0; i < 4; i++){
        if (bottom[active][rotation][i]!=4){
            int8_t j = 0;
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

    spin = 0;
    kick = 0;
    cleared = 0;
    for (int8_t i = 0; i < 4; i++) {
        if (bottom[active][rotation][i] != 4 && !(board[1 + y + 3 - bottom[active][rotation][i]][x + i] == -1 && 1 + y - bottom[active][rotation][i] < 27)){
            goto end;

        }
    }
    y += 1;
end:;
}
void game::softdrop() {
    y += softdropdist();
    spin = 0;
    kick = 0;
    cleared = 0;
}
void game::harddrop(){
    y+=softdropdist();
    place();
}
void game::harddrop2() { //slower
    int8_t ny = y;
    bool allowed = true;
    while (allowed) {
        ny++;
        for (int8_t i = 0; i < 4; i++) {
            for (int8_t j = 0; j < 4; j++) {
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
    for (int8_t i = 0; i < 4; i++) {
        for (int8_t j = 0; j < 4; j++) {
            if (piecedefs[active][rotation][j][i] != -1) {
                board[ny-1 + j][x + i] = piecedefs[active][rotation][j][i];
            }
        }
    }
    new_piece();
    hold_used = false;
}
int8_t mod(int8_t x,int8_t y) {
    return ((x % y) + y) % y;
}
void game::rotate(int8_t direction)
{
    cleared = 0;
    int8_t ny, nx, nr;
    if (direction != 2) {

        for (int8_t n = 0; n < 5; n++)
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
            for (int8_t i = 0; i < 4; i++) {

                for (int8_t j = 0; j < 4; j++) {

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
            if(n>0)
            {
                spin = 1;
                if (n == 4) {
                    kick = 1;
                }
            }
            goto end2;
end:;
        }
    }
    else
    {
        nr = mod(rotation + direction, 4);
        for (int8_t n = 0; n < 2; n++)
        {

            ny = y - n*((rotation == 0) ? 1 : (rotation == 2) ? -1 : 0);
            nx = x + n*((rotation == 1) ? 1 : (rotation == 3) ? -1 : 0);
            for (int8_t i = 0; i < 4; i++) {

                for (int8_t j = 0; j < 4; j++) {

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
void game::move(bool das,int8_t d) {
    int8_t nx = x+d;
    bool allowed = 1;

    kick = 0;
    cleared = 0;
    spin = 0;
    do {
        for (int8_t i = 0; i < 4; i++) {
            for (int8_t j = 0; j < 4; j++) {
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

void game::copy_board(int8_t dest[ROWS][COLUMNS], const int8_t src[ROWS][COLUMNS]) {
    for (int8_t i = 0; i < ROWS; ++i) {
        for (int8_t j = 0; j < COLUMNS; ++j) {
            dest[i][j] = src[i][j];
        }
    }
}
