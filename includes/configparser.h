/*
 * Implements the functions needed to parse
 * the configuration file
 * @authors Luis Lepore, Oriol Julián
 */
#ifndef CONFIGPARSER_H
#define CONFIGPARSER_H

/*
 * Parses the server.conf file and extracts from there the information about the server
 * @authors Luis Lepore, Oriol Julián
 * 
 * @param server_root root directory of the server media
 * @param max_clients number of maximum simultaneous clients connected to the server
 * @param listen_port port opened for the client to access in the server
 * @param server_signature name of the server
 * @return 0 if everything worked. -1 if an error ocurred
 */
int server_conf(char *server_root, int *max_clients, int *listen_port, char *server_signature);

#endif
