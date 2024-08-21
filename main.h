#ifndef MAIN_H
#define MAIN_H

//#define _POSIX_C_SOURCE 200112L



#include <netdb.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // For strcasecmp
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "list.h"
#include "parse.h"
#include "retrieve.h"
#include "mime.h"
#define IMAP_PORT "143"
#define BUFFER_SIZE 81940
#define CONTENT_TYPE "Content-Type"
#define MIME_VAL "1.0"
#define CONTENT_TYPE_VALUE "multipart/alternative;"
#define MAX_COMMAND_LEN 20
#define MAX_SERVER_LEN 50
#define MAX_USERNAME_LEN 50
#define MAX_PASSWORD_LEN 50
#define MAX_FOLDER_LEN 50

int resolve_address(const char *hostname, const char *port, struct addrinfo **servinfo);
int connect_to_server(struct addrinfo *servinfo);
int send_command(int sockfd, const char *command);
int read_response(int sockfd, char *buffer, size_t buffer_size);
int main(int argc, char **argv);


#endif
