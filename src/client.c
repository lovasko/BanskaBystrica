#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "client.h"
#include "event.h"
#include "message.h"
#include "util.h"
#include "send.h"
#include "common.h"
#include "channel.h"

void client_nickname_hashtable_add(client_data *client);
void client_nickname_hashtable_remove(client_data *client);

static client_data *clients = NULL;
static client_data *client_nickname[CLIENT_NICKNAME_HASHTABLE_SIZE];

client_data *client_allocate_new()
{
    client_data *client = malloc(sizeof(client_data));
    client->fd = 0;
    client->server = 0;
    
    client->line_buffer_pos = 0;
    
    client->send_queue_start = NULL;
    client->send_queue_end = NULL;
    
    client->prev = NULL;
    client->next = clients;
    
    client->nickname = NULL;
    client->nickname_next = NULL;
    client->nickname_prev = NULL;
    
    client->channels = NULL;
    
    client->registered = 0;
    client->username = NULL;
    client->quitting = 0;
    
    // Prepend to the client list
    if (clients != NULL)
    {
        clients->prev = client;
    }
    clients = client;
    
    return client;
}

void client_delete(client_data* client)
{
    // Remove from the linked list of clients
    if (client->prev != NULL)
    {
        client->prev->next = client->next;
    }
    else
    {
        clients = client->next;
    }
    
    if (client->next != NULL)
    {
        client->next->prev = client->prev;
    }
    
    // Clean up all data stored in this client's client_data
    if (client->send_queue_start != NULL)
    {
        send_delete_queue(client->send_queue_start);
    }
    
    // Remove client from channels
    client_channel *channel = client->channels;
    while (channel != NULL)
    {
        client_channel *tmp = channel->next;
        
        channel_client_part(channel->channel, client);
        
        channel = tmp;
    }   
    
    // Remove from nickname hashtable
    if (client->nickname != NULL)
    {
        client_nickname_hashtable_remove(client);
    }
    
    if (client->nickname != NULL)
    {
        free(client->nickname);
    }
    if (client->username != NULL)
    {
        free(client->username);
    }
    
    free(client);
}

client_data *client_get_first()
{
    return clients;
}

void client_channel_join(client_data *client, channel_data *channel)
{
    client_channel *c_channel = malloc(sizeof(client_channel));
    
    c_channel->channel = channel;
    c_channel->next = client->channels;
    
    client->channels = c_channel;
}

int client_channel_part(client_data *client, channel_data *channel)
{
    client_channel *c_channel = client->channels;
    client_channel *prev = NULL;
    while(c_channel != NULL)
    {
        if (c_channel->channel == channel)
        {
            if(prev == NULL)
            {
                client->channels = c_channel->next;
            }
            else
            {
                prev->next = c_channel->next;
            }
            
            free (c_channel);
            return 1;
        }
        prev = c_channel;
        c_channel = c_channel->next;
    }
    
    return 0;
}

int client_callback_data_in(event_callback_data *e)
{
    const char *start = e->buffer;
    const char *pos = start+1;
    message_callback_data callback_data;
    for(int i = 1; i < e->buffer_length; i++)
    {
        if (*(pos-1) == '\r' && *pos == '\n')
        {
            // Found line end, append everything from start to pos-1 to the
            // line buffer and process it
            int length = pos - start - 1;
            if (e->client->line_buffer_pos + length >= sizeof(e->client->line_buffer))
            {
                length = sizeof(e->client->line_buffer) - e->client->line_buffer_pos - 1;
            }
            memcpy(e->client->line_buffer + e->client->line_buffer_pos, start, length);
            e->client->line_buffer_pos += length;
            e->client->line_buffer[e->client->line_buffer_pos] = '\0';
            
            // Parse the line into a message structure
            message_data *message = message_parse(e->client->line_buffer);
            
            // Process the message
            debug_print_format("%d :: %s", e->client->fd, e->client->line_buffer);
            if (message != NULL)
            {
                debug_print_format("Command [%s], argc=%d", message->command, message->argc);
                
                callback_data.event_data = e;
                callback_data.message_data = message;
                message_dispatch_command(message->command, &callback_data);
            }
            
            // Clean up
            message_delete(message);
            
            e->client->line_buffer_pos = 0;
            start = pos+1;
        }
        pos++;
    }
    // Reached the end, copy any remaining unterminated characters to the buffer
    if (start != pos)    
    {
        memcpy(e->client->line_buffer, start, pos - start);
        e->client->line_buffer_pos = pos - start;
    }
    
    return 0;
}

int client_callback_disconnect(event_callback_data *e)
{
    if (e == NULL)
    {
        /* Server shutdown, ignore ... */
        return 0;
    }
    if (e->client->registered != 1)
    {
        /* Not even registered, ignore */
        return 0;
    }
    
    /* Inform clients on channels */
    client_channel *c_channel = e->client->channels;
    send_message_buffer *buffer = NULL;
    while(c_channel != NULL)
    {
        if (buffer == NULL)
        {
            buffer = send_create_buffer_format(":%s QUIT", e->client->nickname);
        }
        send_enqueue_channel(c_channel->channel, buffer);
        c_channel = c_channel->next;
    }
    
    return 0;    
}

// TODO: Lowercase (RFC lowercase!) hashing and comparison

void client_nickname_hashtable_add(client_data *client)
{
    int bucket = hash(client->nickname) % CLIENT_NICKNAME_HASHTABLE_SIZE;
    
    if (client_nickname[bucket] == NULL)
    {
        client_nickname[bucket] = client;
    }
    else
    {
        client->nickname_next = client_nickname[bucket];
        client_nickname[bucket] = client;
    }
}

void client_nickname_hashtable_remove(client_data *client)
{
    if (client->nickname_prev == NULL)
    {
        client_nickname[hash(client->nickname) % CLIENT_NICKNAME_HASHTABLE_SIZE] = client->nickname_next;
    }
    else
    {
        client->nickname_prev->nickname_next = client->nickname_next;
    }
}

client_data *client_nickname_hashtable_find(char *nickname)
{
    client_data *client = client_nickname[hash(nickname) % CLIENT_NICKNAME_HASHTABLE_SIZE];
    while (client != NULL)
    {
        if (strcmp(client->nickname, nickname) == 0)
        {
            return client;
        }
        client = client->nickname_next;
    }
    
    return NULL;
}

void client_set_nickname(client_data *client, const char *nickname)
{
    if (client->nickname != NULL)
    {
        client_nickname_hashtable_remove(client);
        free(client->nickname);
    }
    
    client->nickname = malloc(sizeof(char)*(strlen(nickname)+1));
    strcpy(client->nickname, nickname);
    
    client_nickname_hashtable_add(client);    
}

void client_set_username(client_data *client, const char *username)
{
    if (client->username != NULL)
    {
        free(client->username);
    }
    client->username = malloc(sizeof(char)*(strlen(username)+1));
    strcpy(client->username, username);
}