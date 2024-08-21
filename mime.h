#ifndef MIME_H
#define MIME_H
#include "main.h"

#define MIME_VERSION_HEADER "MIME-Version: 1.0"
#define MIME_VERSION_HEADER_2 "Mime-Version: 1.0"
#define CONTENT_TYPE_HEADER "Content-Type: multipart/alternative;"
#define CONTENT_TYPE_TEXT_PLAIN "Content-Type: text/plain; charset=UTF-8"
#define CONTENT_TRANSFER_ENCODING "Content-Transfer-Encoding:"

/**
 * Fetches the MIME headers of an email and processes the response.
 * 
 * @param sockfd The socket file descriptor for the server connection.
 * @param messageNum The number of the message to fetch. If 0, fetches all messages.
 */

void mime(int sockfd, int messageNum) ;


/**
 * Parses the MIME response received to find the boundary string.
 * 
 * @param sockfd The socket file descriptor for the server connection.
 * @param messageNum The number of the message being processed.
 * @param mime_response The MIME response received from the server.
 */

void mime_parse(int sockfd, int messageNum,char* mime_response) ;

/**
 * Fetches the body of the email and processes the response.
 * 
 * @param sockfd The socket file descriptor for the server connection.
 * @param messageNum The number of the message to fetch. If 0, fetches all messages.
 * @param buffer_boundary The boundary string for multipart content.
 */

void mime_2(int sockfd, int messageNum,char *buffer_boundary) ;

/**
 * Parses the response body for content and prints the lines.
 * 
 * @param response The response received from the server.
 * @param buffer_boundary The boundary string for multipart content.
 */

void mime_parse_2(char *response, char *buffer_boundary);

#endif
