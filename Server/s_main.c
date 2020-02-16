#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <semaphore.h>
#include <pthread.h>
#include <fcntl.h>
#include <ncurses.h>
#include <string.h>
#include "s_feat.h"
#include "global.h"
#include "s_player.h"
#include "s_beast.h"

int main(int argc, char **argv)
{
    log("[ %d ] Starting server\n", getpid());
    srand(time(NULL));
    
///////// Semaphores init
    log("Semaphores...");
        char *sem_c_name = "sem_crit";
        sem_t *sem_crit = sem_open(sem_c_name,O_CREAT,0600,0);
        assert( sem_crit != SEM_FAILED );
    log(" done\n"); 
/////////
///////// Shared memory init
    log("Shared memory...");
        int serv_fd[MAX_CLIENTS];
        int cl_fd[MAX_CLIENTS];
        server_t *serv_msg[MAX_CLIENTS];
        client_t *cl_msg[MAX_CLIENTS];
        char serv_sh_name[MAX_CLIENTS][25];
        char cl_sh_name[MAX_CLIENTS][25];
        for (int i=0; i<MAX_CLIENTS; ++i){
            serv_msg[i] = init_server_shm(serv_fd + i, serv_sh_name[i], i);
            cl_msg[i] = init_client_shm(cl_fd + i, cl_sh_name[i], i);
        }
    log(" done\n");
/////////
///////// Structures init
    log("Structures...");
        board_t *board = load_board("./Server/board.txt");
        assert( board != NULL );
        init_clients(serv_msg);
        beast_t beasts[MAX_BEAST];
        int beasts_num = 0;
        
        sem_post(sem_crit);     //opening server
    log(" done\n");
/////////
///////// Threads init
    log("Threads...");
        pthread_t ptJoin[MAX_CLIENTS];
        s_for_join_t sf_join[MAX_CLIENTS];
        for (int i=0; i<MAX_CLIENTS; ++i){
            sf_join[i].cl_msg = cl_msg[i];
            sf_join[i].serv_msg = serv_msg[i];
            sf_join[i].board = board;
            sf_join[i].sem_crit = sem_crit;
            pthread_create( ptJoin + i,NULL,join_client,sf_join + i);
        }
        
        char input = 0;
        pthread_t ptInput;
        pthread_create(&ptInput,NULL,input_thread,&input);
    log(" done\n");
/////////
///////// Starting Game
    log("Starting game\n");

        initscr();
        noecho();
        curs_set(FALSE);
        assert( has_colors() != false );
        start_color();
        init_col();

        displayBoard(board);
        
        while(1){
            refresh();
            sem_wait(sem_crit);
            for (int i=0; i<beasts_num; ++i)
                move_beast(board,beasts+i);
            sem_post(sem_crit);
    
            for (int i=0; i<MAX_CLIENTS; ++i){
                if ( serv_msg[i]->joined && cl_msg[i]->procces_status == RUNNING ){
                    
                    int act_x = serv_msg[i]->x;
                    int act_y = serv_msg[i]->y;
                    if ( board->b[act_x+1][act_y] == '*' || board->b[act_x-1][act_y] == '*' || board->b[act_x][act_y+1] == '*' ||board->b[act_x][act_y-1] == '*' ){
                        collision_beast(board,serv_msg[i]);
                    } else if ( serv_msg[i]->sleep_time > 0 )
                        serv_msg[i]->sleep_time--;
                    else if ( serv_msg[i]->request_nr <= cl_msg[i]->request_nr ){
                        sem_wait(sem_crit);
                        if ( check_move(board,serv_msg[i],cl_msg[i],serv_msg ) )
                            break;
                        serv_msg[i]->request_nr++;
                        sem_post(sem_crit);
                    }
                    
                    update_render(serv_msg[i],board); 
                    int begin_x = SHIFTX + board->rows + 7 + 2;
                    int begin_y = SHIFTY + strlen(" PID      \t\t  ") + 6;
                    int next_y = strlen("---  \t  ");
                    mvprintw(begin_x,begin_y + (next_y * serv_msg[i]->id)-1,"%02d/%02d", serv_msg[i]->x, serv_msg[i]->y);
                    
                }
            }
            usleep(SERV_USLEEP);
            
            if ( input && input_react(&input,board,beasts,&beasts_num) )
                break;
        }
        
        endwin();
/////////
///////// Closing server
    log("Closing server\n");    
    
    close_connections(serv_fd,cl_fd,serv_msg,cl_msg,serv_sh_name,cl_sh_name,
                sem_c_name,sem_crit,ptJoin,ptInput,board);
    log("[ %d ] End server\n", getpid());
/////////
	return 0;
}
