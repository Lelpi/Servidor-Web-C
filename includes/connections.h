/*
 * Implements the functions needed to connect 
 * client with server using sockets
 * @authors Luis Lepore, Oriol Julián
 */

#ifndef CONNECTIONS_H
#define CONNECTIONS_H

/* 
 * Creates and initiates a socket in the 
 * @authors Luis Lepore, Oriol Julián
 * 
 * @param listen_port port opened by the function
 * @param max_clients maximum number of connections the server can have simultaneously
 * @return value of the socket or -1 if an error ocurred
 */
int initiate_server(int listen_port, int max_clients);

/*
 * Blocks the process until a client connects to the socket
 * @authors Luis Lepore, Oriol Julián
 * 
 * @param sockval value of the socket, returned by initiate_server
 * @return value of the socket listening to the client
 */
int accept_connection(int sockval);

#endif
