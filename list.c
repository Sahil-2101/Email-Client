#include "list.h"

// Function to list all mailboxes and their subjects
int list_mailboxes(int sockfd) {
    char tag[10];
    // Create a unique tag for the LIST command
    sprintf(tag, "A%03d", 3);
    char list_command[256];
    // Prepare the LIST command to fetch headers of all messages in the mailbox
    sprintf(list_command, "%s FETCH 1:* (BODY.PEEK[HEADER.FIELDS (SUBJECT)])\r\n", tag);

    // Send the LIST command
    if (send_command(sockfd, list_command) != 0) {
        return EXIT_FAILURE;
    }

    char* list_buffer = malloc(BUFFER_SIZE);
    char* ok_response = strstr(list_buffer, "A003");
    // Check if the response contains the initial part of the command tag
    if (ok_response == NULL) {
        while (1) {
            // Read the server response into the buffer
            if (read_response(sockfd, list_buffer, BUFFER_SIZE) < 0) {
                free(list_buffer);
                return EXIT_FAILURE;
            }

            // Check for the command tag in the response
            ok_response = strstr(list_buffer, "A003");
            if (ok_response != NULL) {
                break;
            }
        }
    }
    char* subject_start = strstr(list_buffer, "SUBJECT");
    int seqno = 1;
    int list_trav = 0;
    int fetch_catch = 0;
    // Check if there is any subject in the response
    if (subject_start){
        while (1) {
            // Traverse the buffer to find and print subjects
            for (int i = 0; i < strlen(list_buffer); i++){
                // Detect the beginning of a FETCH response
                if(memcmp(list_buffer + i, "FETCH", 5) == 0){
                    fetch_catch = 1;
                }
                // Detect and print the subject of the email
                if(memcmp(list_buffer + i, "Subject:", 8) == 0){
                    printf("%d", seqno);
                    seqno++;
                    list_trav = i+8;
                    // Continue until the end of the subject line
                    while(memcmp(list_buffer+list_trav, "\n", 1) != 0|| list_buffer[list_trav+1] != 41){
                        printf("%c", list_buffer[list_trav-1]);
                        list_trav++;
                        // Skip to the next line if a newline character is found
                        if (memcmp(list_buffer+list_trav, "\n", 1) == 0 && list_buffer[list_trav+1] != 41) {
                            list_trav += 2;
                        }
                    }
                    printf("\n");
                    fetch_catch = 0;
                } 
                // Handle cases where there is no subject in the FETCH response
                else if(fetch_catch == 1 && memcmp(list_buffer + i, "\n", 1) == 0 && memcmp(list_buffer + i + 1, "Subject", 7) != 0){
                    printf("%d: <No subject>\n", seqno);
                    seqno++;
                    fetch_catch = 0;
                }
                // Break the loop if the end of the response is reached
                if(memcmp(list_buffer + i, "A003", 4) == 0){
                    break;
                }
            }
            break;
        }
    }

    // Free the allocated memory for the buffer
    free(list_buffer);
    return 0;
}
