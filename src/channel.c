#include <string.h>
#include <stdlib.h>

#include "channel.h"
#include "client.h"
#include "common.h"
#include "util.h"

static channel_data *channels = NULL;
static channel_data *channel_name[CHANNEL_HASHTABLE_SIZE];

channel_data *channel_create_new(const char *name)
{
    channel_data *channel = malloc(sizeof(channel_data));
    channel->clients = NULL;
    channel->name_prev = NULL;
    channel->name_next = NULL;
    
    channel->name = malloc(sizeof(char)*(strlen(name)+1));
    strcpy(channel->name, name);
    
    channel_hashtable_add(channel);
    
    channel->prev = NULL;
    channel->next = channels;
    
    if (channels != NULL)
    {
        channels->prev = channel;
    }
    channels = channel;
    
    return channel;    
}

void channel_delete(channel_data *channel)
{
    if (channel->prev != NULL)
    {
        channel->prev->next = channel->next;
    }
    else
    {
        channels = channel->next;
    }
    
    if(channel->next != NULL)
    {
        channel->next->prev = channel->prev;
    }
    
    channel_hashtable_remove(channel);
    free(channel->name);
    
    // Remove all clients
    while(channel->clients != NULL)
    {
        channel_client_part(channel, channel->clients->client);
    }
    
    free(channel);    
}

channel_data *channel_get_first()
{
    return channels;
}

void channel_client_join(channel_data *channel, client_data *client)
{
    channel_client *c_client = malloc(sizeof(channel_client));
    
    c_client->client = client;
    c_client->next = channel->clients;
    
    channel->clients = c_client;
    
    client_channel_join(client, channel);
}

int channel_client_part(channel_data *channel, client_data *client)
{
    channel_client *c_client = channel->clients;
    channel_client *prev = NULL;
    int found = 0;
    while(c_client != NULL)
    {
        if (c_client->client == client)
        {
            if(prev == NULL)
            {
                channel->clients = c_client->next;
            }
            else
            {
                prev->next = c_client->next;
            }
            found = 1;
            free (c_client);
            break;
        }
        prev = c_client;
        c_client = c_client->next;
    }
    
    if (found == 0)
    {
        return 0;
    }
    client_channel_part(client, channel);
    
    /* Last client parted */
    if (channel->clients == NULL)
    {
        debug_print_format("Destroying channel: %s", channel->name);
        channel_delete(channel);
        /* Return -1 to show that the channel was deleted */
        return -1;
    }
    
    return 1;
}

void channel_hashtable_add(channel_data *channel)
{
    int bucket = hash(channel->name) % CHANNEL_HASHTABLE_SIZE;
    if (channel_name[bucket] == NULL)
    {
        channel_name[bucket] = channel;
    }
    else
    {
        channel->name_next = channel_name[bucket];
        channel_name[bucket] = channel;
    }
    
}

void channel_hashtable_remove(channel_data *channel)
{
    if (channel->name_prev == NULL)
    {
        channel_name[hash(channel->name) % CHANNEL_HASHTABLE_SIZE] = channel->name_next;
    }
    else
    {
        channel->name_prev->name_next = channel->name_next;
    }
}

channel_data *channel_hashtable_find(const char *name)
{
    channel_data *channel = channel_name[hash(name) % CHANNEL_HASHTABLE_SIZE];
    while (channel != NULL)
    {
        if (strcmp(channel->name, name) == 0)
        {
            return channel;
        }
        channel = channel->name_next;
    }
    
    return NULL;
}
