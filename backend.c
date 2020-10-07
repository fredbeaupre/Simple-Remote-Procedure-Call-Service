#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include "a1_lib.h"
#include "rpc.h"

#define BUFSIZE 1024
#define EXIT_FAILURE 1
int rval;
pid_t childpid;

int add(int x, int y)
{
    return x + y;
}

int multiply(int x, int y)
{
    return x * y;
}

float divide(float x, float y)
{
    if (y == 0.00)
    {
        fprintf(stderr, "Error: Division by zero.");
        return EXIT_FAILURE;
    }
    else
    {
        return (float)x / y;
    }
}

uint64_t factorial(int x)
{
    uint64_t aux = 1;
    while (x > 1)
    {
        aux *= x;
        x--;
    }
    return aux;
}

char *compute_result(Message *msg, char *server_msg)
{
    int arg1 = atoi(msg->arg1);
    int arg2 = atoi(msg->arg2);

    // if input command is "add"
    if (strcmp(msg->cmd, "add") == 0)
    {
        int result = add(arg1, arg2);
        sprintf(server_msg, "%d", result);
    }
    // if input command is "multiply"
    else if (strcmp(msg->cmd, "multiply") == 0)
    {
        int result = multiply(arg1, arg2);
        sprintf(server_msg, "%d", result);
    }
    // if input command is "divide"
    else if (strcmp(msg->cmd, "divide") == 0)
    {
        // Check for division by zero before doing anything
        if (arg2 == 0)
        {
            sprintf(server_msg, "%s", "Error: Divison by zero.");
        }
        // division by zero check successful
        else
        {
            float result = divide(arg1, arg2);
            sprintf(server_msg, "%.6f", result);
        }
    }
    // if input command is "factorial"
    else if (strcmp(msg->cmd, "factorial") == 0)
    {
        uint64_t result = factorial(arg1);
        sprintf(server_msg, "%lu", result);
    }
    // if input command is "sleep"
    else if (strcmp(msg->cmd, "sleep") == 0)
    {
        sleep(atoi(msg->arg1));
        // notify client that server is back after sleep call
        sprintf(server_msg, "%s", "Server back online");
    }
    // if invalid input
    else
    {
        // strip the message from its newline terminating character
        // necessary to handle case where user inputs an invalid command with
        // no additional arguments
        char *stripped_msg = strtok(msg->cmd, "\n");
        sprintf(server_msg, "%s %s%s%s %s", "Error: Command", "'", stripped_msg, "'", "not found.");
    }
    return server_msg;
}

int main(int argc, char *argv[])
{
    int sockfd, clientfd;

    int newSocket;
    char msg[BUFSIZE];
    char server_msg[BUFSIZE];
    char temp_msg[BUFSIZE];

    if (create_server(argv[1], atoi(argv[2]), &sockfd) < 0)
    {
        fprintf(stderr, "error\n");
        return -1;
    }
    printf("\nServer listening on 127.0.0.1:1234\n\n");

    while (1)
    {

        newSocket = accept_connection(sockfd, &clientfd);
        if (newSocket < 0)
        {
            fprintf(stderr, "oh no\n");
            return -1;
        }

        printf("Connection accepted from client with id %d\n", clientfd);

        if ((childpid = fork()) == 0)
        {
            close(sockfd);
            while (1)
            {
                memset(msg, 0, sizeof(msg));
                ssize_t byte_count = recv_message(clientfd, msg, BUFSIZE);
                if (byte_count <= 0)
                {
                    exit(1);
                }

                Message *message = (Message *)msg;
                if (strcmp(message->cmd, "shutdown\n") != 0 && strcmp(message->cmd, "quit\n") != 0 && strcmp(message->cmd, "exit\n") != 0)
                {
                    strcpy(server_msg, compute_result(message, server_msg));
                }
                else if (strcmp(message->cmd, "exit\n") == 0)
                {
                    shutdown(clientfd, SHUT_RDWR);
                    close(clientfd);
                    close(sockfd);
                    exit(0);
                }
                else
                {
                    printf("HERE");
                    sprintf(server_msg, "%s", "Shutting down server");
                    exit(3);
                }

                send_message(clientfd, server_msg, sizeof(server_msg));
            }
        }
        else
        {
            waitpid(childpid, &rval, WNOHANG);
            sleep(2);
            if (WEXITSTATUS(rval) == 3)
            {
                close(sockfd);
                exit(0);
            }
        }
    }
    close(newSocket);
    exit(0);

    return 0;
}
