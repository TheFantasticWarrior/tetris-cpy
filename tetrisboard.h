#ifndef boardh
#define boardh

#include <random>
#include <vector>
const int8_t COLUMNS=10;
const int8_t ROWS=30;
const int8_t INVISIBLE_ROWS=10;
int8_t mod(int8_t x, int8_t y);

class game
{
    public:
        game() {};
        int8_t game_over=0;
        int8_t cleared=0;
        int8_t board[ROWS][COLUMNS] = {};
        int8_t active=0;
        int8_t rotation=0;
        int8_t x=0;
        int8_t y=0;
        int8_t received = 0;
        int8_t queue[5] = {};
        int8_t held_piece=-1;
        bool hold_used = false;
        bool b2b=0;
        int8_t attack = 0;
        int8_t combo = 0;
        int8_t gheight=0;
        void set_seed(int8_t seed);

        void random_recv(int8_t max);


        void new_piece();
        void receive(std::vector<int8_t> list);

        int8_t softdropdist() const;

        void reset();
        void sd();

        void softdrop();
        void harddrop();
        void harddrop2();
        void hold();
        void move(bool das,int8_t d);
        void rotate(int8_t direction);
        
        
        game(const game& other) {
            // Copy scalar values
            game_over = other.game_over;
            cleared = other.cleared;
            active = other.active;
            rotation = other.rotation;
            x = other.x;
            y = other.y;
            received = other.received;
            held_piece = other.held_piece;
            hold_used = other.hold_used;
            b2b = other.b2b;
            attack = other.attack;
            combo = other.combo;
            gheight = other.gheight;
            spin = other.spin;
            kick = other.kick;
            for (int8_t i = 0; i < 5; ++i) {
                queue[i] = other.queue[i];
            }
            copy_board(board, other.board);
            hidden_queue.assign(other.hidden_queue.begin(), other.hidden_queue.end());
            gen=other.gen;
            gen2=other.gen2;

        }
        int8_t garbage=0;
        int8_t piecedefs [7][4][4][4]= { //piece(SZJLTOI), rotation, position in board

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
        int8_t bottom[7][4][4] = {
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
        int8_t wallkick[4][5][2] = {
            {{ 0, 0},{-1, 0},{-1,+1},{ 0,-2},{-1,-2}},
            {{ 0, 0},{+1, 0},{+1,-1},{ 0,+2},{+1,+2}},
            {{ 0, 0},{+1, 0},{+1,+1},{ 0,-2},{+1,-2}},
            {{ 0, 0},{-1, 0},{-1,-1},{ 0,+2},{-1,+2}},
        };
        int8_t ikick[4][5][2] = {
            {{0, 0},{-2, 0},{+1, 0},{-2,-1},{+1,+2}},
            {{0, 0},{-1, 0},{+2, 0},{-1,+2},{+2,-1}},
            {{0, 0},{+2, 0},{-1, 0},{+2,+1},{-1,-2}},
            {{0, 0},{+1, 0},{-2, 0},{+1,-2},{-2,+1}}
        };

        std::mt19937 gen;
        std::mt19937 gen2;
        bool spin = 0;
        bool kick=0;
        std::vector<int8_t> hidden_queue = {};
    private:

        int8_t next_seed=0;
        bool seeded = false;
        void bag_randomizer();
        void place();
        void spawn_game_over();
        void check_clear();
        void copy_board(int8_t dest[ROWS][COLUMNS], const int8_t src[ROWS][COLUMNS]);
};
#endif // !boardh
