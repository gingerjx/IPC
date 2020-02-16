#ifndef S_BEAST_H_INCLUDED
#define S_BEAST_H_INCLUDED
#include "global.h"

void add_beast(board_t *board, beast_t *beast, int *beasts_num);

void move_beast(board_t *board, beast_t *beast);

void clear_position(board_t *board, beast_t *beast);
void mv_b_to_empty(board_t *board, beast_t *beast, int next_x, int next_y);
void mv_b_to_bush(board_t *board, beast_t *beast, int next_x, int next_y);
void mv_b_to_money(board_t *board, beast_t *beast, int next_x, int next_y);

int find_player(board_t *board, int Bx, int By, int *Px, int *Py);
int visible(board_t *board, int Bx, int By, int Px, int Py);
int adjacent(board_t *board, int Bx, int By, int Px, int Py);

int mv_to_player(board_t *board, beast_t *beast, int Px, int Py);
void calculate_distance(board_t *board, int Bx, int By, int Px, int Py, double dist[4]);

#endif