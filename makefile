all: server human cpu

server: ./Server/*.c
	gcc -o server ./Server/*.c -lm -lpthread -lrt -lncurses

human: ./Human/*.c
	gcc -o human ./Human/*.c -lm -lpthread -lrt -lncurses

cpu: ./Cpu/*.c
	gcc -o cpu ./Cpu/*.c -lm -lpthread -lrt -lncurses