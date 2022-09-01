#include <syslog.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include "../includes/connections.h"

int initiate_server(int listen_port, int max_clients)
{
  int sockval;
  struct sockaddr_in Direccion;
  
  syslog(LOG_INFO, "Creating socket");
  if ((sockval = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    syslog(LOG_ERR, "Error creating socket");
    return -1;
  }

  Direccion.sin_family = AF_INET; /* TCP/IP family */
  Direccion.sin_port = htons(listen_port); /* Asigning port */
  Direccion.sin_addr.s_addr = htonl(INADDR_ANY); /* Accept all adresses */
  bzero((void *)&(Direccion.sin_zero), 8);

  syslog(LOG_INFO, "Binding socket");
  if (bind(sockval, (struct sockaddr *)&Direccion, sizeof(Direccion))<0){
    syslog(LOG_ERR, "Error binding socket");
    close(sockval);
    return -1;
  }

  syslog(LOG_INFO, "Listening connections");
  if (listen(sockval, max_clients)<0){
    syslog(LOG_ERR, "Error listening");
    close(sockval);
    return -1;
  }

  return sockval;
}

int accept_connection(int sockval)
{
  int desc, len;
  struct sockaddr Conexion;

  len = sizeof(Conexion);

  if ((desc = accept(sockval, &Conexion, &len)) < 0){
    syslog(LOG_ERR, "Error accepting connection");
    return -1;
  }

  return desc;
}
