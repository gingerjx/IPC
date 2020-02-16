#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <fcntl.h>
#include <ncurses.h>
#include <pthread.h>
#include "c_feat.h"
#include "../Server/global.h"

int main(int argc, char **argv){
    assert(argc >= 2);
    int ID = atoi(argv[1]);
    assert(ID > 0 && ID <= MAX_CLIENTS); 
    ID--;
    log("[ %d ] Starting client\n", getpid());
    srand(time(NULL));
///////// Semaphores init
    log("Semaphores...");
        sem_t *sem_join;
        char name[20];
        sprintf( name,"sem_client_%d", ID);
        sem_join = sem_open( name,O_CREAT | O_EXCL ,0600,0 );
        assert( sem_join != SEM_FAILED );
        char *sem_c_name = "sem_crit";
        sem_t *sem_crit = sem_open(sem_c_name,O_CREAT,0600,0);
        assert( sem_crit != SEM_FAILED );
        sem_wait(sem_crit);
    log(" done\n"); 
/////////
///////// Shared memory init
    log("Shared memory...");
        int serv_fd;
        int cl_fd;
        char serv_sh_name[25];
        char cl_sh_name[25]; 
        server_t *serv_msg = init_server_shm(&serv_fd,serv_sh_name,ID);
        client_t *cl_msg = init_client_shm(&cl_fd,cl_sh_name,ID);
    log(" done\n"); 
/////////
///////// Structures init
    log("Structures...");
        char board[MAX_BOARD][MAX_BOARD];
        init_board(board);
        
        init_client(cl_msg,ID,argv[0]);
        sem_post(sem_crit);
    log(" done\n"); 
/////////
///////// Threads init
/////////
///////// Starting Game
    log("Starting game\n");

        initscr();
        noecho();
        curs_set(FALSE);
        assert( has_colors() != false );
        start_color();
        init_col();
        display_legend();
        display_info(serv_msg,cl_msg);
        
        while(1){
            if ( serv_msg->request_nr > cl_msg->request_nr ){
                update_info(serv_msg);

                sem_wait(sem_crit);
                update_board(board,serv_msg);
                move_me(cl_msg, serv_msg);
                cl_msg->request_nr++;
                sem_post(sem_crit);
                
            refresh();
            }
            if ( serv_msg->client_status == CLOSE )
                    break;
        }
        
        endwin();
/////////
///////// Closing server
    log("Closing client\n");
    
    close_connections(serv_fd,cl_fd,serv_msg,cl_msg,sem_crit,sem_join);
    log("[ %d ] Close client\n", getpid());
	return 0;
}
