#define PY_SSIZE_T_CLEAN
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <Python.h>
#include <numpy/arrayobject.h>

#ifndef tb
#define tb
#include "board.h"
#endif // !tb


#ifndef io
#define io
#include <iostream>
#endif // !io
#ifdef RENDER
#include <SDL.h>
#include <chrono>
#endif
//#include <tuple>
#include <vector>
#include <sstream>
class game_server;
class game_client;

class game_container {
    public:
        class game_server {
            public:
                int stored_attack=0;
                std::vector<int8_t> attack_queue{};
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
                std::vector<int8_t> receive(int port) {
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
                    else receive(server->receive(port));
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

                PyObject* serialize() const {
                    // Serialize the state of the game client
                    PyObject* serialized = PyTuple_New(5);

                    // Serialize the variables from the game base class
                    npy_intp dims[1] = {17};
                    PyObject* np_variables = PyArray_SimpleNew(1, dims, NPY_INT8);
                    int8_t* np_variables_data = (int8_t*)PyArray_DATA((PyArrayObject*)np_variables);
                    int8_t size=(int8_t) hidden_queue.size();  
                    np_variables_data[0] = (int8_t)game_over;
                    np_variables_data[1] = (int8_t)cleared;
                    np_variables_data[2] = (int8_t)active;
                    np_variables_data[3] = (int8_t)rotation;
                    np_variables_data[4] = (int8_t)x;
                    np_variables_data[5] = (int8_t)y;
                    np_variables_data[6] = (int8_t)received;
                    np_variables_data[7] = (int8_t)held_piece;
                    np_variables_data[8] = (int8_t)hold_used;
                    np_variables_data[9] = (int8_t)b2b;
                    np_variables_data[10] = (int8_t)attack;
                    np_variables_data[11] = (int8_t)combo;
                    np_variables_data[12] = (int8_t)gheight;
                    np_variables_data[13] = (int8_t)garbage;
                    np_variables_data[14] = (int8_t)spin;
                    np_variables_data[15] = (int8_t)kick;
                    np_variables_data[16] =size;
                    PyTuple_SetItem(serialized, 0, np_variables);

                    npy_intp dims_queue[1] = {5};
                    PyObject* np_queue = PyArray_SimpleNew(1, dims_queue, NPY_INT8);
                    int8_t* np_queue_data = (int8_t*)PyArray_DATA((PyArrayObject*)np_queue);

                    // Populate np_queue_data with values from queue directly
                    std::copy(queue, queue + 5, np_queue_data);
                    PyTuple_SetItem(serialized, 1, np_queue);
                    PyArray_ENABLEFLAGS((PyArrayObject*)np_queue, NPY_ARRAY_OWNDATA);
                    // Serialize the board as a 2D NumPy array
                    npy_intp dims_board[2] = {ROWS, COLUMNS};
                    PyObject* np_board = PyArray_SimpleNew(2, dims_board, NPY_INT8);
                    int8_t* np_board_data = (int8_t*)PyArray_DATA((PyArrayObject*)np_board);

                    // Copy data from the 2D board array into the NumPy array row by row
                    for (int i = 0; i < ROWS; ++i) {
                        const int8_t* board_row_data = board[i]; // Pointer to the current row in the board array
                        int8_t* np_board_row_data = np_board_data + i * COLUMNS; // Pointer to the corresponding row in the NumPy array
                        std::copy(board_row_data, board_row_data + COLUMNS, np_board_row_data);
                    }

                    // Set the OWNDATA flag to indicate that np_board owns its data
                    PyArray_ENABLEFLAGS((PyArrayObject*)np_board, NPY_ARRAY_OWNDATA);

                    // Set np_board in the serialized tuple
                    PyTuple_SetItem(serialized, 2, np_board);

                    // Create a NumPy array to hold hidden_queue data
                    npy_intp hidden_queue_dims[1] = {size};
                    PyObject* np_hidden_queue = PyArray_SimpleNew(1, hidden_queue_dims, NPY_INT8);
                    int8_t* np_hidden_queue_data = (int8_t*)PyArray_DATA((PyArrayObject*)np_hidden_queue);
                    // Copy hidden_queue data into NumPy array
                    std::copy(hidden_queue.begin(), hidden_queue.end(), np_hidden_queue_data);
                    PyArray_ENABLEFLAGS((PyArrayObject*)np_hidden_queue, NPY_ARRAY_OWNDATA);
                    PyTuple_SetItem(serialized, 3, np_hidden_queue);


                    std::stringstream ss;

                    // Serialize the state into the stringstream
                    ss << gen;

                    // Convert the stringstream to a Python bytes object
                    std::string data = ss.str();
                    std::stringstream ss2;

                    // Serialize the state into the stringstream
                    ss2 << gen2;

                    // Convert the stringstream to a Python bytes object
                    std::string data2 = ss2.str();
                    // Create a tuple to store the NumPy arrays
                    PyObject* gen_states = PyTuple_New(2);
                    PyTuple_SetItem(gen_states, 0, PyBytes_FromStringAndSize(data.data(), data.size()));
                    PyTuple_SetItem(gen_states, 1,PyBytes_FromStringAndSize(data2.data(), data2.size()));
                    // Return a tuple containing both lists
                    PyTuple_SetItem(serialized, 4,gen_states);
                    return serialized;
                }
                int deserialize(PyObject* serialized) {
                    if (!PyTuple_Check(serialized)) {
                        PyErr_SetString(PyExc_TypeError, "Serialized data must be a tuple");
                        return -1;  // Return -1 to indicate failure
                    }

                    // Check if the tuple has the expected number of elements
                    if (PyTuple_Size(serialized) != 5) {
                        PyErr_SetString(PyExc_ValueError, "Serialized data tuple must contain variables tuple, queue array, and board array");
                        return -1;  // Return -1 to indicate failure
                    }

                    // Extract variables tuple, queue array, and board array from serialized data
                    PyObject* variables_tuple = PyTuple_GetItem(serialized, 0);
                    PyObject* queue_array = PyTuple_GetItem(serialized, 1);
                    PyObject* board_array = PyTuple_GetItem(serialized, 2);
                    PyObject* hidden_queue_array = PyTuple_GetItem(serialized, 3);
                    PyObject* gen_states = PyTuple_GetItem(serialized, 4);

                    // Check if variables tuple is a NumPy array
                    if (!PyArray_Check(variables_tuple)) {
                        PyErr_SetString(PyExc_TypeError, "Variables tuple must be a NumPy array");
                        return -1;  // Return -1 to indicate failure
                    }

                    // Check if queue array is a NumPy array
                    if (!PyArray_Check(queue_array)) {
                        PyErr_SetString(PyExc_TypeError, "Queue array must be a NumPy array");
                        return -1;  // Return -1 to indicate failure
                    }

                    // Check if board array is a NumPy array
                    if (!PyArray_Check(board_array)) {
                        PyErr_SetString(PyExc_TypeError, "Board array must be a NumPy array");
                        return -1;  // Return -1 to indicate failure
                    }

                    // Check the size of the variables tuple
                    PyArrayObject* np_variables = (PyArrayObject*)variables_tuple;
                    if (PyArray_SIZE(np_variables) != 17) {
                        PyErr_SetString(PyExc_ValueError, "Variables tuple must have 16 elements");
                        return -1;  // Return -1 to indicate failure
                    }

                    // Check the size of the queue array
                    PyArrayObject* np_queue = (PyArrayObject*)queue_array;
                    if (PyArray_SIZE(np_queue) != 5) {
                        PyErr_SetString(PyExc_ValueError, "Queue array must have 5 elements");
                        return -1;  // Return -1 to indicate failure
                    }

                    // Check the size of the board array
                    PyArrayObject* np_board = (PyArrayObject*)board_array;
                    if (PyArray_DIM(np_board, 0) != ROWS || PyArray_DIM(np_board, 1) != COLUMNS) {
                        PyErr_SetString(PyExc_ValueError, "Board array must have dimensions ROWS x COLUMNS");
                        return -1;  // Return -1 to indicate failure
                    }
                    if (!PyTuple_Check(gen_states) || PyTuple_Size(gen_states) != 2) {
                        PyErr_SetString(PyExc_TypeError, "Invalid gen data: expected a tuple of size 2");
                        return -1;
                    }

                    // Extract variables from the tuple
                    int8_t* np_variables_data = (int8_t*)PyArray_DATA(np_variables);

                    this->game_over = (int)np_variables_data[0];
                    this->cleared = (int)np_variables_data[1];
                    this->active = (int)np_variables_data[2];
                    this->rotation = (int)np_variables_data[3];
                    this->x = (int)np_variables_data[4];
                    this->y = (int)np_variables_data[5];
                    this->received = (int)np_variables_data[6];
                    this->held_piece = (int)np_variables_data[7];
                    this->hold_used = (bool)np_variables_data[8];
                    this->b2b = (bool)np_variables_data[9];
                    this->attack = (int)np_variables_data[10];
                    this->combo = (int)np_variables_data[11];
                    this->gheight = (int)np_variables_data[12];
                    this->garbage = (int)np_variables_data[13];
                    this->spin = (bool)np_variables_data[14];
                    this->kick = (bool)np_variables_data[15];

                    int8_t* qdata = (int8_t*)PyArray_DATA(np_queue);
                    // Extract queue from the variables tuple
                    for (int i = 0; i < 5; ++i) {
                        this->queue[i] = (int)qdata[i];
                    }

                    // Extract board data
                    int8_t* np_board_data = (int8_t*)PyArray_DATA(np_board);
                    for (int i = 0; i < ROWS; ++i) {
                        for (int j = 0; j < COLUMNS; ++j) {
                            this->board[i][j] = (int)np_board_data[i * COLUMNS + j];
                        }
                    }

                    int8_t hidden_queue_size =np_variables_data[14];

                    // Extract hidden_queue data from the separate NumPy array
                    PyArrayObject* np_hidden_queue = (PyArrayObject*)hidden_queue_array;
                    int8_t* np_hidden_queue_data = (int8_t*)PyArray_DATA(np_hidden_queue);
                    this->hidden_queue.assign(np_hidden_queue_data, np_hidden_queue_data + hidden_queue_size);

                    std::stringstream ss;

                    PyObject* state1 = PyTuple_GetItem(gen_states, 0);
                    PyObject* state2 = PyTuple_GetItem(gen_states, 1);
                    // Convert the Python object to a stringstream
                    PyObject* pyBytes = PyBytes_FromObject(state1);
                    const char* bytes = PyBytes_AsString(pyBytes);
                    ss.write(bytes, PyBytes_Size(state1));

                    // Extract the state from the stringstream into the mt19937 engine
                    ss >> gen;
                    pyBytes = PyBytes_FromObject(state1);
                    bytes = PyBytes_AsString(pyBytes);
                    ss.write(bytes, PyBytes_Size(state2));

                    // Extract the state from the stringstream into the mt19937 engine
                    ss >> gen2;
                    // Don't forget to release the Python bytes object
                    Py_DECREF(pyBytes);

                    return 0;
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
                std::cerr << "Exception during initialization: " << e.what() << std::endl;
                return -1;
            }

            return 0;
        }

        static void dealloc(game_container* self) {
            if (self->server != nullptr) {
                delete self->server;
            }
            if (self->clients[0] != nullptr) {
                delete self->clients[0];
            }
            if (self->clients[1] != nullptr) {
                delete self->clients[1];
            }
            //delete self;
            //Py_TYPE(self)->tp_free((PyObject*)self);

        }
        static PyObject* reduce(game_container* self) {
            PyObject* t = PyTuple_New(2);
            PyTuple_SetItem(t, 0, PyBool_FromLong(self->server->stored_attack < 0));
            npy_intp size = self->server->attack_queue.size();
            PyObject* a = PyArray_SimpleNewFromData(1, &size, NPY_INT8, self->server->attack_queue.data());
            PyTuple_SetItem(t,1,a);


            PyObject* args = PyTuple_New(0);  

            PyObject* state = PyTuple_New(3);
            PyTuple_SetItem(state, 0, t);
            PyTuple_SetItem(state, 1, self->clients[0]->serialize()); 
            PyTuple_SetItem(state, 2, self->clients[1]->serialize());  

            // Return a tuple containing the callable object and its arguments
            PyObject* result = PyTuple_Pack(3, Py_TYPE(self), args, state);

            // Decrement reference counts to avoid memory leaks
            Py_XDECREF(args);
            Py_XDECREF(state);
            return result;
        }
        static PyObject* set_state(game_container* self, PyObject* state) {
            // Check if the state argument is a tuple
            if (!PyTuple_Check(state)) {

                PyErr_Format(PyExc_TypeError, "Invalid state: expected a tuple");

                return NULL;  
            }

            
            if(PyTuple_Size(state) != 3) {
                PyErr_Format(PyExc_ValueError, "Invalid state: expected a tuple of size 2, got tuple of size %d", PyTuple_Size(state));
                return NULL;  
            }
            
            PyObject* server_state = PyTuple_GetItem(state, 0);
            self->server->stored_attack=PyLong_AsLong(PyTuple_GetItem(server_state, 0));
            PyObject* np_array = PyTuple_GetItem(server_state, 1);
            npy_intp* dims = PyArray_DIMS((PyArrayObject*)np_array);
            int size = (int)dims[0];
            int8_t* attack_queue_data = (int8_t*)PyArray_DATA((PyArrayObject*)np_array);
            self->server->attack_queue.assign(attack_queue_data, attack_queue_data + size);


            
            PyObject* client_state = PyTuple_GetItem(state, 1);
            if (self->clients[0]->deserialize(client_state) == -1) {
                return NULL;  
            }

            PyObject* client_state2 = PyTuple_GetItem(state, 2);
            if (self->clients[1]->deserialize(client_state2) == -1) {
                return NULL;  
            }


            Py_RETURN_NONE;

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
        static PyObject* get_state(game_container* self, PyObject * args) {
            /*
            int x = 0;
            if (!PyArg_ParseTuple(args, "|i", &x))
                return NULL;
            if(x!=1 and x!=2 and x!=0){
                return NULL;
            }
            */
            const npy_intp dim[1] = {500};
            PyObject* ret=PyArray_ZEROS(1,dim,NPY_INT8,0);
            int8_t* state = (int8_t*)PyArray_DATA((PyArrayObject*)ret);
            if (self->clients[0]->game_over|| self->clients[1]->game_over||
                self->server->stored_attack>30||self->server->stored_attack<-30){
                if (self->clients[0]->game_over&&self->clients[1]->game_over)
                {
                    state[0] = 127;
                    state[232] = 127;
                }
                else if (self->clients[0]->game_over||self->server->stored_attack<-30) {
                    state[0] = 127;
                    state[232] = 126;

                }
                else {
                    state[232 + 0] = 127;
                    state[0] = 126;
                }
                PyArray_ENABLEFLAGS((PyArrayObject*)ret, NPY_ARRAY_OWNDATA);
                return ret;
            }
            else {
                if (self->clients[0]->last_invalid) {
                    state[0] = 125;
                }
                else {
                    state[0] = self->clients[0]->cleared + self->clients[0]->spin;
                }
                if (self->clients[1]->last_invalid) {
                    state[232] = 125;
                }
                else {
                    state[232 + 0] = self->clients[1]->cleared + self->clients[1]->spin;
                }
            }
            state[1] = self->clients[0]->x+2;
            state[2] = self->clients[0]->y-8;
            state[3] = self->clients[0]->softdropdist();
            state[4] = self->clients[0]->rotation;

            state[5] = self->clients[0]->garbage;
            state[6] = self->clients[0]->hold_used;
            state[7] = self->clients[0]->action_count;
            state[8] = self->clients[0]->active;
            state[9] = self->clients[0]->held_piece;
            for (size_t i = 0; i < 5; i++)
            {
                state[i + 10] = self->clients[0]->queue[i];
            }
            for (size_t i = 0; i < self->clients[0]->hidden_queue.size(); i++)
            {
                state[i + 15] = self->clients[0]->hidden_queue[i];
            }
            for (size_t i = 0; i < 21; ++i) {
                for (size_t j = 0; j < 10; ++j) {
                    state[i * 10 + j + 22] = self->clients[0]->board[i + 9][j] + 1 > 0;
                }
            }
            state[232 + 1] = self->clients[0]->x+2;
            state[232 + 2] = self->clients[0]->y-8;
            state[232 + 3] = self->clients[0]->softdropdist();
            state[232 + 4] = self->clients[1]->rotation;

            
            state[232 + 5] = self->clients[1]->gheight;
            state[232 + 6] = self->clients[1]->hold_used;
            state[232 + 7] = self->clients[1]->action_count;
            state[232 + 8] = self->clients[1]->active;
            state[232 + 9] = self->clients[1]->held_piece;
            for (size_t i = 0; i < 5; i++)
            {
                state[232 + i + 10] = self->clients[1]->queue[i];
            }
            for (size_t i = 0; i < self->clients[1]->hidden_queue.size(); i++)
            {
                state[i + 247] = self->clients[1]->hidden_queue[i];
            }
            for (size_t i = 0; i < 21; ++i) {
                for (size_t j = 0; j < 10; ++j) {
                    state[i * 10 + j + 254] = self->clients[0]->board[i + 9][j] + 1 > 0;
                }
            }

            state[464]=self->server->stored_attack;
            std::copy(self->server->attack_queue.begin(),self->server->attack_queue.end(),state+465);

            PyArray_ENABLEFLAGS((PyArrayObject*)ret, NPY_ARRAY_OWNDATA);
            return ret;
        }
        static PyObject* piecedef(game_container* self, PyObject* Py_UNUSED) {
            npy_intp piecedef_dims[4] = {7, 4, 4, 4};

            // Create a NumPy array to hold the piecedef data
            PyObject* np_piecedef = PyArray_SimpleNew(4, piecedef_dims, NPY_INT8);
            int8_t* np_piecedef_data = (int8_t*)PyArray_DATA((PyArrayObject*)np_piecedef);

            for (size_t i = 0; i < 7; i++)
            {
                for (size_t j = 0; j < 4; j++)
                {
                    for (size_t k = 0; k < 4; k++)
                    {
                        for (size_t l = 0; l < 4; l++)
                        {
                            np_piecedef_data[i *64 + j * 16 + k*4+l] = ((self->clients[0]->piecedefs[i][j][k][l] + 1) > 0);
                        }
                    }
                }
            }
            return np_piecedef;
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

static PyMethodDef gc_methods[] = {
    {"seed_reset", (PyCFunction)game_container::seed_reset, METH_VARARGS, "Seed and reset"},
    {"reset", (PyCFunction)game_container::reset, METH_NOARGS, "Reset game"},
    {"get_state", (PyCFunction)game_container::get_state, METH_NOARGS, "Get game state"},
    {"__reduce__", (PyCFunction)game_container::reduce, METH_NOARGS,
        "Reduce function for pickling"},
    {"get_shapes", (PyCFunction)game_container::piecedef, METH_NOARGS,
        "Get 7 pieces shapes, SZLJTOI"},
    {"__setstate__", (PyCFunction)game_container::set_state, METH_O,
        "Set state function for unpickling"},
    {"step", (PyCFunction)game_container::step, METH_VARARGS, "Step game, two inputs for each board"},
    {"copy", (PyCFunction)game_container::copy, METH_NOARGS, "Copy game state"},
    {NULL, NULL, 0, NULL} // Sentinel
};
static PyTypeObject game_container_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
        .tp_name = "tetris.Container",
    .tp_basicsize = sizeof(game_container),
    .tp_itemsize = 0,
    .tp_dealloc = (destructor)game_container::dealloc,
    .tp_flags = Py_TPFLAGS_BASETYPE,//|Py_TPFLAGS_HEAPTYPE,
    .tp_doc = "game_container objects\ninit(no arg):new game state\ninit(state,hidden_queue length, hidden queue,attack length, stored attacks)",
    .tp_methods = gc_methods,
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

#ifdef RENDER
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
            //Py_TYPE(self)->tp_free((PyObject*)self);
            //delete self;
            SDL_Quit();
        }

        bool window_opened = false;
        static int init(game_renderer* self, PyObject* args) {
            int render_mode = -1;
            int render_size=-1;
            float fps=-1;
            if (!PyArg_ParseTuple(args, "|iii", &render_mode, &render_size,&fps)) {
                return NULL;
            }
            game_renderer* r;
            if (render_mode == -1 && render_size == -1)
                r = new (self) game_renderer();
            else
                r = new (self) game_renderer(render_mode, render_size);
            if (fps > 0) {
                r->frameDuration = std::chrono::milliseconds((int)(1000/fps));

            }

            return 0;
        }
        static PyObject* create_window(game_renderer* self, PyObject* args) {
            int mode = 1;
            int size = 30;
            float fps=-1;
            if (!PyArg_ParseTuple(args, "|iii", &mode, &size,&fps)) {
                return NULL;
            }
            self->c_set_size(mode, size);
            self->c_create_window();
            if (fps > 0) {
                self->frameDuration = std::chrono::milliseconds((int)(1000/fps));
            }
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
        SDL_Window* window=nullptr;
        SDL_Renderer* renderer = nullptr;
        std::chrono::steady_clock::time_point last=std::chrono::steady_clock::now();
        std::chrono::duration<float, std::milli> frameDuration= std::chrono::milliseconds(100);
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

            int width = (block_size + 1) * 100 / 3 + 2;
            int height = 20.5f * (block_size + 1);
            BOARDX = width / 10;
            bg.y = 0; bg.w = width * 3 / 10; bg.h = height;
            rect.w = block_size; rect.h = block_size;
            red_line.w = (int)(block_size / 10), red_line.h = block_size + 1;
            red_line_small.w = (int)(block_size / 10), red_line_small.h = block_size;
            if (!this->window) {

                this->window = SDL_CreateWindow("Tetris", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
            }
            if (!this->window) {
                std::cerr << "SDL window creation failed: " << SDL_GetError() << std::endl;
                return;
            }
            if (!this->renderer) {

                this->renderer = SDL_CreateRenderer(this->window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
            }
            if (!this->renderer) {
                std::cerr << "SDL renderer creation failed: " << SDL_GetError() << std::endl;
                return;
            }

            SDL_SetRenderDrawBlendMode(this->renderer, SDL_BLENDMODE_BLEND);
            this->window_opened = true;
        }

        void c_close() {
            if (this->renderer!=nullptr) {
                SDL_DestroyRenderer(this->renderer);
                this->renderer = nullptr;

            }
            if (this->window != nullptr) {
                SDL_DestroyWindow(this->window);
                this->window = nullptr;

            }
            window_opened = false;

        }
        void color_from_rgb(int32_t v) {
            SDL_SetRenderDrawColor(renderer, (v >> 16) & 0xFF, (v >> 8) & 0xFF, v & 0xFF, 0xFF);
        }
        void rgba_from_rgb(int32_t v) {
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
            //if (active_piece)
            //{
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
            /*}
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
            }*/

            //ghost
            //if (ghost) {
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
            //}
        }
        void draw_atk(int side, std::vector<int8_t> attacks) {
            red_line.x = BOARDX*4 + 11 * (block_size + 1) + side * BOARDX * 5;
            int sum = 0;

            SDL_SetRenderDrawColor(renderer,255,0,0,255);
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
PyObject* game_renderer::render(game_renderer* self, PyObject* args) {

    PyObject* pyg;
    if (!PyArg_ParseTuple(args, "O", &pyg)) {
        PyErr_SetString(PyExc_TypeError, "Can't parse.");
        return NULL;
    }
    if (!PyObject_IsInstance(pyg, (PyObject*)&game_container_type)) {
        PyObject* obj_type = PyObject_Type(pyg);
        if (obj_type != NULL) {
            const char* type_name = Py_TYPE(obj_type)->tp_name;
            PyErr_Format(PyExc_TypeError, "Expected a game_container instance, got %s.", type_name);
            Py_DECREF(obj_type);
        } else {
            PyErr_SetString(PyExc_TypeError, "Expected a game_container instance.");
        }
        return NULL;
    }/*if (!PyObject_IsInstance(pyg, (PyObject*) & game_container_type)) {
       PyErr_SetString(PyExc_TypeError, "Expected a game_container instance.");
       return NULL;
       }*/
    const game_container* g = reinterpret_cast<const game_container*>(pyg);
    if (!g) {
        PyErr_SetString(PyExc_RuntimeError, "Failed to retrieve game_container instance from the capsule.");
        return NULL;
    }

    auto n=std::chrono::steady_clock::now();
    if (self->window_opened)
    {
        if(n-self->last>self->frameDuration){
            self->c_render(*g);
            SDL_PollEvent(&self->event);
            if (self->event.type == SDL_QUIT) {
                self->c_close();
                return PyBool_FromLong(1);
            }
            self->last=n;
        }
    }
    else {
        std::cerr << "No Window open!\nUse create_window(render_mode,render_size) to create a window\n";
        return PyBool_FromLong(1);
    }

    return PyBool_FromLong(0);
}


static PyTypeObject game_renderer_type = {
    PyVarObject_HEAD_INIT(NULL, 0)
        "tetris.Renderer",  // name of the type
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

#endif
static PyMethodDef Methods[] = {
    /*{"make",  make, METH_VARARGS,
      "Makes and returns the game container. "},
      {"render",  render, METH_VARARGS,
      "Creates a renderer, returns renderer. "},*/
    {NULL, NULL, 0, NULL}
};
static struct PyModuleDef Module = {
    PyModuleDef_HEAD_INIT,
    "tetris",   /* name of module */
    NULL,
    -1,
    Methods
};
    PyMODINIT_FUNC
PyInit_tetris(void)
{
    import_array();
    PyObject* m= PyModule_Create(&Module);
    if (m == NULL)
        return NULL;

    if (PyType_Ready(&game_container_type) != 0)
        return NULL;

    Py_INCREF(&game_container_type);
    PyModule_AddObject(m, "Container", (PyObject*)&game_container_type);
    #ifdef RENDER
    if (PyType_Ready(&game_renderer_type) < 0)
        return NULL;
    Py_INCREF(&game_renderer_type);
    PyModule_AddObject(m, "Renderer", (PyObject*)&game_renderer_type);
    #endif
    return m;
}
int main(int argc, char* argv[]) {

    return 0;
}
