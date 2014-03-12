#ifndef CLIENT_H
#define	CLIENT_H

#include "send.h"
#include "rfc.h"

struct channel_data;

typedef struct client_channel
{
    struct channel_data *channel;
    struct client_channel *next;
} client_channel;

typedef struct client_data
{
    int fd;
    int server;
    
    char line_buffer[RFC_MESSAGE_MAXLENGTH];
    int line_buffer_pos;
    
    // Write queue
    send_queue_element *send_queue_start;
    send_queue_element *send_queue_end; // For fast appending to the queue    
    
    // Linked-list elements
    struct client_data *prev;
    struct client_data *next;
    
    // Nickname hashtable
    char *nickname;
    struct client_data *nickname_next;
    struct client_data *nickname_prev;
    
    // Channels
    client_channel *channels;
    
    // User data
    int registered;
    char *username;
    int quitting;
    
} client_data;

client_data *client_allocate_new();
void client_delete(client_data *client_data);

client_data *client_get_first();

void client_channel_join(client_data *client, struct channel_data *channel);
int client_channel_part(client_data *client, struct channel_data *channel);

struct event_callback_data;
int client_callback_data_in(struct event_callback_data *e);
int client_callback_disconnect(struct event_callback_data *e);

client_data *client_nickname_hashtable_find(char *nickname);

void client_set_nickname(client_data *client, const char *nickname);
void client_set_username(client_data *client, const char *username);

#endif	/* CLIENT_H */