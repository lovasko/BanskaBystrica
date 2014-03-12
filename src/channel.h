#ifndef CHANNEL_H
#define	CHANNEL_H

#include "client.h"

typedef struct channel_client
{
    client_data *client;
    struct channel_client *next;
} channel_client;

typedef struct channel_data
{
    char *name;
    
    // Linked list of channels
    struct channel_data *prev;
    struct channel_data *next;
    
    // List of users
    channel_client *clients;
    
    // Channel name hashtable
    struct channel_data *name_prev;
    struct channel_data *name_next;
    
} channel_data;

channel_data *channel_create_new(const char *name);
void channel_delete(channel_data *channel);

channel_data *channel_get_first();

void channel_client_join(channel_data *channel, client_data *client);
int channel_client_part(channel_data *channel, client_data *client);

void channel_hashtable_add(channel_data *channel);
void channel_hashtable_remove(channel_data *channel);
channel_data *channel_hashtable_find(const char *name);

#endif	/* CHANNEL_H */
