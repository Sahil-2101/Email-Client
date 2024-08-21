#include "main.h"
void mime(int sockfd, int messageNum) {
    char mime_command[BUFFER_SIZE];
    char response[BUFFER_SIZE * 2] = {0}; // To handle multi-line responses

    // Construct the IMAP FETCH command
    if (messageNum == 0) {
        snprintf(mime_command, sizeof(mime_command), "A003 FETCH * BODY.PEEK[HEADER.FIELDS (MIME-Version Content-Type)]\r\n");
    } else {
        snprintf(mime_command, sizeof(mime_command), "A003 FETCH %d BODY.PEEK[HEADER.FIELDS (MIME-Version Content-Type)]\r\n", messageNum);
    }

    // Send the command to the server
    if (send_command(sockfd, mime_command) != 0) {
        fprintf(stderr, "Error sending command\n");
        return;
    }

    // Read the server's response
    while (read_response(sockfd, response, sizeof(response)) > 0) {
        // If the response contains the email headers, parse them
        if (strstr(response, "A003 OK") != NULL) {
            mime_parse(sockfd,messageNum,response);
            return;
        }
    }
    fprintf(stderr, "Error reading response\n");
}
void mime_parse(int sockfd,int messageNum,char* mime_response) {
    char boundary[BUFFER_SIZE] = {0};
    int found_mime = 0;
    int found_content_type = 0;
    char *boundary_prefix;
    int found_bound=0;
    char *line = strtok(mime_response, "\r\n");
    while (line != NULL) {
        if (strstr(line, MIME_VERSION_HEADER) || strstr(line, MIME_VERSION_HEADER_2)) {
            found_mime = 1;
        } else if (found_mime && strstr(line, CONTENT_TYPE_HEADER)) {
            found_content_type = 1;
        } else if (found_content_type && strstr(line, "boundary=")) {
            char *boundary_start = strchr(line, '=');
            if (boundary_start) {
                boundary_start++;
                // Removing any leading whitespace or quotes
                while (*boundary_start == ' ' || *boundary_start == '"' || *boundary_start == '\'') {
                    boundary_start++;
                }
                // Removing any trailing whitespace or quotes
                char *boundary_end = boundary_start + strlen(boundary_start) - 1;
                while (*boundary_end == '"' || *boundary_end == ' ' || *boundary_end == '\n' || *boundary_end == ';' || *boundary_end == '\'') {
                    *boundary_end = '\0';
                    boundary_end--;
                }
                strcpy(boundary, boundary_start);
                found_bound = 1;
                break;
            }
        }
        line = strtok(NULL, "\r\n");
    }
    //printf("%d",found_bound);
    if (found_bound) {
        boundary_prefix = malloc(strlen(boundary) + 3);
        sprintf(boundary_prefix, "--%s", boundary);
        //printf("%s",boundary_prefix);
    } else {
        fprintf(stderr, "Boundary not found\n");
        return;
    }
    mime_2(sockfd,messageNum,boundary_prefix);
    free(boundary_prefix);
}

void mime_2(int sockfd, int messageNum,char *buffer_boundary) {
    char mime_command[BUFFER_SIZE];
    char response[BUFFER_SIZE * 2] = {0}; // To handle multi-line responses

    // Construct the IMAP FETCH command
    if (messageNum == 0) {
        snprintf(mime_command, sizeof(mime_command), "A003 FETCH * BODY.PEEK[]\r\n");
    } else {
        snprintf(mime_command, sizeof(mime_command), "A003 FETCH %d BODY.PEEK[]\r\n", messageNum);
    }

    // Send the command to the server
    if (send_command(sockfd, mime_command) != 0) {
        fprintf(stderr, "Error sending command\n");
        return;
    }

    // Read the server's response
    while (read_response(sockfd, response, sizeof(response)) > 0) {
        // If the response contains the boundary header, parse them
        if (strstr(response, buffer_boundary) != NULL) {
            mime_parse_2(response, buffer_boundary);
            return;
        }
    }
    fprintf(stderr, "Error reading response\n");
}
void mime_parse_2(char *response, char *buffer_boundary){
    int in_body = 0;
    int body_2=0;
    char *next_line;
    int body_3=0;
    int content_type_2=0;
    int content_type_3=0;
    int first_line_skipped = 0;
    char *prev_line = NULL; // to store the previous line

    char *line ;
    line = response;
    while (line != NULL && *line != '\0') {
        // Find the end of the current line
        next_line = strstr(line, "\r\n");
        if (next_line != NULL) {
            *next_line = '\0'; // Replace "\r\n" with null terminator
            next_line += 2;    // Move to the start of the next line
        }
        if (strstr(line, CONTENT_TRANSFER_ENCODING)!=NULL){
            in_body = 1;
        }
        else if(in_body && strcmp(line,CONTENT_TYPE_TEXT_PLAIN)==0){
            content_type_2=1;
        }
        else if(content_type_2){
            if (strcmp(line,buffer_boundary)==0) {
                break;  // End of this part
            }
            if (!first_line_skipped) {
                first_line_skipped = 1; // Set the flag to indicate the first line has been skipped
                line = next_line; // Move to the next line
                continue; // Skip this line
            }if (prev_line != NULL) {
                printf("%s\r\n", prev_line); // Print the previous line
            }
            prev_line = line; // Update the previous line
        }
        if(strstr(line,"Content-Type: text/plain;")){
            body_2=1;
        }else if (body_2 && strcmp(line," charset=UTF-8")==0){
            body_3=1;
        }else if (body_3 && strstr(line,CONTENT_TRANSFER_ENCODING)!=NULL){
            body_2=0;
            content_type_3=1;
        }else if(content_type_3){
            if (strcmp(line, buffer_boundary)==0) {
                break;  // End of this part
            }if (!first_line_skipped) {
                first_line_skipped = 1; // Set the flag to indicate the first line has been skipped
                line = next_line; // Move to the next line
                continue; // Skip this line
            }
            if (prev_line != NULL) {
                printf("%s\r\n", prev_line); // Print the previous line
            }
            prev_line = line; // Update the previous line
        
        
        } 
        
        // Move to the next line
        if (next_line != NULL) {
            line = next_line;
        } else {
            break;
        }
    }
}
