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

/**
 * Simple function to print prompt
 */
void print_prompt()
{
    printf(">>> ");
}

/**
 * Function which takes the read-in user input and converts it into a message struct
 * Returns struct message
 */
Message parse_line(char *userInput)
{
    Message msg;
    int i = 0;
    char *ptr = strtok(userInput, " "); // splits the user input by spaces
    while (ptr != NULL)
    {
        // initialize first token from strtok as command
        if (i == 0)
        {
            strcpy(msg.cmd, ptr);
        }
        // second token -> arg1
        else if (i == 1)
        {
            strcpy(msg.arg1, ptr);
        }
        // third token -> arg2
        else if (i == 2)
        {
            strcpy(msg.arg2, ptr);
        }
        // NOTE that we ignore tokens beyond the third
        i++;
        ptr = strtok(NULL, " ");
    }
    return msg;
}

/**
 * Function which reads in user input
 * Returns char pointer
 */
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

    // attempt to connect to server
    if (connect_to_server(argv[1], atoi(argv[2]), &sockfd) < 0)
    {
        fprintf(stderr, "Error in connecting to the server\n");
        return -1;
    }

    // listen for user input
    while (1)
    {
        print_prompt();               // prints prompt
        char *userInput = readLine(); // read in user input
        // compare against quit || exit,
        // in which case we shutdown the client and stop listening for input
        if (strcmp(userInput, "quit\n") == 0 || strcmp(userInput, "shutdown\n") == 0)
        {
            shutdown(sockfd, SHUT_RDWR);
            break;
        }
        // input is neither quit nor shutdown so:
        // convert input into a message
        Message message = parse_line(userInput);
        // convert message into a char pointer
        char *msg = (char *)&message;
        // send message to server
        send_message(sockfd, msg, sizeof(message));
        // receive processed message back from server
        ssize_t byte_count = recv_message(sockfd, server_msg, sizeof(server_msg));
        if (byte_count <= 0)
        {
            break;
        }
        // print server response
        printf("%s\n", server_msg);
        // free userInput
        free(userInput);
    }
    // on quit, print goodbye message
    printf("Goodbye!\n");

    return 0;
}