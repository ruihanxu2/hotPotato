This is an inplementaion of the hot potato game using POSIX sockets. The assignment is to create one ringmaster process and some number of player processes,
then play a game and terminate all the processes gracefully. You may explicitly create each
process from an interactive shell; however, the player processes must exit cleanly at the end of
the game in response to commands from the ringmaster.

In this assignment, you will use TCP sockets as the mechanism for communication between the
ringmaster and player processes. Your programs must use exactly the command line
arguments described here. The ringmaster program is invoked as shown below, where
num_players must be greater than 1 and num_hops must be greater than or equal to zero
and less than or equal to 512 (make sure to validate your command line arguments!).
ringmaster <port_num> <num_players> <num_hops>
The player program is invoked as:
player <machine_name> <port_num>
where machine_name is the machine name (e.g. login-teer-03.oit.duke.edu) where the
ringmaster process is running and port_num is the port number given to the ringmaster
process which it uses to open a socket for player connections. If there are N players, each
player will have an ID of 0, 1, 2, to N-1. A player’s ID and other information that each player will
need to connect to their left and right neighbor can be provided by the ringmaster as part of
setting up the game. The players are connected in the ring such that the left neighbor of player i
is player i-1 and the right neighbor is player i+1. Player 0 is the right neighbor of player N-1,
and Player N-1 is the left neighbor of player 0.
Zero is a valid number of hops. In this case, the game must create the