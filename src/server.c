#include <netinet/in.h>
#include <syslog.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include "../includes/server.h"
#include "../includes/connections.h"
#include "../includes/configparser.h"
#include "../includes/processhttp.h"

int socketfd, newSocketfd;
Http_request *http_request;
sem_t *sem;

/*
 * Function in which every child listens to the incoming connections from clients
 * @authors Luis Lepore, Oriol Julián
 * 
 * @param socketfd number of the socket listening
 * @param server_root root directory of the server media
 * @param server_signature name of the server
 */
void child_main(int socketfd, char *server_root, char *server_signature);

/*
 * Handler called when a SIGINT signal is sent 
 * to a process of this program.
 * It frees the resources used and exits.
 * @authors Luis Lepore, Oriol Julián
 * 
 * @param sig number of the signal called (SIGINT)
 */
void sigint_handler(int sig)
{
  syslog(LOG_INFO, "SIGINT caught, terminating process");
  free(http_request);
  munmap(sem, sizeof(sem_t));
  close(newSocketfd);
  close(socketfd);
  exit(EXIT_SUCCESS);
}

void child_main(int socketfd, char *server_root, char *server_signature)
{

  http_request = malloc(sizeof(Http_request));
  while(1){
    sem_wait(sem);
    newSocketfd = accept_connection(socketfd);
    sem_post(sem);
    if(newSocketfd == -1){
      continue;
    }
    process_request(newSocketfd, http_request, server_root, server_signature);
    close(newSocketfd);
  }
}

/*
 * main function of the server.
 * @authors Luis Lepore, Oriol Julián
 * 
 * @return 0 or other number if an error ocurred
 */
int main(void)
{
  pid_t pid;
  char server_root[MAXLINE], server_signature[MAXLINE];
  int max_clients, listen_port;

  sem = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

  if (signal(SIGINT, sigint_handler) == SIG_ERR){
    syslog(LOG_ERR, "Error creating sigint handler");
    munmap(sem, sizeof(sem_t));
    exit(EXIT_FAILURE);
  }

  if (server_conf(server_root, &max_clients, &listen_port, server_signature) == -1){
    syslog(LOG_ERR, "Error opening server.conf");
    munmap(sem, sizeof(sem_t));
    exit(EXIT_FAILURE);
  }

  socketfd = initiate_server(listen_port, max_clients);
  if (socketfd == -1){
    munmap(sem, sizeof(sem_t));
    exit(EXIT_FAILURE);
  }

  sem_init(sem, 1, 1);
  for (int i = 0; i < NUM_CHILDS; i++){
    if ((pid = fork()) == 0){
      child_main(socketfd, server_root, server_signature);
    } else if(pid < 0){
      syslog(LOG_ERR, "Fork error");
      munmap(sem, sizeof(sem_t));
      close(socketfd);
      exit(EXIT_FAILURE);
    }
  }

  close(socketfd);
  while(1);
}
