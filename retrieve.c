#include "retrieve.h"

// Function to retrieve an email based on message number
int retrieve_email(int sockfd, int messageNum) {
    char tag[10];
    // Create a unique tag for the FETCH command
    sprintf(tag, "A%03d", 3);
    char fetch_command[256];
    char *ok_response_retrieve;
    // Prepare the FETCH command, fetch last message if messageNum is 0
    if (messageNum == 0) {
        // Fetch the last-added message in the folder
        sprintf(fetch_command, "%s FETCH * BODY.PEEK[]\r\n", tag);
    } else {
        sprintf(fetch_command, "%s FETCH %d BODY.PEEK[]\r\n", tag, messageNum);
    }
    // Send the FETCH command
    if (send_command(sockfd, fetch_command) != 0) {
        return EXIT_FAILURE;
    }

    char* buffer_retrieve = malloc(BUFFER_SIZE);
    int read_data = read_response(sockfd, buffer_retrieve, BUFFER_SIZE);
    int comb_data = 0;
    comb_data += read_data;
    // Check if read operation was successful
    if(read_data < 0){
        free(buffer_retrieve);
        return EXIT_FAILURE;
    }
    // Look for the command tag in the response
    ok_response_retrieve = strstr(buffer_retrieve, "A003");
    if (ok_response_retrieve == NULL) {
        // If the response is incomplete, continue reading
        while (1){
            read_data = read_response(sockfd, buffer_retrieve + comb_data, BUFFER_SIZE - comb_data);
            comb_data += read_data;
            if(read_data < 0){
                free(buffer_retrieve);
                return EXIT_FAILURE;
            }
            ok_response_retrieve = strstr(buffer_retrieve, "A003");
            if (ok_response_retrieve != NULL) {
                break;
            }
        }
    }
    // Check for successful completion of the FETCH command
    ok_response_retrieve = strstr(buffer_retrieve, "A003 OK");
    if (ok_response_retrieve == NULL) {
        printf("%s", "Message not found\n");
        free(buffer_retrieve);
        return 3;
    }
    // Temporary buffer for processing the response
    char* buffer_temp = malloc(BUFFER_SIZE);
    int first_enc = 0;
    int last_enc = 1;
    int trav = 0;
    // Process the response to extract the message body
    for (int i = 0; i < strlen(buffer_retrieve); i++){
        if (first_enc == 1 && last_enc == 1) {
            strcpy(&buffer_temp[trav], &buffer_retrieve[i]);
            trav++;
        }
        if (buffer_retrieve[i] == '\n' && first_enc == 0 && buffer_retrieve[i+1] != 42){
            first_enc = 1;
        }
        if (buffer_retrieve[i] == '\n' && buffer_retrieve[i+1] == 41) {
            strcpy(&buffer_temp[trav], &buffer_retrieve[-1]);
            last_enc = 0;
            break;
        }
    }
    // Print the extracted message body
    printf("%s", buffer_temp);
    free(buffer_temp);
    free(buffer_retrieve);
    return 0;
}
