#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include "rpc.h"
#include "a1_lib.h"

#define BUFSIZE 1024

void print_prompt()
{
    printf(">>> ");
}

Message parse_line(char *userInput)
{
    Message msg;
    int i = 0;
    char *ptr = strtok(userInput, " ");
    while (ptr != NULL)
    {
        if (i == 0)
        {
            strcpy(msg.cmd, ptr);
        }
        else if (i == 1)
        {
            strcpy(msg.arg1, ptr);
        }
        else if (i == 2)
        {
            strcpy(msg.arg2, ptr);
        }
        i++;
        ptr = strtok(NULL, " ");
    }
    return msg;
}

char *readLine()
{
    char *userInput = (char *)malloc(sizeof(char) * BUFSIZE);
    fgets(userInput, BUFSIZE, stdin);
    return userInput;
}

int main(int argc, char *argv[])
{
    int sockfd;
    char user_input[BUFSIZE] = {0};
    char server_msg[BUFSIZE] = {0};

    if (connect_to_server(argv[1], atoi(argv[2]), &sockfd) < 0)
    {
        fprintf(stderr, "oh no\n");
        return -1;
    }

    while (1)
    {
        print_prompt();
        char *userInput = readLine();
        if (strcmp(userInput, "quit\n") == 0 || strcmp(userInput, "shutdown\n") == 0)
        {
            shutdown(sockfd, SHUT_RDWR);
            break;
        }
        Message message = parse_line(userInput);
        char *msg = (char *)&message;
        send_message(sockfd, msg, sizeof(message));
        ssize_t byte_count = recv_message(sockfd, server_msg, sizeof(server_msg));
        if (byte_count <= 0)
        {
            break;
        }
        printf("%s\n", server_msg);
        free(userInput);
    }
    printf("Goodbye!\n");

    return 0;
}