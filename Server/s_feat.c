#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <assert.h>
#include <ncurses.h>
#include <string.h>
#include <semaphore.h>
#include <stdlib.h>
#include "s_feat.h"
#include "s_beast.h"
#include "global.h"

server_t *init_server_shm(int *serv_fd, char *serv_sh_name, int id){
        sprintf(serv_sh_name,"shm_serv_%d",id); 
        *serv_fd = shm_open( serv_sh_name, O_CREAT | O_RDWR, 0600 );
        assert( *serv_fd != -1 );
        ftruncate( *serv_fd, sizeof(server_t) );
        server_t * res = (server_t *)mmap(NULL, 
                    sizeof(server_t), PROT_READ | PROT_WRITE, MAP_SHARED, *serv_fd, 0 );
        if ( res != MAP_FAILED );
        res->joined = 1;   
        return res;
}
client_t *init_client_shm(int *cl_fd, char *cl_sh_name, int id){
        sprintf(cl_sh_name,"shm_cl_%d",id);    
        *cl_fd = shm_open( cl_sh_name, O_CREAT | O_RDONLY, 0600 );
        assert( *cl_fd != -1 );
        client_t * res = (client_t *)mmap( NULL, 
                    sizeof(client_t), PROT_READ, MAP_SHARED, *cl_fd, 0);
        if ( res != MAP_FAILED ); 
        return res;
}

void *join_client(void *p){   
    s_for_join_t *s = (s_for_join_t *)p;
    char name[20];
    sprintf( name,"sem_client_%d", s->serv_msg->id);
    sem_t *sem_join;

    while(1){
        while(s->serv_msg->joined == 0){
            usleep(SEC);
            sem_join = sem_open( name, O_CREAT | O_EXCL, 0600, 0);
            if ( sem_join == SEM_FAILED )
                s->serv_msg->joined = 1;
            else{
                sem_close(sem_join);
                sem_unlink(name);
            }
        }
        sem_wait(s->sem_crit);
        mvprintw(0,0,"[ %d ]Client joins the game           ", s->serv_msg->id+1);
        join_client_to_game(s->board,s->serv_msg,s->cl_msg);
        sem_post(s->sem_crit);
        while ( s->cl_msg->procces_status == RUNNING && getpgid(s->cl_msg->pid) >= 0 ) {}     //waiting to be able to join next client on this place
        
        sem_wait(s->sem_crit);
        mvprintw( 0,0,"[ %d ]Client leaves           ", s->serv_msg->id+1 );
        s->serv_msg->joined = 0;
        reset_client(s->board,s->serv_msg);
        sem_unlink(name);
        sem_post(s->sem_crit);
    }
}
void join_client_to_game(board_t *board, server_t *serv_msg, client_t *cl_msg){
    rand_spawn(serv_msg,board);
    board->b[serv_msg->x][serv_msg->y] = serv_msg->id+'0'+1;
    attron(COLOR_PAIR(3));
    mvprintw(serv_msg->x+SHIFTX,serv_msg->y+SHIFTY,"%c", board->b[serv_msg->x][serv_msg->y] );  
    attron(COLOR_PAIR(5));
    update_render(serv_msg,board);
    
    int begin_x = SHIFTX + board->rows + 7;
    int begin_y = SHIFTY + strlen(" PID      \t\t  ") + 6;
    int next_y = strlen("---  \t  ");
    
    mvprintw(begin_x++,begin_y + (next_y * serv_msg->id)-1,"%d", cl_msg->pid);
    if ( cl_msg->type == BOT ) 
        mvprintw(begin_x++,begin_y + (next_y * serv_msg->id),"BOT");
    else mvprintw(begin_x++,begin_y + (next_y * serv_msg->id)-2,"PLAYER");
    mvprintw(begin_x++,begin_y + (next_y * serv_msg->id)-1,"%02d/%02d", serv_msg->respX, serv_msg->respY);
    mvprintw(begin_x++,begin_y + (next_y * serv_msg->id)," 0 ");
    begin_x += 2;
    mvprintw(begin_x++,begin_y + (next_y * serv_msg->id)," 0  ");
    mvprintw(begin_x++,begin_y + (next_y * serv_msg->id)," 0  ");
    
    join_client_to_game_data(serv_msg);
}
void join_client_to_game_data(server_t *serv_msg){
    serv_msg->client_status = OKEY;
    serv_msg->carry = 0;
    serv_msg->score = 0;
    serv_msg->brought = 0;
    serv_msg->deaths = 0;
    serv_msg->sleep_time = 1;
    serv_msg->request_nr = 0;
}
void reset_client(board_t *board, server_t *serv_msg){
    int begin_x = SHIFTX + board->rows + 7;
    int begin_y = SHIFTY + strlen(" PID      \t\t  ") + 6;
    int next_y = strlen("---  \t  ");
    
    mvprintw(begin_x++,begin_y + (next_y * serv_msg->id)-1," ---");
    mvprintw(begin_x++,begin_y + (next_y * serv_msg->id)-2,"  --- ");
    mvprintw(begin_x++,begin_y + (next_y * serv_msg->id)-1,"----- ", serv_msg->x, serv_msg->y);
    mvprintw(begin_x++,begin_y + (next_y * serv_msg->id),"--- ");
    begin_x += 2;
    mvprintw(begin_x++,begin_y + (next_y * serv_msg->id),"--- ");
    mvprintw(begin_x++,begin_y + (next_y * serv_msg->id),"--- "); 
    
    board->b[serv_msg->x][serv_msg->y] = ' ';
    mvprintw(serv_msg->x+SHIFTX,serv_msg->y+SHIFTY," ");  
    reset_client_data(serv_msg);
}
void reset_client_data(server_t *serv_msg){
    serv_msg->client_status = OKEY;
    serv_msg->carry = 0;
    serv_msg->brought = 0;
    serv_msg->deaths = 0;
    serv_msg->sleep_time = 1;
    for (int i=0; i<RENDER_SIZE; ++i){
        for (int j=0; j<RENDER_SIZE; ++j)
            serv_msg->tab[i][j] = ' ';
    }
}

void *input_thread(void *p){
    char *in = (char *)p;
    while(1) *in = getchar();
}
int input_react(char *input, board_t *board, beast_t *beasts, int *beasts_num){
    if ( *input == 'q' || *input == 'Q' )
        return 1;
    else if ( *input == 'c' || *input == 't' || *input == 'T' ){
        attron(COLOR_PAIR(7));
        rand_new(board,*input);
        attron(COLOR_PAIR(5));
    }   
    else if ( *input == 'b' || *input == 'B' ){
        add_beast(board, beasts, beasts_num);
    }
    *input = 0;
    return 0;
}

void init_clients(server_t *serv_msg[MAX_CLIENTS]){
    for (int i=0; i<MAX_CLIENTS; ++i){
        serv_msg[i]->serv_pid = getpid();
        serv_msg[i]->client_status = OKEY;
        serv_msg[i]->id = i;
        serv_msg[i]->joined = 0; 
        serv_msg[i]->carry = 0;
        serv_msg[i]->score = 0;
        serv_msg[i]->brought = 0;
        serv_msg[i]->deaths = 0;
        serv_msg[i]->sleep_time = 1;
        serv_msg[i]->request_nr = 0;
    }
}
void rand_spawn(server_t *serv_msg, board_t *board){
    int half_x = board->rows/2;
    int half_y = board->cols/2;
    int x;
    int y;
    
    if ( serv_msg->id == 0 ){
        x = rand()%half_x;
        y = rand()%half_y;
        while( board->b[x][y] != ' ' ){
            x = rand()%half_x;
            y = rand()%half_y; 
        }
    }
    else if ( serv_msg->id == 1 ){
        x = half_x + rand()%half_x;
        y = rand()%half_y;
        while( board->b[x][y] != ' ' ){
            x = half_x + rand()%half_x;
            y = rand()%half_y;
        }     
    }
    else if ( serv_msg->id == 2 ){
        x = half_x + rand()%half_x;
        y = half_y + rand()%half_y;
        while( board->b[x][y] != ' ' ){
            x = half_x + rand()%half_x;
            y = half_y + rand()%half_y;
        }        
    }
    else if ( serv_msg->id == 3 ){
        x = rand()%half_x;
        y = half_y + rand()%half_y;
        while( board->b[x][y] != ' ' ){
            x = rand()%half_x;
            y = half_y + rand()%half_y;
        }         
    }
    
    serv_msg->respX = serv_msg->x = x;
    serv_msg->respY = serv_msg->y = y;
}
void update_render(server_t*serv_msg, board_t *board){
    int xb,yb, xr, yr;  //xboard... xrender...
    int y = serv_msg->y;
    int x = serv_msg->x;
    int shift = RENDER_SIZE/2;

    for (xb = x-shift, xr=0;    xb < x-shift+RENDER_SIZE;       ++xb, ++xr){
        for (yb = y-shift, yr=0;    yb < y-shift+RENDER_SIZE;       ++yb, ++yr){
            if ( !(xb>=0 && xb<board->rows) || !(yb>=0 && yb<board->cols) )
                serv_msg->tab[xr][yr] = 'O';   //overflow
            else serv_msg->tab[xr][yr] = board->b[xb][yb];
        }
    }    
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
board_t *load_board(const char *filename){
    FILE *f;
    if ( (f=fopen(filename,"r")) == NULL )
        return NULL;
    int row = 0;
    int col = 0;

    char c, *possible_chars = "#WA\n ";    
    while(!feof(f)){
        c = fgetc(f);
        if ( c == EOF )
            break;
        if ( strchr(possible_chars,c) == NULL ){
            fclose(f);
            return NULL;
        }
        if ( c == '\n' ){
            row++;
            col = 0;
        }
        col++;
    }
    row++; col--;
    if ( col >= MAX_BOARD || row >= MAX_BOARD ){
        fclose(f);
        return NULL;
    }
    
    board_t *board = (board_t *)malloc( sizeof(board_t ) );
    if ( board == NULL ){
        fclose(f);
        return NULL;
    }
    char **b;
    b = (char **)malloc( sizeof(char *)*(row+1) );
    if ( b == NULL ){
        fclose(f);
        free(board);
        return NULL;
    }
        
    for (int i=0; i<row; ++i){
            b[i] = (char *)malloc( sizeof(char)*(col+1) );
            if ( b[i] == NULL ){
                free_maze(b,i);
                fclose(f);
                free(board);
                return NULL;
            }
    }   
    

    fseek(f,0,SEEK_SET);
    int i=0, j=0;
    while(!feof(f)){
            c = fgetc(f);
            if ( EOF == c )
                break;
            b[i][j++] = c;
            if ( c == '\n' ){
                i += 1;
                j = 0;
            }
    }
    board->b = b; board->rows = row; board->cols = col;
    
    attron(COLOR_PAIR(7));
    for (int i=0; i<COIN; ++i)
        rand_new(board,'c');
    for (int i=0; i<LITTLE_TRES; ++i)
        rand_new(board,'t');
    for (int i=0; i<BIG_TRES; ++i)
        rand_new(board,'T');
    attron(COLOR_PAIR(5));
    fclose(f);
    return board;
}
void rand_new(board_t *board, char c){
    int x = rand()%(board->rows);
    int y = rand()%(board->cols);
        while( board->b[x][y] != ' ' ){
            x = rand()%(board->rows);
            y = rand()%(board->cols); 
        }
    board->b[x][y] = c;
    mvprintw(x+SHIFTX,y+SHIFTY,"%c",c);
}

void displayBoard(board_t *board){
	int i=0,j=0;
    int camp_x, camp_y;
    for (i=0; i<board->rows; ++i){
        for (j=0; j<board->cols; ++j){
            if ( board->b[i][j] == ' ' )
                attron(COLOR_PAIR(1));
            else if ( board->b[i][j] == 'W' )
                attron(COLOR_PAIR(2));
            else if ( board->b[i][j] >= '1' && board->b[i][j] <= '4' )
                attron(COLOR_PAIR(3));
            else if ( board->b[i][j] == 'A' ){
                attron(COLOR_PAIR(4));
                camp_x = i; camp_y = j;
            }
            else if ( board->b[i][j] == '#' )
                attron(COLOR_PAIR(5));
            else if ( board->b[i][j] == '*' )
                attron(COLOR_PAIR(6));
            else if ( board->b[i][j] == 'c' || board->b[i][j] == 't' || board->b[i][j] == 'T')
                attron(COLOR_PAIR(7));
                
            mvprintw(i+SHIFTX,j+SHIFTY,"%c", board->b[i][j] );          
        }
    }

    display_legend(SHIFTY+board->cols+4);
    display_info(board->rows, camp_x, camp_y);
    attron(COLOR_PAIR(5));
}
void display_legend(int begin){
    char title[][11] = {"Wall","Human/CPU","Camp","Bush","Beast","Money"};
    char title_acr[][5] = {" ","1234","A","#","*","ctTD"};
    for (int i=0; i<6; ++i){
        attron(COLOR_PAIR(i+2));
        mvprintw(SHIFTX+i,begin,"%s", title_acr[i]); 
        attron(COLOR_PAIR(5));
        mvprintw(SHIFTX+i,begin+4," - %s", title[i]); 
    }   
}
void display_info(int rows, int c_x, int c_y){
    int begin = SHIFTX + rows + 2;
    mvprintw(begin++,SHIFTY,"Server's PID: %d", getpid());
    mvprintw(begin++,SHIFTY," Campsite X/Y: %02d/%02d", c_x, c_y);
    mvprintw(begin++,SHIFTY," Round number: %d", 1410);
    begin++;
    mvprintw(begin++,SHIFTY,"Parameter:\tPlayer1\tPlayer2\tPlayer3\tPlayer4");
    mvprintw(begin++,SHIFTY," PID      \t  ---  \t  ---  \t  ---  \t  ---  ");
    mvprintw(begin++,SHIFTY," Type     \t  ---  \t  ---  \t  ---  \t  ---  ");
    mvprintw(begin++,SHIFTY," Curr X/Y \t ----- \t ----- \t ----- \t ----- ");
    mvprintw(begin++,SHIFTY," Deaths   \t  ---  \t  ---  \t  ---  \t  ---  ");
    begin++;
    mvprintw(begin++,SHIFTY," Coins");
    mvprintw(begin++,SHIFTY,"  Carried \t  ---  \t  ---  \t  ---  \t  ---  ");
    mvprintw(begin++,SHIFTY,"  Brought \t  ---  \t  ---  \t  ---  \t  ---  ");
    
}

void close_connections(int serv_fd[MAX_CLIENTS], int cl_fd[MAX_CLIENTS],server_t *serv_msg[MAX_CLIENTS], 
        client_t *cl_msg[MAX_CLIENTS], char serv_sh_name[MAX_CLIENTS][25], char cl_sh_name[MAX_CLIENTS][25], 
        char *sem_name, sem_t *sem_crit, pthread_t ptJ[MAX_CLIENTS], pthread_t ptIn, board_t *board){
    for (int i=0; i<MAX_CLIENTS; ++i)
        pthread_cancel(ptJ[i]);
    pthread_cancel(ptIn);
    free_maze(board->b,board->rows);
    free(board);
    
    for (int i=0; i<MAX_CLIENTS; ++i){
        if (serv_msg[i]->joined && cl_msg[i]->procces_status == RUNNING){
            serv_msg[i]->client_status = CLOSE;
            while( cl_msg[i]->procces_status == RUNNING && getpgid(cl_msg[i]->pid) >= 0 );
            char sem_join_name[20];
            sprintf(sem_join_name,"sem_client_%d",serv_msg[i]->id);
            sem_unlink(sem_join_name);
        }
        log("Client [%d] closed\n", i);
    }
    
    for (int i=0; i<MAX_CLIENTS; ++i){
        close(serv_fd[i]); 
        close(cl_fd[i]);
        munmap(serv_msg[i],sizeof(server_t));
        munmap(cl_msg[i],sizeof(client_t));
        shm_unlink(serv_sh_name[i]);
        shm_unlink(cl_sh_name[i]);
        sem_close(sem_crit);
        sem_unlink(sem_name);
    }            
}
void free_maze(char **board, int num){
    if (board == NULL) return;
    for (int i=0; i<num; ++i)
        free(board[i]);
    free(board);
}
