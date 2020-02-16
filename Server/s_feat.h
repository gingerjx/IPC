#ifndef S_FEAT_H_INCLUDED
#define S_FEAT_H_INCLUDED
#include <semaphore.h>
#include <pthread.h>
#include "global.h"

server_t * init_server_shm(int *serv_fd, char *serv_sh_name, int id);
client_t * init_client_shm(int *cl_fd, char *cl_sh_name, int id);

void *join_client(void *p); 
void *input_thread(void *p); 
int input_react(char *input, board_t *board, beast_t *beasts, int *beasts_num); //TODO

void join_client_to_game(board_t *board, server_t *serv_msg, client_t *cl_msg);
void join_client_to_game_data(server_t*serv_msg);
void reset_client(board_t *board, server_t *serv_msg);
void reset_client_data(server_t *serv_msg);

void init_clients(server_t *serv_msg[MAX_CLIENTS]);      //TODO  
void rand_spawn(server_t *serv_msg, board_t *board);
void update_render(server_t *serv_msg, board_t *board);

void init_col();
board_t *load_board(const char *filename);
void rand_new(board_t *board, char c);

void displayBoard(board_t *board);
void display_legend(int begin);
void display_info(int rows, int c_x, int c_y);

void close_connections(int serv_fd[MAX_CLIENTS], int cl_fd[MAX_CLIENTS], server_t *serv_msg[MAX_CLIENTS], 
        client_t *cl_msg[MAX_CLIENTS], char serv_sh_name[MAX_CLIENTS][25], char cl_sh_name[MAX_CLIENTS][25], 
        char *sem_name, sem_t *sem_crit, pthread_t ptJ[MAX_CLIENTS], pthread_t ptIn, board_t *board);       //TODO
void free_maze(char **board, int num);

#endif