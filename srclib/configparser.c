#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

int server_conf(char *server_root, int *max_clients, int *listen_port, char *server_signature){
  FILE *f;
  char *line = NULL;
  size_t len = 0;
  char *clave = NULL, *valor = NULL, num[10];

  f = fopen("server.conf", "r");
  if(f == NULL){
    return -1;
  }

  while (getline(&line, &len, f) != -1){
    if(line[0] == '#'){
      continue;
    }

    clave = strtok(line, " =");
    valor = strtok(NULL, " =");

    if(strcmp(clave, "server_root") == 0){
      valor = strtok(valor, "\n");
      strcpy(server_root, valor);
    } else if(strcmp(clave, "max_clients") == 0){
      *max_clients = atoi(valor);
    } else if(strcmp(clave, "listen_port") == 0){
      *listen_port = atoi(valor);
    } else if(strcmp(clave, "server_signature") == 0){
      strcpy(num, strtok(NULL, "\n"));
      strcat(valor, " ");
      strcat(valor, num);
      strcpy(server_signature, valor);
    } else {
      fclose(f);
      free(line);
      return -1;
    }
  }

  fclose(f);
  free(line);
  return 0;
}
