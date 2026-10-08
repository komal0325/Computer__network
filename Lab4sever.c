#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void find_student(char *reg_no, char *response)
{
    FILE *fp = fopen("students.txt", "r");

    if (fp == NULL) {
        strcpy(response, "Error opening file");
        return;
    }

    char line[BUFFER_SIZE];
    int found = 0;

    while (fgets(line, sizeof(line), fp) != NULL) {
        char *file_reg;
        char *name;
        char *branch;
        char *college;

        /* Remove newline */
        line[strcspn(line, "\n")] = '\0';

        /* Split record using | */
        file_reg = strtok(line, "|");
        name = strtok(NULL, "|");
        branch = strtok(NULL, "|");
        college = strtok(NULL, "|");

        if (file_reg && name && branch && college) {
            if (strcmp(file_reg, reg_no) == 0) {
                snprintf(response, BUFFER_SIZE,
                         "Name: %s, Branch: %s, College: %s",
                         name, branch, college);

                found = 1;
                break;
            }
        }
    }

    if (!found) {
        strcpy(response, "Data not found");
    }

    fclose(fp);
}

int main()
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    /* Create socket */
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    /* Allow reuse of address */
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
                   &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    /* Configure server address */
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    /* Bind socket */
    if (bind(server_fd, (struct sockaddr *)&address,
             sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    /* Listen */
    if (listen(server_fd, 5) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server started on port %d...\n", PORT);

    /* Accept one client */
    new_socket = accept(server_fd,
                        (struct sockaddr *)&address,
                        (socklen_t *)&addrlen);

    if (new_socket < 0) {
        perror("Accept failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Client connected.\n");

    /* Persistent communication */
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);

        int bytes_read = read(new_socket, buffer, BUFFER_SIZE - 1);

        if (bytes_read <= 0) {
            printf("Client disconnected.\n");
            break;
        }

        buffer[bytes_read] = '\0';

        printf("Client query: %s\n", buffer);

        /* Terminate connection if client sends bye */
        if (strcmp(buffer, "bye") == 0) {
            printf("Client requested termination.\n");
            break;
        }

        /* Search student record */
        find_student(buffer, response);

        /* Send response */
        send(new_socket, response, strlen(response), 0);
    }

    close(new_socket);
    close(server_fd);

    printf("Server terminated.\n");

    return 0;
}
