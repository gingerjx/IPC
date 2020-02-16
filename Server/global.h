#ifndef GLOBAL_H_INCLUDED
#define GLOBAL_H_INCLUDED
#include <unistd.h>
#include <semaphore.h>

#define SEC 1000000
#define SERV_USLEEP SEC*0.1
#define MAX_CLIENTS 4
#define MAX_BEAST 6
#define MAX_BOARD 70
#define RENDER_SIZE 5
#define SHIFTX 2
#define SHIFTY 6
#define COIN 30
#define LITTLE_TRES 10
#define BIG_TRES 5

#define DEBUG 1
#if DEBUG
  #define log(...)  printf(__VA_ARGS__);
#else 
  #define log(...)  printf("");
#endif

enum msg_e {MSG, NO_MSG};
enum client_type_e {PLAYER, BOT};
enum status_e {OKEY, WALL_COL, PLAYERS_COL, BEAST_COL, BUSH_COL, CLOSE};
enum move_e {UP,LEFT,DOWN,RIGHT,NONE};
enum procces_e {CLOSED,RUNNING};

typedef struct {
    int rows;
    int cols;
    char **b;
} board_t;
typedef struct {
    enum status_e client_status;
    pid_t serv_pid;
    int id;
    int joined;
    int x,y;
    int respX, respY;
    int carry;
    int score;
    int deaths;
    int brought;
    int sleep_time;
    unsigned long long request_nr;
    char tab[RENDER_SIZE][RENDER_SIZE];     
} server_t;
typedef struct {
    enum client_type_e type;
    enum move_e move;
    enum procces_e procces_status;
    int id;
    pid_t pid;
    unsigned long long request_nr;
} client_t;
typedef struct {
    server_t *serv_msg;
    client_t *cl_msg;
    board_t *board;
    sem_t *sem_crit;
} s_for_join_t;
typedef struct {
    board_t *board;
    char *input;
} s_for_join_beast_t;
typedef struct {
    int x;
    int y;
    int id;
} beast_t;
#endif