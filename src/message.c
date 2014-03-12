#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "message.h"
#include "util.h"
#include "common.h"

message_data *message_parse(char *buffer)
{
    message_data *message = malloc(sizeof(message_data));
    char *start, *c;
    int length;
    
    /* Check for prefix */
    if (*buffer == ':')
    {
        // Move to the first character of the prefix
        start = ++buffer;
        // Find fist space, everything from start to there is the prefix
        buffer = strchr(buffer, ' ');
        if (buffer == NULL)
        {
            free(message);
            return NULL;
        }
        
        length = buffer - start;
        if (length > RFC_PREFIX_MAXLENGTH)
        {
            length = RFC_PREFIX_MAXLENGTH;
        }
        strncpy(message->prefix, start, length);
        message->prefix[length] = '\0';
        
        buffer++; // Move behind the space
    }
    
    /* Parse the command */
    start = buffer;
    buffer = strchr(buffer, ' ');
    if (buffer == NULL)
    {
        buffer = strchr(start, '\0');
    }
    
    length = buffer - start;
    strncpy(message->command, start, length);
    message->command[length] = '\0';
    
    /* Turn to lowercase */
    c = message->command;
    while(*c)
    {
        *c = tolower(*c);
        c++;
    }
    
    /* Check for numeric commands */
    message->command_numeric = 0;
    if (length == 3)
    {
        // Might be numeric message
        message->command_numeric = atoi(message->command);
    }
    
    // Move behind the space, if there are any arguments
    if (*buffer != '\0')
    {
        buffer++;
    }
    
    /* Parse arguments */
    message->argc = 0;
    while(*buffer != '\0')
    {
        start = buffer;
        if (*start == ':')  
        {
            // Trailing argument
            buffer = strchr(buffer, '\0');
            start++;
        }
        else
        {
            buffer = strchr(buffer, ' ');
            if (buffer == NULL)
            {
                // Last argument, find end of string instead
                buffer = strchr(start, '\0');
            }
        }
        
        length = buffer - start;
        message->argv[message->argc] = malloc(sizeof(char)*(length+1));
        strncpy(message->argv[message->argc], start, length);
        message->argv[message->argc][length] = '\0';
        message->argc++;     
        
        // Move behind the space, if any
        if (*buffer != '\0')
        {
            buffer++;
        }
    }
    
    return message;
}

void message_delete(message_data *message)
{
    if (message == NULL)
    {
        return;
    }
    
    for(int i = 0; i < message->argc; i++)
    {
        free(message->argv[i]);
    }
    free(message);
}

static message_handler *message_handlers[MESSAGE_HANDLER_HASHTABLE_SIZE];

void message_register_handler(const char *command, message_callback_func callback)
{
    int bucket = hash(command) % MESSAGE_HANDLER_HASHTABLE_SIZE;
    message_handler *last = message_handlers[bucket];
    
    message_handler *handler = malloc(sizeof(message_handler));
    strcpy(handler->command, command);
    handler->callback = callback;
    handler->next = NULL;
    
    if (last == NULL)
    {
        message_handlers[bucket] = handler;
    }
    else
    {
        while (last->next != NULL)
        {
            last = last->next;
        }
        last->next = handler;
    }
}

void message_dispatch_command(const char *command, message_callback_data *data)
{
    int bucket = hash(command) % MESSAGE_HANDLER_HASHTABLE_SIZE;
    message_handler *handler = message_handlers[bucket];
    
    int found = 0;
    while (handler != NULL)
    {
        if (strcmp(command, handler->command) == 0)
        {
            found = 1;
            handler->callback(data);
            // TODO: Check return value, 0 means handled, other values are some
            // error codes (not enough params, wrong handler, ...)
            // Stop calling other handlers once 0 is returned, print error
            // if there isn't any 0 returned
        }
        
        handler = handler->next;
    }
    
    if (found == 0)
    {
        debug_print_format("No handler for %s\n", command);
    }
}