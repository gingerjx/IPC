#ifndef H_FEAT_H_INCLUDED
#define H_FEAT_H_INCLUDED
#include <semaphore.h>
#include <pthread.h>
#include "../Server/global.h"

server_t * init_server_shm(int *serv_fd, char *serv_sh_name, int id);        
client_t * init_client_shm(int *cl_fd, char *cl_sh_name, int id);

void *input_thread(void *p); 

void init_client(client_t *cl_msg, int id, char *type);
void init_board(char board[MAX_BOARD][MAX_BOARD]);
void display_legend();
void display_info(server_t *serv_msg, client_t *cl_msg);

void init_col();
void clear_enemies_moves(char board[MAX_BOARD][MAX_BOARD]);
void update_board(char board[MAX_BOARD][MAX_BOARD], server_t*serv_msg);
void display_camp_info(int x, int y);
void update_info(server_t *serv_msg);

int move_me(int input, client_t *cl_msg);

void close_connections(int serv_fd, int cl_fd, server_t *serv_msg, client_t *cl_msg, 
                sem_t *sem_crit, sem_t *sem_join, pthread_t ptIn);
#endif
