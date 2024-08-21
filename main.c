#include "main.h"

// Resolves the address of the server using hostname and port
int resolve_address(const char *hostname, const char *port, struct addrinfo **servinfo) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // Try IPv6 and ipv4
    hints.ai_socktype = SOCK_STREAM;

    int s = getaddrinfo(hostname, port, &hints, servinfo);
    if (s!= 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        return -1;
    }
    return 0;
}

// Connects to the server using the resolved address
int connect_to_server(struct addrinfo *servinfo) {
    int sockfd = -1;
    for (struct addrinfo *rp = servinfo; rp!= NULL; rp = rp->ai_next) {
        sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sockfd == -1) {
            continue;
        }
        if (connect(sockfd, rp->ai_addr, rp->ai_addrlen)!= -1) {
            break; // success
        }
        close(sockfd);
    }
    return sockfd;
}

// Sends a command to the server
int send_command(int sockfd, const char *command) {
    if (write(sockfd, command, strlen(command))!= strlen(command)) {
        perror("write");
        return -1;
    }
    return 0;
}

// Reads the response from the server
int read_response(int sockfd, char *buffer, size_t buffer_size) {
    int n = read(sockfd, buffer, buffer_size - 1);
    if (n < 0) {
        perror("read");
        return -1;
    }
    buffer[n] = '\0';
    return n;
}

int main(int argc, char **argv) {
    struct addrinfo *servinfo;
    int sockfd;
    char* buffer = malloc(BUFFER_SIZE);
    char* folder_buffer = malloc(BUFFER_SIZE);
    char tag[10]; // unique tag for each command
    char username[MAX_USERNAME_LEN] = "";
    char password[MAX_PASSWORD_LEN] = "";
    char folder[MAX_FOLDER_LEN];
    int messageNum = -258;
    //int isText = 0; // 0 for False, 1 for True only for stretch
    char command[MAX_COMMAND_LEN] = "";
    char server_name[MAX_SERVER_LEN] = "";

    // Parsing command line arguments
    for (int i = 1; i < argc; i++) {
        if (strstr(argv[i], "$")) {
            free(buffer);
            return EXIT_FAILURE;
        }
        if (strcmp(argv[i], "-u") == 0) {
            if (i+4>argc){
                free(buffer);
                return EXIT_FAILURE;
            }
            if (strstr(argv[++i], " ")){
                strncpy(folder, "\"", MAX_FOLDER_LEN);
                strncat(username, argv[i], MAX_USERNAME_LEN - strlen(folder));
                strncat(folder, "\"", MAX_FOLDER_LEN - strlen(folder));
            }
            else {
                strncpy(username, argv[i], MAX_USERNAME_LEN);
            }
        } else if (strcmp(argv[i], "-p") == 0) {
            if (i+4>argc){
                free(buffer);
                return EXIT_FAILURE;
            }
            if (strstr(argv[++i], " ")){
                strncpy(folder, "\"", MAX_FOLDER_LEN);
                strncat(password, argv[i], MAX_PASSWORD_LEN - strlen(folder));
                strncat(folder, "\"", MAX_FOLDER_LEN - strlen(folder));
            }
            else {
                strncpy(password, argv[i], MAX_PASSWORD_LEN);
            }
        } else if (strcmp(argv[i], "-f") == 0) {
            if (i+4>argc || strlen(argv[i+1]) == 0){
                free(buffer);
                return EXIT_FAILURE;
            }
            if (strstr(argv[++i], " ")){
                strncpy(folder, "\"", MAX_FOLDER_LEN);
                strncat(folder, argv[i], MAX_FOLDER_LEN - strlen(folder));
                strncat(folder, "\"", MAX_FOLDER_LEN - strlen(folder));
            }
            else {
                strncpy(folder, argv[i], MAX_FOLDER_LEN);
            }
        } else if (strcmp(argv[i], "-n") == 0) {
            if (i+4>argc){
                free(buffer);
                return EXIT_FAILURE;
            }
            messageNum = atoi(argv[++i]);
        } /*else if (strcmp(argv[i], "-t") == 0) {
            isText = 1; // only for stretch
        }*/ else if (strcmp(argv[i], "retrieve") == 0 || strcmp(argv[i], "parse") == 0 || strcmp(argv[i], "mime") == 0 || strcmp(argv[i], "list") == 0) {
            strncpy(command, argv[i], MAX_COMMAND_LEN);
        } else {
            strncpy(server_name, argv[i], MAX_SERVER_LEN);
        }
    }

    // checking if compulsory values have been provided
    if (strlen(username) == 0 || strlen(password) == 0) {
        free(buffer);
        return EXIT_FAILURE;
    }

    //checking n value and weather it is correct
    if(messageNum < 1 && messageNum != -258) {
        free(buffer);
        return EXIT_FAILURE;
    }
    
    // Resolving server address
    if (resolve_address(server_name, IMAP_PORT, &servinfo)!= 0) {
        free(buffer);
        return EXIT_FAILURE;
    }

    // Connecting to the server
    sockfd = connect_to_server(servinfo);
    if (sockfd == -1) {
        printf("%s", "client: failed to connect\n");
        free(buffer);
        return EXIT_FAILURE;
    }

    freeaddrinfo(servinfo);

    // LOGIN command
    sprintf(tag, "A%03d", 1);
    char login_command[256];
    sprintf(login_command, "%s LOGIN %s %s\r\n", tag, username, password);

    if (send_command(sockfd, login_command)!= 0) {
        free(buffer);
        return EXIT_FAILURE;
    }

    // Read server response for login
    if (read_response(sockfd, buffer, BUFFER_SIZE) < 0) {
        free(buffer);
        return EXIT_FAILURE;
    }

    if (read_response(sockfd, buffer, BUFFER_SIZE) < 0) {
        free(buffer);
        return EXIT_FAILURE;
    }

    // Check for login success
    for (int i = 0; i < strlen(buffer); i++) {
        buffer[i] = toupper(buffer[i]);
    }

    char *ok_response = strstr(buffer, "OK");
    if (ok_response == NULL) {
        printf("%s", "Login failure\n");
        free(buffer);
        return 3;
    }
    free(buffer);

    // CHOOSE FOLDER command
    sprintf(tag, "A%03d", 2);
    char select_command[256];
    sprintf(select_command, "%s SELECT INBOX\r\n", tag);
    if (strlen(folder) != 0) {
        sprintf(select_command, "%s SELECT %s\r\n", tag, folder);
    }
    
    if (send_command(sockfd, select_command)!= 0) {
        free(folder_buffer);
        return EXIT_FAILURE;
    }

    if (read_response(sockfd, folder_buffer, BUFFER_SIZE) < 0) {
        free(folder_buffer);
        return EXIT_FAILURE;
    }

    // Check for folder selection success
    for (int i = 0; i < strlen(folder_buffer); i++) {
        folder_buffer[i] = toupper(folder_buffer[i]);
    }

    char* ok_response_retrieve = strstr(folder_buffer, "A002");
    int read_data;
    if (ok_response_retrieve == NULL) {
        while (1){
            read_data = read_response(sockfd, folder_buffer, BUFFER_SIZE);
            if(read_data < 0){
                free(folder_buffer);
                return EXIT_FAILURE;
            }
            ok_response_retrieve = strstr(folder_buffer, "A002");
            if (ok_response_retrieve != NULL) {
                break;
            }
        }
    }
    char *ok_response_folder = strstr(folder_buffer, "OK");
    if (ok_response_folder == NULL) {
        printf("%s", "Folder not found\n");
        free(folder_buffer);
        return 3;
    }
    free(folder_buffer);

    // Execute command
    if (strcmp(command, "retrieve") == 0) {
        int ret_val = retrieve_email(sockfd, messageNum);
        return ret_val;
    }
    if (strcmp(command, "parse") == 0) {
        parse(sockfd, messageNum);
    }
    if (strcmp(command, "mime") == 0) {
        mime(sockfd, messageNum);
    }
    
    if (strcmp(command, "list") == 0) {
        int ret_val = list_mailboxes(sockfd);
        return ret_val;
    }
    close(sockfd);
    return 0;
}
