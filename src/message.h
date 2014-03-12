#ifndef MESSAGE_H
#define	MESSAGE_H

#include "event.h"
#include "rfc.h"

typedef struct message_data
{
    char prefix[RFC_PREFIX_MAXLENGTH+1];
    
    char command[RFC_COMMAND_MAXLENGTH+1];
    int command_numeric;
    
    int argc;
    char *argv[RFC_MAX_ARGUMENTS];
} message_data;

message_data *message_parse(char *buffer);
void message_delete(message_data *message);


typedef struct message_callback_data
{
    event_callback_data *event_data;
    message_data *message_data;
} message_callback_data;

typedef int (*message_callback_func)(message_callback_data *callback_data);

typedef struct message_handler
{
    message_callback_func callback;
    char command[RFC_COMMAND_MAXLENGTH+1];
    
    struct message_handler *next;
} message_handler;

void message_register_handler(const char *command, message_callback_func callback);
void message_dispatch_command(const char *command, message_callback_data *data);

#endif	/* MESSAGE_H */

