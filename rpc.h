#ifndef MESSAGE_H_
#define MESSAGE_H_

#define CMD_LENGTH 256
#define ARGS_LENGTH 256

typedef struct message
{
    char cmd[CMD_LENGTH];
    char arg1[ARGS_LENGTH];
    char arg2[ARGS_LENGTH];
} Message;

#endif // MESSAGE_H_