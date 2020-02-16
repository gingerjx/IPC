#ifndef S_PLAYER_H_INCLUDED
#define S_PLAYER_H_INCLUDED
#include "global.h"

int check_move(board_t *board, server_t *serv_msg, client_t *cl_msg, server_t *all_serv_msg[MAX_CLIENTS]);

void move_to_empty(char **board, int x, int y, server_t *serv_msg);
void move_to_bush(char **board, int x, int y, server_t *serv_msg);
void move_to_money(board_t *board, int x, int y, server_t *serv_msg, char money);
void move_to_camp(int rows, server_t *serv_msg);

void collision_players(board_t *board, server_t *serv_msg_1, server_t *serv_msg_2);
void collision_beast(board_t *board, server_t *serv_msg);

#endif