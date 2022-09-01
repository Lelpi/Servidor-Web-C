#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <time.h>
#include "../includes/processhttp.h"
#include "../includes/picohttpparser.h"

//Used for the length of fixed strings
#define WORDLENGTH 100
//Size of the array of file extensions
#define SIZE_ARRAY 13
//Max number of parameters a get or post request can have
#define MAX_PARAMS 10

/*
 * Structure containing a termination(sufix)
 * and the filetype of those files
 */
typedef struct {
  char sufix[10];
  char content_type[20];
}Term;

/*
 * Parameter structure. 
 * It has the name of the variable and its value
 */
typedef struct {
  char var[50];
  char value[50];
}Parameter;

/*
 * Array containing every Term of the file extensions the server supports
 */
static Term extensions[SIZE_ARRAY]={{".txt", "text/plain"},
                                    {".html", "text/html"},
                                    {".htm", "text/html"},
                                    {".gif", "image/gif"},
                                    {".jpeg", "image/jpeg"},
                                    {".jpg", "image/jpeg"},
                                    {".mpeg", "video/mpeg"},
                                    {".mpg", "video/mpeg"},
                                    {".doc", "application/msword"},
                                    {".docx", "application/msword"},
                                    {".pdf", "application/pdf"},
                                    {".py", "text/plain"},
                                    {".php", "text/plain"},
};

/*
 * Parses the parameters found in string_params in the array of Parameter params
 * @authors Luis Lepore, Oriol Julián
 * 
 * @param string_params string with the parameters stored
 * @param params array that will have all the parameters 
 * stored after the execution of the function
 */
void complete_params(char *string_params, Parameter *params);

/*
 * Executes a script and stores the output
 * @authors Luis Lepore, Oriol Julián
 * 
 * @param extension termination of the file to be executed
 * @param url location of the file
 * @param params array of arguments for the execution
 * @param script_string string in which the output will be stored
 * @return 0. -1 if an error ocurred
 */
int execute_script(char *extension, char *url, Parameter *params, char *script_string);

/*
 * Sends the content of a file
 * @authors Luis Lepore, Oriol Julián
 * 
 * @param socketfd Number of the socket through which the response will be sent
 * @param url location of the file
 */
void send_body(int socketfd, char *url);

/*
 * It creates the header of the request
 * @authors Luis Lepore, Oriol Julián
 * 
 * @param buffer string in which the header will be stored
 * @param server_signature name of the server
 * @param content_type format of the body returned in the response
 * @param content_length length of the body
 */
void create_header(char *buffer, char *server_signature, char *content_type, int content_length);

/*
 * Sends an OPTIONS response to the client
 * @authors Luis Lepore, Oriol Julián
 * 
 * @param socketfd Number of the socket through which the response will be sent
 * @param request structure that will have the information of the request
 * @param server_signature name of the server
 */
void process_options(int socketfd, Http_request *request, char *server_signature);

/*
 * Sends a GET response to the client
 * @authors Luis Lepore, Oriol Julián
 * 
 * @param socketfd Number of the socket through which the response will be sent
 * @param request structure that will have the information of the request
 * @param server_signature name of the server
 * @param url url of the resource requested
 */
void process_get(int socketfd, Http_request *request, char *server_signature, char *url);

/*
 * Sends a POST response to the client
 * @authors Luis Lepore, Oriol Julián
 * 
 * @param socketfd Number of the socket through which the response will be sent
 * @param request structure that will have the information of the request
 * @param server_signature name of the server
 * @param url url of the resource requested
 * @param body content of the http request
 */
void process_post(int socketfd, Http_request *request, char *server_signature, char *url, char *body);

/*
 * Sends an error 500 response to the client, being this an error of the server
 * @authors Luis Lepore, Oriol Julián
 * 
 * @param socketfd Number of the socket through which the response will be sent
 * @param request structure that will have the information of the request
 * @param server_signature name of the server
 */
void process_500(int socketfd, Http_request *request, char *server_signature);

/*
 * Sends an error 404 response to the client, sent when the file requested is not found
 * @authors Luis Lepore, Oriol Julián
 * 
 * @param socketfd Number of the socket through which the response will be sent
 * @param request structure that will have the information of the request
 * @param server_signature name of the server
 */
void process_404(int socketfd, Http_request *request, char *server_signature);

/*
 * Sends an error 404 response to the client, sent when the request syntax is wrong
 * @authors Luis Lepore, Oriol Julián
 * 
 * @param socketfd Number of the socket through which the response will be sent
 * @param request structure that will have the information of the request
 * @param server_signature name of the server
 */
void process_400(int socketfd, Http_request *request, char *server_signature);

/*
 * Parses the request using picohttpparser library and creates the adequate response
 * @authors Luis Lepore, Oriol Julián
 * 
 * @param socketfd Number of the socket through which the response will be sent
 * @param request structure that will have the information of the request
 * @param server_root root directory of the server media
 * @param server_signature name of the server
 */
void process_request(int socketfd, Http_request *request, char *server_root, char *server_signature){
  char buffer[MAXLINE];
  int len=0, ret, request_ret = 0;
  char method[WORDLENGTH] = "", url[WORDLENGTH] = "", aux[WORDLENGTH] = "/index.html";
  memset(buffer, 0, MAXLINE);

  syslog(LOG_INFO, "New access");
  while(request_ret <= 0){
    ret = recv(socketfd, buffer+len, MAXLINE-len, 0);

    if(ret == -1){
      syslog(LOG_ERR, "Error reading from socket");
      return;
    }

    request->num_headers=sizeof(request->headers)/sizeof(request->headers[0]);
    request_ret = phr_parse_request(buffer, len+ret, &(request->method), &(request->method_len),
		      &(request->url), &(request->url_len), &(request->version),
		      request->headers, &(request->num_headers), len);

    if (request_ret == -1){
      process_400(socketfd, request, server_signature);
      syslog(LOG_ERR, "Error phr_parse");
      return;
    }
    len += ret;
    if(len >= MAXLINE){
      syslog(LOG_ERR, "Package too large");
      return;
    }
  }

  syslog(LOG_INFO, "Request parsed");

  sprintf(method, "%.*s", (int)request->method_len, request->method);
  if(request->url_len > 1 || request->url[0] != '/'){
    sprintf(aux, "%.*s", (int)request->url_len, request->url);
  }
  sprintf(url, "%s%s", server_root, aux);

  if (strcmp(method, "GET") == 0){
    process_get(socketfd, request, server_signature, url);
  }else if(strcmp(method, "POST") == 0){
    process_post(socketfd, request, server_signature, url, strrchr(buffer, '\n')+1);
  }else if(strcmp(method, "OPTIONS") == 0){
    process_options(socketfd, request, server_signature);
  } else {
    process_400(socketfd, request, server_signature);
  }
  return;
}


void complete_params(char *string_params, Parameter *params){

  int j = 0;
  char *param_aux;

  param_aux = strtok(string_params, "&");
  strcpy(params[j].var, param_aux);
  while((param_aux = strtok(NULL, "&")) != NULL){
    j++;
    strcpy(params[j].var, param_aux);
  }

  strcpy(params[j+1].var, "");
  while(j >= 0){
    strtok(params[j].var, "=");
    strcpy(params[j].value, strtok(NULL, "="));
    j--;
  }
}

/*
 * Used to process a petition and create an answer 
 * from both a post and get request
 * @authors Luis Lepore, Oriol Julián
 * 
 * @param socketfd Number of the socket through which the response will be sent
 * @param http_request structure that will have the information of the request
 * @param server_signature name of the server
 * @param url url of the resource requested
 * @param params array of params of the request
 */
void get_post(int socketfd, Http_request *request, char *server_signature, char *url, Parameter *params){
  char buffer[MAXLINE] = "", aux[WORDLENGTH] = "", script_string[MAXLINE] = "";
  time_t time_var = time(NULL);
  int i, j;
  struct stat s;
  
  strcpy (aux, url);
  if(stat(strtok(aux, "?"), &s) != 0){
    syslog(LOG_ERR, "The file does not exist");
    process_404(socketfd, request, server_signature);
    return;
  }
  
  for (i = 0; url[i] != '?' && i < strlen(url); i++);
  for (i--; url[i] != '.' && i > 0; i--);

  for (j = 0; j < SIZE_ARRAY; j++){
    if (strncmp(&url[i], extensions[j].sufix, strlen(extensions[j].sufix)) == 0){
      break;
    }
  }

  if(j == SIZE_ARRAY){
    syslog(LOG_ERR, "Extension %s not supported", &url[i]);
    process_404(socketfd, request, server_signature);
  }
  
  sprintf(buffer, "HTTP/1.%d 200 OK\r\n", request->version);
  sprintf(aux, "Last-Modified: %.*s\r\n", (int) strlen(ctime(&s.st_mtime)) - 1, ctime(&s.st_mtime));
  strcat(buffer, aux);

  if (strcmp(extensions[j].sufix, ".py") == 0 || strcmp(extensions[j].sufix, ".php") == 0){

    if (execute_script(extensions[j].sufix, strtok(url, "?"), params, script_string) == -1){
      process_500(socketfd, request, server_signature);
      return;
    }
    create_header(buffer, server_signature, extensions[j].content_type, strlen(script_string));
    send(socketfd, buffer, strlen(buffer), 0);
    send(socketfd, script_string, strlen(script_string), 0);
    syslog(LOG_INFO, "HTTP %s executable sent", request->method);
  } else {
    create_header(buffer, server_signature, extensions[j].content_type, (int)s.st_size);
    send(socketfd, buffer, strlen(buffer), 0);
    send_body(socketfd, url);
    syslog(LOG_INFO, "HTTP %s non-executable sent", request->method);
  }
}

void process_get(int socketfd, Http_request *request, char *server_signature, char *url){
  char *string_params = NULL;
  Parameter params[MAX_PARAMS+1];
  int i;

  for (i = 0; url[i] != '?' && i < strlen(url); i++);
  if (i != strlen(url)){
    string_params = &url[i] + 1;
    complete_params(string_params, params);
  } else {
    strcpy(params[0].var, "");
  }

  get_post(socketfd, request, server_signature, url, params);
  return;
}

void process_post(int socketfd, Http_request *request, char *server_signature, char *url, char *body){
  Parameter params[MAX_PARAMS+1];

  if (strlen(body)){
    complete_params(body, params);
  } else {
    strcpy(params[0].var, "");
  }

  get_post(socketfd, request, server_signature, url, params);
  return;
}

int execute_script(char *extension, char *url, Parameter *params, char *script_string){
  char command[MAXLINE] = "", buffer[MAXLINE];
  int i, ret;
  FILE *f;

  if(strcmp(extension, ".py") == 0){
    sprintf(command, "python ");
  }else{
    sprintf(command, "php ");
  }

  strcat(command, url);

  for(i = 0; strcmp(params[i].var, "") != 0; i++){
    strcat(command, " ");
    strcat(command, params[i].value);
  }

  if((f = popen(command, "r")) == NULL){
    syslog(LOG_ERR, "Error executing script");
    return -1;
  }

  while(feof(f) == 0){
    if((ret = fread(buffer, sizeof(char), MAXLINE, f)) == -1){
      pclose(f);
      syslog(LOG_ERR, "Error reading file");
      return -1;
    }
    strncat(script_string, buffer, ret);
  }
  strcat(script_string, "\r\n");
  
  pclose(f);
  return 0;
}


void send_body(int socketfd, char *url){
  FILE *f = NULL, *f2 = NULL;
  char buffer[MAXLINE] = "";
  int ret;

  if((f = fopen(url, "rb")) == NULL){
    syslog(LOG_ERR, "Error opening file");
    return;
  }

  while(feof(f) == 0){
    if((ret = fread(buffer, sizeof(char), MAXLINE, f)) == -1){
      fclose(f);
      syslog(LOG_ERR, "Error reading file");
      return;
    }
    send(socketfd, buffer, ret, 0);
  }

  fclose(f);
  return;
}


void process_options(int socketfd, Http_request *request, char *server_signature){
  char buffer[MAXLINE] = "";
  time_t time_var = time(NULL);

  sprintf(buffer, "HTTP/1.%d 204 No Content\r\n", request->version);
  strcat(buffer, "Allow: OPTIONS, GET, POST\r\n");
  create_header(buffer, server_signature, "text/html", 0);
  send(socketfd, buffer, strlen(buffer), 0);
  syslog(LOG_INFO, "HTTP OPTIONS sent");

  return;
}


void process_500(int socketfd, Http_request *request, char *server_signature){
  char buffer[MAXLINE] = "";

  sprintf(buffer, "HTTP/1.%d 500 Internal Server Error\r\n", request->version);
  create_header(buffer, server_signature, "text/html", 0);
  send(socketfd, buffer, strlen(buffer), 0);
  syslog(LOG_INFO, "HTTP 500 sent");

  return;
}


void process_404(int socketfd, Http_request *request, char *server_signature){
  char buffer[MAXLINE] = "";

  sprintf(buffer, "HTTP/1.%d 404 Not Found\r\n", request->version);
  create_header(buffer, server_signature, "text/html", 0);
  send(socketfd, buffer, strlen(buffer), 0);
  syslog(LOG_INFO, "HTTP 404 sent");

  return;
}


void process_400(int socketfd, Http_request *request, char *server_signature){
  char buffer[MAXLINE] = "";

  sprintf(buffer, "HTTP/1.%d 400 Bad Request\r\n", request->version);
  create_header(buffer, server_signature, "text/html", 0);
  send(socketfd, buffer, strlen(buffer), 0);
  syslog(LOG_INFO, "HTTP 400 sent");

  return;
}

void create_header(char *buffer, char *server_signature, char *content_type, int content_length){
  char aux[WORDLENGTH]="";
  time_t time_var = time(NULL);

  sprintf(aux, "Date: %.*s\r\n", (int) strlen(ctime(&time_var)) - 1, ctime(&time_var));
  strcat(buffer, aux);
  sprintf(aux, "Server: %s\r\n", server_signature);
  strcat(buffer, aux);
  sprintf(aux, "Content-Type: %s\r\n", content_type);
  strcat(buffer, aux);
  sprintf(aux, "Content-Length: %d\r\n\r\n", content_length);
  strcat(buffer, aux);
  syslog(LOG_INFO, "HTTP header created");

  return;
}
