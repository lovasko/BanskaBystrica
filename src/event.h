#ifndef EVENT_H
#define	EVENT_H

#include <stddef.h>

#include "socket.h"
#include "client.h"

#define MAXEVENTS 32
#define TIMEOUT 5000

typedef enum event_flags
{
    event_flags_timeout         = 1,
    event_flags_connect         = 2,
    event_flags_disconnect      = 4,
    event_flags_data_in         = 8,
    event_flags_data_out        = 16,
} event_flags;

typedef struct event_callback_data
{
    event_flags flags;
    client_data *client;
    client_data *client_new;
    const char *buffer;
    size_t buffer_length;
} event_callback_data;

typedef int (*event_callback_func)(event_callback_data *callback_data);

typedef struct event_handler 
{
    event_callback_func callback;
    event_flags flags;
    struct event_handler *next;
} event_handler;

extern event_handler *event_handlers;

void event_register_handler(event_flags flags, event_callback_func callback);

int event_dispatch_event(event_flags flags, event_callback_data *callback_data);
void event_register_handlers();

extern int event_shutting_down;

void event_start_loop(int serverfd);
void event_initiate_shutdown();

int event_read_available(client_data *client_event_data, event_callback_data *callback_data);
void event_disconnect_client(client_data *client_event_data, event_callback_data *callback_data);
#endif	/* EVENT_H */