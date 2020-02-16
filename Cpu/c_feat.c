#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <ncurses.h>
#include <string.h>
#include <ncurses.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include "c_feat.h"

server_t * init_server_shm(int *serv_fd, char *serv_sh_name, int id){
    sprintf(serv_sh_name,"shm_serv_%d",id);
    *serv_fd = shm_open(serv_sh_name, O_RDONLY, 0600);
    assert( *serv_fd != -1 );
    server_t * res = (server_t *)mmap(NULL, 
                sizeof(server_t), PROT_READ, MAP_SHARED, *serv_fd, 0);
    assert( res != MAP_FAILED );
    return res;
}
client_t * init_client_shm(int *cl_fd, char *cl_sh_name, int id){
    sprintf(cl_sh_name,"shm_cl_%d",id); 
    *cl_fd = shm_open(cl_sh_name, O_CREAT | O_RDWR, 0600 );
    assert( *cl_fd != -1 );
    ftruncate(*cl_fd,sizeof(client_t));
    client_t * res = (client_t *)mmap( NULL, 
                sizeof(client_t), PROT_READ | PROT_WRITE, MAP_SHARED, *cl_fd, 0 );
    assert( res != MAP_FAILED );
    res->pid = getpid();
    return res;
}

void init_client(client_t *cl_msg, int id, char *type){
    if ( !strcmp(type,"./Human") )
        cl_msg->type = PLAYER;
    else cl_msg->type = BOT;
    cl_msg->move = NONE;
    cl_msg->procces_status = RUNNING;
    cl_msg->id = id;
    cl_msg->pid = getpid();
    cl_msg->request_nr = 0;
}
void init_board(char board[MAX_BOARD][MAX_BOARD]){
    for (int i=0; i<MAX_BOARD; ++i){
        for (int j=0; j<MAX_BOARD; ++j)
            board[i][j] = ' ';
    }
}
void display_legend(){
    char title[][11] = {"Wall","Human/CPU","Camp","Bush","Beast","Money"};
    char title_acr[][5] = {" ","1234","A","#","*","ctTD"};
    int begin = MAX_BOARD + SHIFTY + 2;
    for (int i=0; i<6; ++i){
        attron(COLOR_PAIR(i+2));
        mvprintw(SHIFTX+i,begin,"%s", title_acr[i]); 
        attron(COLOR_PAIR(5));
        mvprintw(SHIFTX+i,begin+4," - %s", title[i]); 
    } 
}
void display_info(server_t *serv_msg, client_t *cl_msg){
    int beg_x = SHIFTX + 7;
    int beg_y = MAX_BOARD + SHIFTY + 2;
    
    mvprintw(beg_x++,beg_y,"Server's PID:      %d", serv_msg->serv_pid );
    mvprintw(beg_x++,beg_y," Campsite X/Y:     unknown" );
    mvprintw(beg_x++,beg_y," Round number X/Y: 1");
    beg_x++;
    mvprintw(beg_x++,beg_y,"Player:");
    mvprintw(beg_x++,beg_y," Number:        %d", serv_msg->id+1);
    if ( cl_msg->type == BOT )
        mvprintw(beg_x++,beg_y," Type:          BOT");
    else mvprintw(beg_x++,beg_y," Type:          HUMAN");
    mvprintw(beg_x++,beg_y," Curr X/Y:      %02d/%02d", serv_msg->respX, serv_msg->respY);
    mvprintw(beg_x++,beg_y," Deaths:        0");
    beg_x++;
    mvprintw(beg_x++,beg_y," Coins found:   0");
    mvprintw(beg_x++,beg_y," Coins brought: 0");
}

void init_col(){
    init_pair(1, COLOR_BLACK, COLOR_BLACK);     //empty space
    init_pair(2, COLOR_WHITE, COLOR_WHITE);     //walls
    init_pair(3, COLOR_BLACK, COLOR_MAGENTA);   //bot, player
    init_pair(4, COLOR_YELLOW, COLOR_GREEN);    //camp
    init_pair(5, COLOR_WHITE, COLOR_BLACK);     //bush, normal
    init_pair(6, COLOR_BLACK, COLOR_RED);       //beast
    init_pair(7, COLOR_BLACK, COLOR_YELLOW);    //money
    init_pair(8, COLOR_BLACK, COLOR_BLUE);      //free color
}
void clear_enemies_moves(char board[MAX_BOARD][MAX_BOARD]){
    for (int i=0; i<MAX_BOARD; ++i){
        for (int j=0; j<MAX_BOARD; ++j){
            if ( (board[i][j] >= '1' && board[i][j] <= '4') || board[i][j] == '*' ){
                board[i][j] = ' ';
                attron(COLOR_PAIR(1));
                mvprintw(i+SHIFTX,j+SHIFTY," " );
            }
        }
    }
}
void update_board(char board[MAX_BOARD][MAX_BOARD], server_t *serv_msg){
    clear_enemies_moves(board);
    
    int xb,yb, xr, yr;  //xboard... xrender...
    int y = serv_msg->y;
    int x = serv_msg->x;
    int shift = RENDER_SIZE/2;
    
    for (xb=x-shift, xr=0;      xb<x-shift+RENDER_SIZE;         ++xb, ++xr){
        for (yb=y-shift, yr=0;      yb<y-shift+RENDER_SIZE;         ++yb, ++yr){
            if ( (xb>=0 && xb<MAX_BOARD) && (yb>=0 && yb<MAX_BOARD) ){
                board[xb][yb] = serv_msg->tab[xr][yr];
                if ( board[xb][yb] == ' ' )
                    attron(COLOR_PAIR(1));
                else if ( board[xb][yb] == 'W' )
                    attron(COLOR_PAIR(2));
                else if ( board[xb][yb] >= '1' && board[xb][yb] <= '4' )
                    attron(COLOR_PAIR(3));
                else if ( board[xb][yb] == 'A' ){
                    display_camp_info(xb,yb);
                    attron(COLOR_PAIR(4));
                }
                else if ( board[xb][yb] == '#' )
                    attron(COLOR_PAIR(5));
                else if ( board[xb][yb] == '*' )
                    attron(COLOR_PAIR(6));
                else if ( board[xb][yb] == 'c' || board[xb][yb] == 't' || board[xb][yb] == 'T' || board[xb][yb] == 'D')
                    attron(COLOR_PAIR(7));
                    
                if (  board[xb][yb] != 'O' )
                    mvprintw(xb+SHIFTX,yb+SHIFTY,"%c", board[xb][yb] ); 
            }
        }
    }
      
    if ( board[x][y] == '#' ){     //x y player position
        attron(COLOR_PAIR(3));         
        mvprintw(x+SHIFTX,y+SHIFTY,"#");         
    }
    attron(COLOR_PAIR(5));  
}
void display_camp_info(int x, int y){
    int beg_x = SHIFTX + 7 + 1;
    int beg_y = MAX_BOARD + SHIFTY + 2; 
    attron(COLOR_PAIR(5));
    mvprintw(beg_x++,beg_y," Campsite X/Y:     %02d/%02d    ", x, y); 
}
void update_info(server_t *serv_msg){
    int beg_x = SHIFTX + 7 + 7;
    int beg_y = MAX_BOARD + SHIFTY + 2 + strlen(" Type:          ");
    mvprintw(beg_x++,beg_y,"%02d/%02d", serv_msg->x, serv_msg->y);    
    mvprintw(beg_x++,beg_y,"%02d", serv_msg->deaths);
    beg_x++;
    mvprintw(beg_x++,beg_y,"%02d", serv_msg->carry);    
    mvprintw(beg_x++,beg_y,"%02d", serv_msg->brought);    
}

void move_me(client_t *cl_msg, server_t *serv_msg){
    int px, py;
    int render_mid = RENDER_SIZE/2;
    if ( find_beast(serv_msg->tab,&px,&py) && visible(serv_msg->tab,render_mid,render_mid,px,py) ){
        cl_msg->move = reaction_on_beast(serv_msg->tab,render_mid,render_mid,px,py);
        return;
    }
    else if ( serv_msg->client_status == WALL_COL || cl_msg->move == NONE || rand()%3)
        cl_msg->move = rand()%4;
}

void close_connections(int serv_fd, int cl_fd, server_t *serv_msg, 
        client_t *cl_msg, sem_t *sem_crit, sem_t *sem_join){
    sem_close(sem_join);
    cl_msg->procces_status = CLOSED;
    close(serv_fd);
    close(cl_fd);
    munmap(serv_msg,sizeof(server_t));
    munmap(cl_msg,sizeof(client_t));
    sem_close(sem_crit);       
    
}

int find_beast(const char tab[RENDER_SIZE][RENDER_SIZE], int *Px, int *Py){
    for (int i=0; i<RENDER_SIZE; ++i){
        for (int j=0; j<RENDER_SIZE; ++j){
            if ( tab[i][j] == '*' ){
                *Px = i;
                *Py = j;
                return 1;
            }
        }
    }
    return 0;
}
int visible(const char tab[RENDER_SIZE][RENDER_SIZE], int Bx, int By, int Px, int Py){
  int abs_dif_x = abs(Bx-Px);
  int abs_dif_y = abs(By-Py);
  int dif_x = Px-Bx;
  int dif_y = Py-Bx;

  if ( abs_dif_x == 2 && abs_dif_y == 1  ){
    int x = Bx+dif_x/2;
    int y1 = By, y2 = Py;
    if ( tab[x][y2] != 'W' && tab[x][y1] != 'W' && tab[x][y2] != 'A' && 
         tab[x][y1] != 'A' && tab[x][y2] != '*' && tab[x][y1] != '*' )
      return 1;
  }
  else if ( abs_dif_x == 1 && abs_dif_y == 2 ){
    int y = By+dif_y/2;
    int x1 = Bx, x2 = Px;
    if  ( tab[x1][y] != 'W' && tab[x2][y] != 'W' && tab[x1][y] != 'A' && 
          tab[x2][y] != 'A' && tab[x1][y] != '*' && tab[x2][y] != '*' )
      return 1;
  }
  else{
    if ( adjacent(tab, Bx, By, Px, Py) )
      return 1;
    else {
      int x = Bx+dif_x/2;
      int y = By+dif_y/2;
      if ( tab[x][y] != 'W' && tab[x][y] != 'A' && tab[x][y] != '*')
        return 1;
    }
  }
  return 0;
}
int adjacent(const char tab[RENDER_SIZE][RENDER_SIZE], int Bx, int By, int Px, int Py){
  for (int i=-1; i<2; ++i){
    for (int j=-1; j<2; ++j){
      if ( tab[Bx+i][By+j] == '*' )
        return 1;
    }
  }
  return 0;
}

int reaction_on_beast(const char tab[RENDER_SIZE][RENDER_SIZE], int Bx, int By, int Px, int Py){
  double dist[4]; //UP,LEFT,DOWN,RIGHT
  calculate_distance(tab,Bx,By,Px,Py,dist);
  int j=0;
  double min;
  
      do{
        min = dist[j++];
      } while ( min < 0 );
      
      for ( int i=0; i<4; ++i ){
        if ( dist[i] > 0 && dist[i] < min )
          min = dist[i];
      }
      
      for ( int i=0; i<4; ++i ){ 
        if ( dist[i] - min <= 0.1 && dist[i] - min >= 0 )
            return i;
      }  
    
  return -1;
}
void calculate_distance(const char tab[RENDER_SIZE][RENDER_SIZE], int Bx, int By, int Px, int Py, double dist[4]){
  int dir_x[4] = {-1,0,1,0};
  int dir_y[4] = {0,-1,0,1};
  for ( int i=0; i<4; ++i ){
    if ( tab[Bx+dir_x[i]][By+dir_y[i]] == ' ' )
      dist[i] = sqrt( pow(Bx+dir_x[i]-Px,2) + pow(By+dir_y[i]-Py,2) );
    else 
      dist[i] = -1;
  }
}
