#ifndef C_FEAT_H_INCLUDED
#define C_FEAT_H_INCLUDED
#include <semaphore.h>
#include <pthread.h>
#include "../Server/global.h"

server_t * init_server_shm(int *serv_fd, char *serv_sh_name, int id);        
client_t * init_client_shm(int *cl_fd, char *cl_sh_name, int id);

void init_client(client_t *cl_msg, int id, char *type);
void init_board(char board[MAX_BOARD][MAX_BOARD]);
void display_legend();
void display_info(server_t *serv_msg, client_t *cl_msg);

void init_col();
void clear_enemies_moves(char board[MAX_BOARD][MAX_BOARD]);
void update_board(char board[MAX_BOARD][MAX_BOARD], server_t *serv_msg);
void display_camp_info(int x, int y);
void update_info(server_t *serv_msg);

void move_me(client_t *cl_msg, server_t *serv_msg);

int find_beast(const char tab[RENDER_SIZE][RENDER_SIZE], int *Px, int *Py);
int visible(const char tab[RENDER_SIZE][RENDER_SIZE], int Bx, int By, int Px, int Py);
int adjacent(const char tab[RENDER_SIZE][RENDER_SIZE], int Bx, int By, int Px, int Py);

int reaction_on_beast(const char tab[RENDER_SIZE][RENDER_SIZE], int Bx, int By, int Px, int Py);
void calculate_distance(const char tab[RENDER_SIZE][RENDER_SIZE], int Bx, int By, int Px, int Py, double dist[4]);

void close_connections(int serv_fd, int cl_fd, server_t *serv_msg, client_t *cl_msg, 
                sem_t *sem_crit, sem_t *sem_join);

#endif
