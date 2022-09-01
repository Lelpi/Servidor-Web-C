/*
 * Responds the client with a HTTP response based on the request
 * @authors Luis Lepore, Oriol Julián
 */
#ifndef PROCESSHTTP_H
#define PROCESSHTTP_H

#include "picohttpparser.h"

/*
 * Structure used to pack all the variables used by picohttpparser
 */
typedef struct {
  const char *method;
  size_t method_len;
  const char *url;
  size_t url_len;
  int version;
  struct phr_header headers[30];
  size_t num_headers;
}Http_request;

//Constant used to create buffers for the http response string
#define MAXLINE 1024

/*
 * Function called to parse the http request and create the adequate response
 * @authors Luis Lepore, Oriol Julián
 * 
 * @param socketfd Number of the socket through which the response will be sent
 * @param http_request structure that will have the information of the request
 * @param server_root root directory of the server media
 * @param server_signature name of the server
 */
void process_request(int socketfd, Http_request *http_request, char *server_root, char *server_signature);

#endif
