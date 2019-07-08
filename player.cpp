#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <vector>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/time.h>
#include "potato.h"

using namespace std;




int main(int argc, char *argv[]) {
	//player <machine_name> <port_num>
	if(argc!=3){
		cout << "player <machine_name> <port_num>\n" << endl;
		return 1;
	}
	
	int status;
	int master_socket_fd;
	struct addrinfo host_info;
	struct addrinfo *host_info_list;
	
	const char *hostname = argv[1];
	int port_num = atoi(argv[2]);
	if(port_num<=1024||port_num>65535){
		cout << "Port already in used or not valid\n" << endl;
		exit(1);
	}
	const char *port = argv[2];
	


	memset(&host_info, 0, sizeof(host_info));
	host_info.ai_family   = AF_UNSPEC;
	host_info.ai_socktype = SOCK_STREAM;

	status = getaddrinfo(hostname, port, &host_info, &host_info_list);
	if (status != 0) {
		cerr << "Error: cannot get address info for host" << endl;
		cerr << "  (" << hostname << "," << port << ")" << endl;
		return -1;
	} //if

	master_socket_fd = socket(host_info_list->ai_family, 
		     host_info_list->ai_socktype, 
		     host_info_list->ai_protocol);
	if (master_socket_fd == -1) {
		cerr << "Error: cannot create socket" << endl;
		cerr << "  (" << hostname << "," << port << ")" << endl;
		return -1;
	} //if
	
	
	status = connect(master_socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
	if (status == -1) {
		perror("Error: cannot connect to socket");
		return -1;
	} //if
	//receive no of playser...
	int player_id;
	int player_num;
	recv(master_socket_fd, &player_id, sizeof(int), 0);
	recv(master_socket_fd, &player_num, sizeof(int), 0);
	
	cout << "Connected as player " << player_id << " out of " << player_num << " total players" << endl;
	
	
	//---------------------------------------------------------------------------------------------------------------
	//find a port(as a server..) that is currently free
	//https://www.linuxquestions.org/questions/linux-server-73/c-code-to-get-next-free-port-4175428177/
	
	
	int player_socket_fd;
	int left_player_socket_fd;
	int right_player_socket_fd;
	int player_port = -1;
	
	struct addrinfo host_info_player_server;
	struct addrinfo *host_info_player_server_list;
	const char *hostname_player = NULL;
	
	

	memset(&host_info_player_server, 0, sizeof(host_info_player_server));

	host_info_player_server.ai_family   = AF_UNSPEC;
	host_info_player_server.ai_socktype = SOCK_STREAM;
	host_info_player_server.ai_flags    = AI_PASSIVE;

	for(player_port = 1025; player_port<65536; player_port++){
		string temp_port = to_string(player_port);
		const char *port     = temp_port.c_str();
		
		status = getaddrinfo(hostname_player, port, &host_info_player_server, &host_info_player_server_list);

		
			

		player_socket_fd = socket(host_info_player_server_list->ai_family, 
			     host_info_player_server_list->ai_socktype, 
			     host_info_player_server_list->ai_protocol);
		if (player_socket_fd == -1) {
			cerr << "Error: cannot create player_socket_fd socket" << endl;
			return -1;
		} //if

		int yes = 1;
		status = setsockopt(player_socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
		status = bind(player_socket_fd, host_info_player_server_list->ai_addr, host_info_player_server_list->ai_addrlen);
		if (status == -1) {
			//cout<<"this port is already in use!";
			
			continue;
		} //if
		else{
			break;
		}
	}
	

	status = listen(player_socket_fd, 100);
	if (status == -1) {
		cerr << "Error: cannot listen on socket" << endl; 
		return -1;
	} //if


	
	status = send(master_socket_fd, &player_port, sizeof(int), 0);
	if(status == -1){
			perror("Cannot send player_port : ");
			exit(1);
		}

	// receive right info
	int port_to_connect;
	char right_addr[512];
	
	status = recv(master_socket_fd, &port_to_connect, sizeof(int), 0);
	//cout<<"recieved size(port) is : "<< status<<endl;
	if(status == -1){
		perror("Cannot receive port_to_connect : ");
		exit(1);
	}

	status = recv(master_socket_fd, right_addr, 512, 0);
	//cout<<"recieved size(addr) is : "<< status<<endl;
	if(status == -1){
		perror("Cannot receive right_addr : ");
		exit(1);
	}

	string port_right_tmp = to_string(port_to_connect);
	const char *right_port     = port_right_tmp.c_str();
	//connect to right_addr and port_to_connect
	
	struct addrinfo host_info_right;
	struct addrinfo *host_info_right_list;
	
	memset(&host_info_right, 0, sizeof(host_info_right));
	host_info_right.ai_family   = AF_UNSPEC;
	host_info_right.ai_socktype = SOCK_STREAM;

	status = getaddrinfo(right_addr, right_port, &host_info_right, &host_info_right_list);
	if (status != 0) {
		cerr << "Error: cannot get address info for host right" << endl;
		cerr << "  (" << right_addr << "," << right_port << ")" << endl;
		return -1;
	} //if

	right_player_socket_fd = socket(host_info_right_list->ai_family, 
		     host_info_right_list->ai_socktype, 
		     host_info_right_list->ai_protocol);
	if (right_player_socket_fd == -1) {
		cerr << "Error: cannot create socket: right_player_socket_fd" << endl;
		return -1;
	} //if
	
//	cout << "Connecting to " << right_addr << " on port " << right_port << "..." << endl;
	
	status = connect(right_player_socket_fd, host_info_right_list->ai_addr, host_info_right_list->ai_addrlen);
	if (status == -1) {
		perror("Error: cannot connect to right: " );
		return -1;
	} //if

	
	struct sockaddr_storage socket_addr;
	socklen_t socket_addr_len = sizeof(socket_addr);

	left_player_socket_fd = accept(player_socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
	if (left_player_socket_fd == -1) {
		cerr << "Error: cannot accpet from left player" << endl;
		return -1;
	}


	
	// receive potato
	
	srand((unsigned int) time(NULL) + player_id);
	
	while(true){
		
		fd_set two_player_one_master_fds;
			
		FD_ZERO(&two_player_one_master_fds);
		FD_SET(master_socket_fd, &two_player_one_master_fds);
		FD_SET(right_player_socket_fd, &two_player_one_master_fds);
		FD_SET(left_player_socket_fd, &two_player_one_master_fds);
		int max_fd = -1;
		max_fd = max(master_socket_fd, max(left_player_socket_fd, right_player_socket_fd))+1;
		assert(max_fd!=-1);
	
		Potato received_potato;
		//bool received = false;
		status = select(max_fd, &two_player_one_master_fds, NULL, NULL, NULL);
		if(status < 0){
			perror("cannot select: ");
			exit(1);
		}
		if(FD_ISSET(master_socket_fd, &two_player_one_master_fds)){
			status = recv(master_socket_fd, &received_potato, sizeof(received_potato), MSG_WAITALL);
			if(status == -1){
				perror("cannot recieve potato from master: ");
				exit(1);
			}
			//received = true;
		}
		else if(FD_ISSET(right_player_socket_fd, &two_player_one_master_fds)){
			status = recv(right_player_socket_fd, &received_potato, sizeof(received_potato), MSG_WAITALL);
			if(status == -1){
				perror("cannot recieve potato from right: ");
				exit(1);
			}
			//received = true;
		}
		else if(FD_ISSET(left_player_socket_fd, &two_player_one_master_fds)){
			status = recv(left_player_socket_fd, &received_potato, sizeof(received_potato), MSG_WAITALL);
			if(status == -1){
				perror("cannot recieve potato from left: ");
				exit(1);
			}
			//received = true;
		}
		
		
		
		
		
//		cout<<"------------------------------------------\n";
//		cout<<"got a potato! Show the potato data:\n";
//		cout<<"num_hops: "<<received_potato.num_hops<<endl;
//		cout<<"no_hop: "<<received_potato.no_hop<<endl;
//		cout<<"current trace is: ";
//		for(int i = 0; i < received_potato.num_hops; i++ ){
//			cout<<received_potato.trace[i]<<" ";
//		}
//		cout<<endl;
//		cout<<"------------------------------------------\n";
		if(received_potato.num_hops == -1){
			break;
		}
		
		else{
			received_potato.no_hop++;
			
			
			assert(received_potato.no_hop>=0);
			received_potato.trace[received_potato.no_hop] = player_id;
			
			
			
			if(received_potato.no_hop == received_potato.num_hops-1){
				cout<<"I’m it\n";
				//send it to master
				status = send(master_socket_fd, &received_potato, sizeof(received_potato), 0);
				if(status == -1){
					perror("cannot send the potato to master: ");
					exit(1);
				}
			}
			else{
				
				//send it to left or right
				
				int send_to_left = rand()%2;
				if(send_to_left == 1){
					status = send(left_player_socket_fd, &received_potato, sizeof(received_potato), 0);
					if(status == -1){
						perror("cannot send the potato to left_player_socket_fd: ");
						exit(1);
					}
					int next_player;
					if(player_id == 0)
						next_player= player_num-1;
						
					else{
						next_player = player_id-1;
					}
					cout<<"Sending potato to "<<next_player<<endl;
				}
				else if(send_to_left == 0){
					status = send(right_player_socket_fd, &received_potato, sizeof(received_potato), 0);
					if(status == -1){
						perror("cannot send the potato to right_player_socket_fd: ");
						exit(1);
					}
					int next_player;
					if(player_id == player_num-1)
						next_player= 0;
						
					else{
						next_player = player_id+1;
					}
					cout<<"Sending potato to "<<next_player<<endl;
				}
				else{
					cout<<"This should not happen!";
					exit(1);
				}
			}
		}	
	}
	
	freeaddrinfo(host_info_list);
	close(player_socket_fd);
	close(left_player_socket_fd);
	close(right_player_socket_fd);
	close(master_socket_fd);

}