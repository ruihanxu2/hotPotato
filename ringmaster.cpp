#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <vector>
#include <assert.h>
#include <sys/time.h>
#include "potato.h"

using namespace std;

struct Player{
	int player_id;
	int player_port;
	int player_fd;
	char player_addr[512];

};

int main(int argc, char *argv[]) {
	//ringmaster <port_num> <num_players> <num_hops>
	
	if(argc!=4){
		cout << "Syntax: ringmaster <port_num> <num_players> <num_hops>\n" << endl;
		exit(1);
	}
	int port_num = atoi(argv[1]);
	int num_players = atoi(argv[2]);
	int num_hops = atoi(argv[3]);
	
	
	if(port_num<=1024||port_num>65535){
		cout << "Port already in used or not valid\n" << endl;
		exit(1);
	}
	
	if(num_players<=1){
		cout << "num_players is not valid\n" << endl;
		exit(1);
	}
	
	if(!(num_hops>=0&&num_hops<=512)){
		cout << "num_hops is not valid\n" << endl;
		exit(1);
	}
	
//	Potato Ringmaster
//	Players = <number>
//	Hops = <number>
	cout<<"Potato Ringmaster"<<endl;
	cout<<"Players = "<< num_players <<endl;
	cout<<"Hops = "<< num_hops <<endl;
	
	vector<Player*> players;
	Potato potato;
	potato.num_hops = num_hops;
	
	
	
	//------Note: part of the code below is from example------
	
	//setting up socket for ring master
	int status;
	int socket_fd;
	struct addrinfo host_info;
	struct addrinfo *host_info_list;
	const char *hostname = NULL;
	const char *port = argv[1];

	memset(&host_info, 0, sizeof(host_info));

	host_info.ai_family   = AF_UNSPEC;
	host_info.ai_socktype = SOCK_STREAM;
	host_info.ai_flags    = AI_PASSIVE;

	status = getaddrinfo(hostname, port, &host_info, &host_info_list);
	if (status != 0) {
		cerr << "Error: cannot get address info for host" << endl;
		cerr << "  (" << hostname << "," << port << ")" << endl;
		return -1;
	}
	socket_fd = socket(host_info_list->ai_family, 
		     host_info_list->ai_socktype, 
		     host_info_list->ai_protocol);
		
	if (socket_fd == -1) {
		cerr << "Error: cannot create socket" << endl;
		cerr << "  (" << hostname << "," << port << ")" << endl;
		return -1;
	} //if

	int yes = 1;
	status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
	if (status == -1) {
		cerr << "Error: cannot bind socket" << endl;
		cerr << "  (" << hostname << "," << port << ")" << endl;
		return -1;
	} //if

	status = listen(socket_fd, num_players);
	if (status == -1) {
		cerr << "Error: cannot listen on socket" << endl; 
		cerr << "  (" << hostname << "," << port << ")" << endl;
		return -1;
	} //if

	//cout << "Waiting for connection on port " << port << endl;
	int i = 0;
	
	//setting up connections to all players
	while(players.size()<num_players){
		
		struct sockaddr_storage socket_addr;
		socklen_t socket_addr_len = sizeof(socket_addr);
		int client_connection_fd;
		client_connection_fd = accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
		if (client_connection_fd == -1) {
			perror("cannot accept connection on socket: ");
			return -1;
		} //if
		
		char hoststr[NI_MAXHOST];
		char portstr[NI_MAXSERV];
		int rc = getnameinfo((struct sockaddr *)&socket_addr, socket_addr_len, hoststr, sizeof(hoststr), portstr, sizeof(portstr), NI_NUMERICHOST | NI_NUMERICSERV);
		//if (rc == 0) printf("New connection from %s\n", hoststr);
		
		
	  //send player_id and total number of players
		
		
		//receive the port of that player
		
		
		

		//printf("Peer IP address: %s\n", hoststr);
		
		Player *player = new Player;
		player->player_id = i;
		player->player_fd = client_connection_fd;
		
		send(client_connection_fd, &i, sizeof(int), 0);
		send(client_connection_fd, &num_players, sizeof(int), 0);
		
		
		int listening_port;
		recv(client_connection_fd, &listening_port, sizeof(int), 0);
		player->player_port = listening_port;
		
		
		//printf("Peer port      : %d\n", player->player_port);
		strcpy(player->player_addr, hoststr);
		players.push_back(player);
		
		cout<<"Player "<<i<<" is ready to play"<<endl;
		//cout<<"Player "<<i<<" has fd "<<player->player_fd<<endl;
		i++;
	}
	
	//send the left and right ip and port to all the players
	for(int k = 0; k < num_players; k++){
		
		if(k!= num_players-1){
			status = send(players[k]->player_fd, &players[k+1]->player_port, sizeof(int), 0);
			if(status == -1){
				perror("cant send player port:");
			}
			status = send(players[k]->player_fd, &players[k+1]->player_addr, 512, 0);
			if(status == -1){
				perror("cant send player addr:");
			}
		}
		else{
			//k == num_players-1
			status = send(players[k]->player_fd, &players[0]->player_port, sizeof(int), 0);
			if(status == -1){
				perror("cant send player port:");
			}
			status = send(players[k]->player_fd, &players[0]->player_addr, 512, 0);
			if(status == -1){
				perror("cant send player addr:");
			}
		}
	}

	
	//construct a potato according to the hops
	
	potato.num_hops = num_hops;
	potato.no_hop = -1;
	for(i = 0; i<512; i++){
		potato.trace[i] = -1;
	}
	
	if(num_hops > 0){
		
		//srand a player to start the game
		
		srand((unsigned int) time(NULL));
		int startplayer_id = rand()%num_players;
		cout<<"Ready to start the game, sending potato to player "<<startplayer_id<<endl;
		send(players[startplayer_id]->player_fd, &potato, sizeof(potato), 0);
		
		// use select to check every player's fd
		// https://blog.csdn.net/wypblog/article/details/6826286
		fd_set player_fds; 
		
		Potato result_potato;
		FD_ZERO(&player_fds);
		int max_fd = -1;
		for(int i = 0; i < num_players; i++){
			FD_SET(players[i]->player_fd, &player_fds);
			max_fd = max(max_fd, players[i]->player_fd);
			
		}
		assert(max_fd!=-1);
		status = select(max_fd+1, &player_fds, NULL, NULL, NULL);
		if(status < 0){
			perror("cannot select: ");
			exit(1);
		}
		for(int i = 0; i < num_players; i++){
			if(FD_ISSET(players[i]->player_fd, &player_fds)){
				status = recv(players[i]->player_fd, &result_potato, sizeof(result_potato), MSG_WAITALL);
				if(status == -1){
					perror("cannot get resulted potato: ");
					exit(1);
				}
				break;
			}
		}
		//print trace
		
//		cout<<"------------------------------------------\n";
//		cout<<"got a potato! Show the potato data:\n";
//		cout<<"num_hops: "<<result_potato.num_hops<<endl;
//		cout<<"no_hop: "<<result_potato.no_hop<<endl;
//		
//		cout<<"------------------------------------------\n";
		
		cout<<"Trace of potato:"<<endl;
		cout<<result_potato.trace[0];
		for(i = 1; i < num_hops; i++){
			if(result_potato.trace[i] == -1){
				cout<<"something was wrong I can feel it"<<endl;
				exit(1);
			}
			cout<<","<<result_potato.trace[i];
		}
	}

	//terminate the game
	//send a -1 hop potato to all the 
	//cout<<"sending poison_potato\n";
	Potato poison_potato;
	poison_potato.num_hops = -1;
	poison_potato.no_hop = -1;
	for(i = 0; i<512; i++){
		poison_potato.trace[i] = -1;
	}
	
	for(i = 0; i < num_players; i++){
		status = send(players[i]->player_fd, &poison_potato, sizeof(poison_potato), 0);
		if(status == -1){
			perror("cannot send the potato to terminate: ");
			exit(1);
		}

	}
	

	freeaddrinfo(host_info_list);
	close(socket_fd);
	
}