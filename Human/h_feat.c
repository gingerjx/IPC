#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <ncurses.h>
#include <string.h>
#include <ncurses.h>
#include <assert.h>
#include "h_feat.h"

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

void *input_thread(void *p){
    int *in = (int *)p;
    while(1) *in = wgetch(stdscr);
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

int move_me(int input, client_t *cl_msg){
    if ( input == 'w' || input == KEY_UP ){ 
        cl_msg->move = UP;
        return 1;
    }
    else if ( input == 's' || input == KEY_DOWN ){
        cl_msg->move = DOWN;
        return 1;
    }
    else if ( input == 'a' || input == KEY_LEFT){
        cl_msg->move = LEFT;
        return 1;
    }
    else if ( input == 'd' || input == KEY_RIGHT){
        cl_msg->move = RIGHT;
        return 1;
    }
    return 0;
}

void close_connections(int serv_fd, int cl_fd, server_t *serv_msg, 
        client_t *cl_msg, sem_t *sem_crit, sem_t *sem_join, pthread_t ptIn){
    pthread_cancel(ptIn);
    sem_close(sem_join);
    cl_msg->procces_status = CLOSED;
    close(serv_fd);
    close(cl_fd);
    munmap(serv_msg,sizeof(server_t));
    munmap(cl_msg,sizeof(client_t));
    sem_close(sem_crit);       
    
}