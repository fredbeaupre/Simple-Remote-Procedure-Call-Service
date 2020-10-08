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
#define MAX_CLIENTS 20

/**
 * Simple add function
 * Returns an int
 */
int add(int x, int y)
{
    return x + y;
}

/**
 * Simple multiply function
 * Returns an int
 */
int multiply(int x, int y)
{
    return x * y;
}

/**
 * Simple divide function
 * Check for division by zero before computing
 * Returns a float
 */
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

/**
 * Function to compute factorial of an integer
 * Note it returns a uint64_t so as to handle up to factorial 20 (20!)
 */
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

/**
 * Function to compute the result from interpreting the client's input
 * Also prepares the return message
 * Return the message to be send back to client
 */
char *compute_result(Message *msg, char *server_msg)
{
    // Message has command, arg1 and arg2 as char arrays
    // so we start by convert arg1 and arg2 to integers
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
        // Check for division by zero before doing anything with the message
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
        // which is necessary to handle case where user inputs an invalid command with
        // no additional arguments
        char *stripped_msg = strtok(msg->cmd, "\n");
        sprintf(server_msg, "%s %s%s%s %s", "Error: Command", "'", stripped_msg, "'", "not found.");
    }
    return server_msg;
}

int main(int argc, char *argv[])
{
    pid_t pid_array[MAX_CLIENTS]; // will hold the processes identifiers (max of 10)
    int rvals[MAX_CLIENTS];
    int client_counter = 0; // to keep track of the number of clients
    int sockfd, clientfd;
    int newSocket;            // will hold new connections
    char msg[BUFSIZE];        // char array to store 'raw' received message
    char server_msg[BUFSIZE]; // message to be sent back

    // create the server, check for error
    if (create_server(argv[1], atoi(argv[2]), &sockfd) < 0)
    {
        fprintf(stderr, "Error when trying to create the server\n");
        return -1;
    }
    // Print message when server created successfully
    printf("\nServer listening on %s:%s\n\n", argv[1], argv[2]);

    // listen for incoming connection requests
    while (1)
    {
        // store file descriptor associated with socket into newSocket
        // check if connection was successful
        newSocket = accept_connection(sockfd, &clientfd);
        if (newSocket < 0)
        {
            fprintf(stderr, "Error from the server in accepting the connection\n");
            return -1;
        }
        else
        {
            // print message for successful connection
            printf("Connection accepted from client with id %d\n", clientfd);
        }

        // checks if any of the clients have terminated
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            waitpid(pid_array[i], &rvals[client_counter], WNOHANG);
            if (WEXITSTATUS(rvals[i]) == 3)
            {
                printf("Shutdown signal received.\n");
                exit(0);
            }
        }

        client_counter++;
        // create a child process to handle the client
        pid_array[client_counter] = fork();
        if (pid_array[client_counter] == 0) // if true, we are in the child
        {
            close(sockfd);
            //receive message
            while (1)
            {
                memset(msg, 0, sizeof(msg));
                ssize_t byte_count = recv_message(clientfd, msg, BUFSIZE);
                if (byte_count <= 0)
                {
                    exit(1);
                }

                // from char * to message *
                Message *message = (Message *)msg;
                printf("Input command: %s\n", message->cmd); // for troubleshooting
                if ((strcmp(message->cmd, "shutdown\n")) != 0)
                {
                    strcpy(server_msg, compute_result(message, server_msg));
                    // after receieved message is processed and return message is created,
                    // send return message back to client
                    send_message(clientfd, server_msg, sizeof(server_msg));
                }
                else
                {
                    sprintf(server_msg, "Shutting down...");
                    send_message(clientfd, server_msg, sizeof(server_msg));
                    exit(3);
                }
            }
        }
        else
        {
            waitpid(pid_array[client_counter], &rvals[client_counter], WNOHANG);
            client_counter++;
        }
    }
}
