# Python 2 Player Tetris Library implemented in C++


Original title: Tetris with instant soft drop delay(branch python library)

Following tetris guidelines, implemented in C++ for speed, actions are taken simultaneously, players turn-based with matching keys instead piece, can be used to train AI/whatever. ~~with instant softdrop after specified time and short softdrop like DAS~~ (not implemented in python lib yet)

Separated from my tetris/pylibtest

# Usage Brief view
```python
import tetris

x=tetris.Container() # game object
y=x.copy() # deep copy if needed
x.reset() # reset state
x.seed_reset(n) # reset with rng seed
state=x.get_state() # shape (500,) numpy array, 1D so easier to use mp shared memory, more info below
shapes=x.get_shapes() # (7,4,4,4) 7 pieces 4 rotations and 4x4 shape 

x.step(ac1,ac2)
```

# Render
Switch to my tetris\_c library and install SDL2 below if manual compile is needed. 

# In Depth Info
## Game Step

action is scalar int
```
0:hold
1:hard drop
2:cw
3:ccw
4:left tap
5:right tap
6:left das
7:right das
8:soft drop
9:180
```
## States
```python
p1_state=state[0:232]
p2_state=state[232:464]
```
sum attack `state[464]`, negative is to p1
individual attacks `state[465:]`
## Reward
state[0] is reward

127 is lose, 126 is win, otherwise 

```
if harddrop (action==1)
    if lines_cleared
        rew=10 * (attack+ b2b *2)+2*lines
    rew+=b2b_not_broke
else rew=wallkick
```
Python for extracting
```python
lines_cleared = (reward%10)//2
atk = reward//10 # b2b rewards 2 atk but is 1 in-game
wallkick=reward%5 if action!=1 else 0
b2b_not_broken=reward%5 if action==1 else 0
```

game doesn't auto reset on game over, check those signals and call reset on your own

## States
i piece goes to -2 so the value is +2 by default, if you want to draw current piece subtract that back

`x = state[1]`

Board is 30 high, bottom 21 is returned, y spawns at 9 normally but 8 if occupied, otherwise game over.\
Value returned to python starts at 8 so pieces spawn at 1 normally, could be negative if you navigate the piece off screen

`y = state[2]`

Distance to bottom

`softdropdist = state[3]`

Default is 10 actions will force harddrop

`action_count = state[4]`

Garbage cleared from last action, use for reward etc

`garbage = state[5]`

`hold_used = state[6]`

`rotation = state[7]`

`active = state[8]`

Draw active piece on current location(you have to check x negative yourself):

`board[x-2,y]=shape[active][rotation]`

Held can be -1 meaning not used

`held_piece = state[9]`

`next_pieces = state[10:15]`

Cheat data(hidden queue)

`hidden_queue=state[15:22]`

Less cheating because you can manually count the bag:

`hidden_queue=sort(state[15:22])`


Board `state[22:232].reshape(21,10)`

# Feedback
Any ideas for improvements are welcome. 

State is mostly boolean/binary so it's possible I can optimise the code further to increase speed as use cases probably will be AI training that needs millions of iterations. 

if you want to read/change the code, tetrisboard is main logic including clear rewards, and env.cpp is the python wrapper+rendering+returning state
