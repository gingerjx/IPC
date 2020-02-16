#include <ncurses.h>
#include <stdlib.h>
#include <math.h>
#include "global.h"
#include "s_beast.h"

void add_beast(board_t *board, beast_t *beast, int *beasts_num){
    if ( *beasts_num >= MAX_BEAST ){
        mvprintw(1,0,"Cannot join more beasts");
        return;
    }
    
    int x = rand()%(board->rows);
    int y = rand()%(board->cols);
    while ( board->b[x][y] != ' ' ){
        x = rand()%(board->rows);
        y = rand()%(board->cols);
    }
    
    board->b[x][y] = '*';
    beast[*beasts_num].x = x;
    beast[*beasts_num].y = y;
    beast[*beasts_num].id = *beasts_num;
    (*beasts_num)++;
    
    attron(COLOR_PAIR(6));
    mvprintw(x+SHIFTX,y+SHIFTY,"*");
    attron(COLOR_PAIR(5));
    mvprintw(1,0,"[ %d ]Beast joins", *beasts_num);
}

void move_beast(board_t *board, beast_t *beast){
    int px, py;
    if ( find_player(board,beast->x,beast->y,&px,&py) && visible(board,beast->x,beast->y,px,py) ){
        mv_to_player(board,beast,px,py);    //doesnt works TODO
    } else {
    enum move_e mv = rand()%4;
        int dirX[] = {-1,0,1,0};
        int dirY[] = {0,-1,0,1};
        int next_x = beast->x + dirX[mv];
        int next_y = beast->y + dirY[mv];
        char c = board->b[next_x][next_y];
        
        if ( c == ' ' )
            mv_b_to_empty(board,beast,next_x,next_y);
        else if ( c == '#' )
            mv_b_to_bush(board,beast,next_x,next_y);
        else if ( c == 'c' || c == 't' || c == 'T' || c == 'D' )
            mv_b_to_money(board,beast,next_x,next_y);
    }
}

void clear_position(board_t *board, beast_t *beast){
    int x = beast->x;
    int y = beast->y;
    if ( board->b[x][y] == '*' ){
        board->b[x][y] = ' ';
        mvprintw(x+SHIFTX,y+SHIFTY," ");
    } else if ( board->b[x][y] == '#' )
        mvprintw(x+SHIFTX,y+SHIFTY,"#");
    else {
        attron(COLOR_PAIR(7));
        mvprintw(x+SHIFTX,y+SHIFTY,"%c", board->b[x][y] );      
        attron(COLOR_PAIR(5));        
    }    
}
void mv_b_to_empty(board_t *board, beast_t *beast, int next_x, int next_y){
    clear_position(board,beast);
    
    board->b[next_x][next_y] = '*';
    beast->x = next_x;
    beast->y = next_y;
    
    attron(COLOR_PAIR(6));
    mvprintw(next_x+SHIFTX,next_y+SHIFTY,"*");        
    attron(COLOR_PAIR(5));
}
void mv_b_to_bush(board_t *board, beast_t *beast, int next_x, int next_y){
    clear_position(board,beast);

    beast->x = next_x;
    beast->y = next_y;

    attron(COLOR_PAIR(6));
    mvprintw(next_x+SHIFTX,next_y+SHIFTY,"#");        
    attron(COLOR_PAIR(5));
}
void mv_b_to_money(board_t *board, beast_t *beast, int next_x, int next_y){
    clear_position(board,beast);

    beast->x = next_x;
    beast->y = next_y; 

    attron(COLOR_PAIR(6));
    mvprintw(next_x+SHIFTX,next_y+SHIFTY, "%c", board->b[next_x][next_y]);        
    attron(COLOR_PAIR(5));  
}

int find_player(board_t *board, int Bx, int By, int *Px, int *Py){
    int start_x = Bx - RENDER_SIZE/2;
    int stop_x = Bx + RENDER_SIZE/2 + 1;
    int start_y = By - RENDER_SIZE/2;
    int stop_y = By + RENDER_SIZE/2 + 1;
    
    for (int i=start_x; i<stop_x; ++i){
        for (int j=start_y; j<stop_y; ++j){
            if ( !( i >= 0 && i < board->rows && j >= 0 && j < board->cols ) )
                continue;
            if ( board->b[i][j] >= '1' && board->b[i][j] <= '4'){
                *Px = i;
                *Py = j;
                return 1;
            }
        }
    }
    return 0;
}
int visible(board_t *board, int Bx, int By, int Px, int Py){
  int abs_dif_x = abs(Bx-Px);
  int abs_dif_y = abs(By-Py);
  int dif_x = Px-Bx;
  int dif_y = Py-Bx;

  if ( abs_dif_x == 2 && abs_dif_y == 1  ){
    int x = Bx+dif_x/2;
    int y1 = By, y2 = Py;
    if ( board->b[x][y2] != 'W' && board->b[x][y1] != 'W' && board->b[x][y2] != 'A' && 
         board->b[x][y1] != 'A' && board->b[x][y2] != '*' && board->b[x][y1] != '*' )
      return 1;
  }
  else if ( abs_dif_x == 1 && abs_dif_y == 2 ){
    int y = By+dif_y/2;
    int x1 = Bx, x2 = Px;
    if  (  board->b[x2][y] != 'W' && board->b[x1][y] != 'W' && board->b[x2][y] != 'A' && 
           board->b[x2][y] != 'A' && board->b[x1][y] != '*' && board->b[x2][y] != '*' )
      return 1;
  }
  else{
    if ( adjacent(board, Bx, By, Px, Py) )
      return 1;
    else {
      int x = Bx+dif_x/2;
      int y = By+dif_y/2;
      if ( board->b[x][y] != 'W' && board->b[x][y] != '*' && board->b[x][y] != 'A' )
        return 1;
    }
  }
  return 0;
}
int adjacent(board_t *board, int Bx, int By, int Px, int Py){
  for (int i=-1; i<2; ++i){
    for (int j=-1; j<2; ++j){
      if ( board->b[Bx+i][By+j] >= '1' && board->b[Bx+i][By+i] <= '4')
        return 1;
    }
  }
  return 0;
}

int mv_to_player(board_t *board, beast_t *beast, int Px, int Py){
    double dist[4]; //UP,LEFT,DOWN,RIGHT
    calculate_distance(board,beast->x,beast->y,Px,Py,dist);
    int j=0;
    int dir_x[4] = {-1,0,1,0};
    int dir_y[4] = {0,-1,0,1};
    int dir = -1;
    double min;

    do{
        min = dist[j++];
    } while ( min < 0 );
    for ( int i=0; i<4; ++i ){
        if ( dist[i] > 0 && dist[i] < min )
        min = dist[i];
    }
    for ( int i=0; i<4; ++i ){ 
        if ( dist[i] - min <= 0.1 && dist[i] - min >= 0 ){
            dir = i;
            break;
        }
    }
            
    if ( dir == -1 || (dist[dir] >= 0 && dist[dir] <= 0.1) )
        return -1;
    
    clear_position(board,beast);
    
    beast->x += dir_x[dir];
    beast->y += dir_y[dir];
    if ( board->b[beast->x][beast->y] == ' ' )
        board->b[beast->x][beast->y] = '*';    
    
    attron(COLOR_PAIR(6));
    mvprintw(beast->x+SHIFTX, beast->y+SHIFTY, "%c", board->b[beast->x][beast->y]);
    attron(COLOR_PAIR(5));

    return 0;
}
void calculate_distance(board_t *board, int Bx, int By, int Px, int Py, double dist[4]){
  int dir_x[4] = {-1,0,1,0};
  int dir_y[4] = {0,-1,0,1};
  for ( int i=0; i<4; ++i ){
    if ( board->b[ Bx+dir_x[i] ][ By+dir_y[i] ] != 'W' && board->b[ Bx+dir_x[i] ][ By+dir_y[i] ] != 'A' && board->b[ Bx+dir_x[i] ][ By+dir_y[i] ] != '*')
      dist[i] = sqrt( pow( Bx+dir_x[i]-Px, 2 ) + pow( By+dir_y[i]-Py, 2 ) );
    else 
      dist[i] = -1;
  }
}