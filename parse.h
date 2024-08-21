#ifndef PARSE_H
#define PARSE_H
#include "main.h"
#define HEADER_FROM "From"
#define HEADER_TO "To"
#define HEADER_DATE "Date"
#define HEADER_SUBJECT "Subject"

/**
 * @brief Parses email headers from an IMAP server response.
 *
 * This function sends an IMAP FETCH command to the server to retrieve the email headers
 * (FROM, TO, DATE, SUBJECT) for a specified message number. If the message number is 0,
 * it fetches the headers for the most recent message. The server's response is then
 * parsed to extract and print these headers.
 *
 * @param sockfd The socket file descriptor connected to the IMAP server.
 * @param messageNum The message number to fetch. If 0, fetches the most recent message.
 */

void parse(int sockfd, int messageNum) ;


/**
 * @brief Parses the email headers from the server response buffer.
 *
 * This function takes the buffer containing the server's response and extracts the
 * headers (FROM, TO, DATE, SUBJECT). It handles multi-line subjects properly by
 * concatenating continued lines. The parsed headers are then printed.
 *
 * @param buffer_parse The buffer containing the server's response with email headers.
 */

void parse_email_headers(char* buffer_parse);


#endif
