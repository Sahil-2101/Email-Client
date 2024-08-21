#include "parse.h"


void parse(int sockfd, int messageNum) {
    char parse_command[BUFFER_SIZE];
    char response[BUFFER_SIZE * 2] = {0}; // To handle multi-line responses

    // Construct the IMAP FETCH command
    if (messageNum == 0) {
        snprintf(parse_command, sizeof(parse_command), "A003 FETCH * BODY.PEEK[HEADER.FIELDS (FROM TO DATE SUBJECT)]\r\n");
    } else {
        snprintf(parse_command, sizeof(parse_command), "A003 FETCH %d BODY.PEEK[HEADER.FIELDS (FROM TO DATE SUBJECT)]\r\n", messageNum);
    }

    // Send the command to the server
    if (send_command(sockfd, parse_command) != 0) {
        fprintf(stderr, "Error sending command\n");
        return;
    }

    // Read the server's response
    while (read_response(sockfd, response, sizeof(response)) > 0) {
        // If the response contains the email headers, parse them
        if (strstr(response, "A003 OK") != NULL) {
            parse_email_headers(response);
            return;
        }
    }

    fprintf(stderr, "Error reading response\n");
}


void parse_email_headers(char* buffer_parse) {
    
        char *from = NULL;
        char *to = NULL;
        char *date = NULL;
        char *subject = NULL;
        char *line = strtok(buffer_parse, "\r\n");
        while (line != NULL) {
            // Ensure line is not empty (could be a body line)
            if (line[0] == '\0') {
                break; // Break if an empty line is encount
            }
            
            // Find the delimiter between header name and value
            char *delimiter = strchr(line, ':');
            if (delimiter != NULL) {
                *delimiter = '\0'; // Null-terminate to separate header name and value

                char *value = delimiter + 1;

                // Check and assign header values
                if (strcasecmp(line, HEADER_FROM) == 0) {
                    from = value;
                } else if (strcasecmp(line, HEADER_TO) == 0) {
                    to = value;
                } else if (strcasecmp(line, HEADER_DATE) == 0) {
                    date = value;
                } else if (strcasecmp(line, HEADER_SUBJECT) == 0) {
                    char *next_line = strtok(NULL, "\r\n");
                    while (next_line != NULL && (*next_line == ' ' || *next_line == '\t')) {
                        // Concatenate with the current subject
                        strcat(value, "");
                        strcat(value, next_line);
                        next_line = strtok(NULL, "\r\n");
                    }
                    subject = value;
                    line = next_line; // Move to the next line after subject continuation
                    continue;
                }
            }

            line = strtok(NULL, "\r\n");
        }
        printf("From:%s\n", from ? from : "");
        if (to) {
            printf("To:%s\n", to);
        } else {
            printf("To:\n");
        }
        printf("Date:%s\n", date ? date : "");
        printf("Subject:%s\n", subject ? subject : " <No subject>");
}
