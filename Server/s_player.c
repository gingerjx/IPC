#include <stdio.h>
#include <ncurses.h>
#include <string.h>
#include "s_feat.h"
#include "s_player.h"
int col_tres_board[MAX_BOARD][MAX_BOARD] = {0};

int check_move(board_t *board, server_t *serv_msg, client_t *cl_msg,  server_t *all_serv_msg[MAX_CLIENTS]){
    int dirX[] = {-1,0,1,0};
    int dirY[] = {0,-1,0,1};
    
    if ( serv_msg->client_status == CLOSE )
        return 1;
    

    if ( cl_msg->move < 4 ){
        int x = serv_msg->x + dirX[cl_msg->move];
        int y = serv_msg->y + dirY[cl_msg->move];
        char c = board->b[x][y];
        
        if ( c == ' ' )
            move_to_empty(board->b, x,y,serv_msg);
        else if ( c == '#' )
            move_to_bush(board->b,x,y,serv_msg);
        else if ( c == 'c' || c == 't' || c == 'T' || c == 'D' )
            move_to_money(board,x,y,serv_msg,board->b[x][y]);
        else if ( c == 'A' )
            move_to_camp(board->rows,serv_msg);
        else if ( c >= '1' && c <= '4' )
            collision_players(board,serv_msg,all_serv_msg[c-'0'-1]);
        else serv_msg->client_status = WALL_COL;
    }
    
    return 0;    
}

void move_to_empty(char **board, int x, int y, server_t *serv_msg){
    if ( board[serv_msg->x][serv_msg->y] != '#' ){
        mvprintw(serv_msg->x+SHIFTX,serv_msg->y+SHIFTY," ");
        board[serv_msg->x][serv_msg->y] = ' ';
    } else mvprintw(serv_msg->x+SHIFTX,serv_msg->y+SHIFTY,"#");
    
    attron(COLOR_PAIR(3));
    mvprintw(x+SHIFTX,y+SHIFTY,"%c", serv_msg->id+'0'+1 );
    attron(COLOR_PAIR(5)); 
  
    board[x][y] = serv_msg->id+'0'+1;
    serv_msg->x = x;
    serv_msg->y = y;
    serv_msg->client_status = OKEY;
    serv_msg->sleep_time = 0;
}
void move_to_bush(char **board, int x, int y, server_t *serv_msg){
    if ( board[serv_msg->x][serv_msg->y] != '#' ){
        mvprintw(serv_msg->x+SHIFTX,serv_msg->y+SHIFTY," ");
        board[serv_msg->x][serv_msg->y] = ' ';
    } else mvprintw(serv_msg->x+SHIFTX,serv_msg->y+SHIFTY,"#");
    
    attron(COLOR_PAIR(3));
    mvprintw(x+SHIFTX,y+SHIFTY,"#");
    attron(COLOR_PAIR(5)); 
    
    serv_msg->x = x;
    serv_msg->y = y;
    serv_msg->client_status = BUSH_COL;
    serv_msg->sleep_time = 1;   
}
void move_to_money(board_t *board, int x, int y, server_t *serv_msg, char money){
    if ( board->b[serv_msg->x][serv_msg->y] != '#' ){
        mvprintw(serv_msg->x+SHIFTX,serv_msg->y+SHIFTY," ");
        board->b[serv_msg->x][serv_msg->y] = ' ';
    } else mvprintw(serv_msg->x+SHIFTX,serv_msg->y+SHIFTY,"#");
    
    attron(COLOR_PAIR(3));
    mvprintw(x+SHIFTX,y+SHIFTY,"%c", serv_msg->id+'0'+1 );
    attron(COLOR_PAIR(5)); 

    board->b[x][y] = serv_msg->id+'0'+1;    
    serv_msg->x = x;
    serv_msg->y = y;
    serv_msg->client_status = OKEY;
    serv_msg->sleep_time = 0;
    
    switch(money){
        case 'c':
            serv_msg->carry += 1;
            break;
        case 't':
            serv_msg->carry += 10;
            break;
        case 'T':
            serv_msg->carry += 50;
            break;
        case 'D':
            serv_msg->carry += col_tres_board[x][y];     //temporary
            col_tres_board[x][y] = 0;
            break;
    }
    if ( money != 'D' ){
        attron(COLOR_PAIR(7));
        rand_new(board,money);
        attron(COLOR_PAIR(5)); 
    }
        
    int begin_x = SHIFTX + board->rows + 7 + 6;
    int begin_y = SHIFTY + strlen(" PID      \t\t  ") + 6;
    int next_y = strlen("---  \t  ");
    mvprintw(begin_x,begin_y + (next_y * serv_msg->id),"%04d", serv_msg->carry);
}
void move_to_camp(int rows, server_t *serv_msg){
    serv_msg->client_status = OKEY;
    serv_msg->sleep_time = 0;
    serv_msg->brought += serv_msg->carry;
    serv_msg->carry = 0;

    int begin_x = SHIFTX + rows + 7 + 6;
    int begin_y = SHIFTY + strlen(" PID      \t\t  ") + 6;
    int next_y = strlen("---  \t  ");
    mvprintw(begin_x++,begin_y + (next_y * serv_msg->id)," 0  ");
    mvprintw(begin_x,begin_y + (next_y * serv_msg->id),"%04d", serv_msg->brought);
}

void collision_players(board_t *board, server_t *serv_msg_1, server_t *serv_msg_2){
    int x1 = serv_msg_1->x;
    int y1 = serv_msg_1->y;
    int x2 = serv_msg_2->x;
    int y2 = serv_msg_2->y;
    int id1 = serv_msg_1->id;
    int id2 = serv_msg_2->id;
    
    if ( board->b[x1][y1] == id1 + '0' + 1){
        board->b[x1][y1] = ' ';
        mvprintw(x1 + SHIFTX,y1 + SHIFTY," ");
    } else mvprintw(serv_msg_1->x+SHIFTX,serv_msg_1->y+SHIFTY,"#");
    if ( board->b[x2][y2] == id2 + '0' + 1){
        board->b[x2][y2] = ' ';
        mvprintw(x2 + SHIFTX,y2 + SHIFTY," ");
    } else mvprintw(serv_msg_2->x+SHIFTX,serv_msg_2->y+SHIFTY,"#");

    int drop = serv_msg_1->carry + serv_msg_2->carry;
    serv_msg_1->carry = serv_msg_2->carry = 0;    
    board->b[serv_msg_1->respX][serv_msg_1->respY] = id1+'0'+1;
    board->b[serv_msg_2->respX][serv_msg_2->respY] = id2+'0'+1;
    serv_msg_1->client_status = serv_msg_2->client_status = PLAYERS_COL;
    serv_msg_1->sleep_time = serv_msg_2->sleep_time = 3;
    serv_msg_1->x = serv_msg_1->respX;
    serv_msg_1->y = serv_msg_1->respY;
    serv_msg_2->x = serv_msg_2->respX;
    serv_msg_2->y = serv_msg_2->respY;
    
    board->b[x1][y1] = 'D';
    col_tres_board[x1][y1] = drop;
    attron(COLOR_PAIR(7));
    mvprintw(x1 + SHIFTX,y1 + SHIFTY, "D");
    
    attron(COLOR_PAIR(3));
    mvprintw(serv_msg_1->respX + SHIFTX, serv_msg_1->respY + SHIFTY, "%c", id1+'0'+1);
    mvprintw( serv_msg_2->respX + SHIFTX, serv_msg_2->respY + SHIFTY, "%c", id2+'0'+1);
    attron(COLOR_PAIR(5));
     
    int begin_x = SHIFTX + board->rows + 7 + 6;
    int begin_y = SHIFTY + strlen(" PID      \t\t  ") + 6;
    int next_y = strlen("---  \t  ");
    mvprintw(begin_x,begin_y + (next_y * serv_msg_1->id)," 0  ");
    mvprintw(begin_x,begin_y + (next_y * serv_msg_2->id)," 0  ");
    //mvprintw(begin_x,begin_y + (next_y * serv_msg_2->id)," 0  ");
    //mvprintw(begin_x,begin_y + (next_y * serv_msg_1->id)," 0  ");
    
    update_render(serv_msg_1, board);
    update_render(serv_msg_2, board);  
}
void collision_beast(board_t *board, server_t *serv_msg){
    int x = serv_msg->x;
    int y = serv_msg->y;
    if ( x == serv_msg->respX && y == serv_msg->respY )     //if player is in spawn
        return;
        
    if ( board->b[x][y] != '#' ){
        mvprintw(serv_msg->x+SHIFTX,serv_msg->y+SHIFTY," ");
        board->b[serv_msg->x][serv_msg->y] = ' ';
    } else mvprintw(serv_msg->x+SHIFTX,serv_msg->y+SHIFTY,"#");
    
    board->b[serv_msg->respX][serv_msg->respY] = serv_msg->id+'0'+1;  
    int drop = serv_msg->carry;
    serv_msg->carry = 0;
    serv_msg->deaths++;
    
    serv_msg->x = serv_msg->respX;
    serv_msg->y = serv_msg->respY;
    serv_msg->sleep_time = 3;
    serv_msg->client_status = BEAST_COL;
    
    col_tres_board[x][y] = drop;
    board->b[x][y] = 'D';
    
    attron(COLOR_PAIR(7));
    mvprintw(x + SHIFTX,y + SHIFTY, "D");
    attron(COLOR_PAIR(3));
    mvprintw(serv_msg->x + SHIFTX,serv_msg->y + SHIFTY, "%c", serv_msg->id+'0'+1);
    attron(COLOR_PAIR(5));
    

    int begin_x = SHIFTX + board->rows + 7 + 3;
    int begin_y = SHIFTY + strlen(" PID      \t\t  ") + 6;
    int next_y = strlen("---  \t  ");
    mvprintw(begin_x,begin_y + (next_y * serv_msg->id),"%02d",serv_msg->deaths);
    begin_x += 3;
    mvprintw(begin_x,begin_y + (next_y * serv_msg->id)," 0  ");
}