# IPC

Interprocess communitaction project visualized by simple game.

## Compile&Run
Firstly for visualization you need to install ncurses library.

Then type ‘make’ in terminal, it will create executable files
```
make
```
Now in different terminals execute in any order server:
```
./server
```

And clients (remember about ID):

```
./cpu 4
```
```
./human 2
```
## Communication details

IPC project is visualized by simple game where server and clients( cpus and humans ) are separated processes. Server is designed for maximum 4 clients. If user try to execute more clients, server will reject to join him. It is also impossible to execute client with ID that already exists. Server deal with situation where client leave the game in natural way ( by typing Q – only in human ) or killing the process. When closing the server, we also close all clients. After the client quits server is ready to join next client on his place. This communication smoothness is ensured by mechanisms like semaphores, shared memory and threads.

Clients can affect on game only by move requests. Server receives this requests and processes this move, by updating board and game. Server passes information to clients about they surrounding ,status and statistics. They can change them only locally.


## Few word about game

Server input:

	B/b – add new beast to game
	c/t/T – add new reward to game
	Q/q – close server and clients
Human input:

	Arrows – move
	Q/q – close client

Client goal is to collect the rewards and store them in campside. Client cannot go through wall and bushes slow him down. In case of collision with other client or beast, client lost his treasure and leave on the map.  

### Server view
![img](/Server.png)
### Client view ( 4 clients in 4 terminals ) 
![img](/4PLayers.png)
