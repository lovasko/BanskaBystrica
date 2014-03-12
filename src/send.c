#include <sys/socket.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "send.h"
#include "client.h"
#include "channel.h"
#include "event.h"
#include "util.h"
#include "rfc.h"

void send_delete_queue(send_queue_element *queue_element)
{
    while (queue_element != NULL)
    {
        send_queue_element *current = queue_element;
        
        send_release_buffer(current->buffer);
        
        queue_element = current->next;
        free(current);
    }
}

void send_release_buffer(send_message_buffer *buffer)
{
    if (buffer != NULL)
    {
        buffer->gc_count--;
        // Delete if this was the last reference to this buffer
        if (buffer->gc_count < 1)
        {
            free(buffer->contents);
            free(buffer);
        }
    }
}

void send_enqueue_client(client_data *client, send_message_buffer *buffer)
{
    buffer->gc_count++;
    
    send_queue_element *queue_element = malloc(sizeof(send_queue_element));
    queue_element->buffer = buffer;
    queue_element->buffer_position = 0;
    queue_element->next = NULL;
    
    if (client->send_queue_end == NULL)
    {
        client->send_queue_start = queue_element;
        client->send_queue_end = queue_element;
    }
    else
    {
        client->send_queue_end->next = queue_element;
        client->send_queue_end = queue_element;
    }
}

void send_enqueue_channel(struct channel_data *channel, send_message_buffer *buffer)
{
    channel_client *client = channel->clients;
    if (client == NULL)
    {
        /* No users, avoid leaking memory */
        send_release_buffer(buffer);
        return;
    }
    
    while(client != NULL)
    {
        send_enqueue_client(client->client, buffer);
        client = client->next;
    }
}

send_message_buffer *send_create_buffer(char* message)
{
    send_message_buffer *buffer = malloc(sizeof(send_message_buffer));
    buffer->contents_length = strlen(message);
    buffer->contents = message;
    buffer->gc_count = 0;
    
    return buffer;
}

send_message_buffer *send_create_buffer_copy(char* message)
{
    send_message_buffer *buffer = malloc(sizeof(send_message_buffer));
    buffer->contents_length = strlen(message);
    buffer->contents = malloc(sizeof(char)*buffer->contents_length);
    memcpy(buffer->contents, message, buffer->contents_length);
    buffer->gc_count = 0;
    
    return buffer;
}

send_message_buffer *send_create_buffer_format(const char* format, ...)
{
    send_message_buffer *buffer = malloc(sizeof(send_message_buffer));
    buffer->gc_count = 0;
    
    buffer->contents = malloc(sizeof(char)*RFC_MESSAGE_MAXLENGTH);
    
    va_list args;
    va_start(args, format);
    
    int length = vsprintf(buffer->contents, format, args);
    
    va_end(args);
    
    if (buffer->contents[length-2] != '\r' || buffer->contents[length-1] != '\n')
    {
        buffer->contents[length] = '\r';
        buffer->contents[length+1] = '\n';
        length += 2;
    }
    
    buffer->contents_length = length;    
    return buffer;
}

void send_message_client(client_data *client, const char *message)
{
    send_message_buffer *buffer = malloc(sizeof(send_message_buffer));
    buffer->contents_length = strlen(message);
    buffer->contents = malloc(sizeof(char)*buffer->contents_length);
    memcpy(buffer->contents, message, buffer->contents_length);
    buffer->gc_count = 0;
    
    send_enqueue_client(client, buffer);    
}

int send_callback_data_out(event_callback_data *e)
{
    while (e->client->send_queue_start != NULL)
    {
        send_queue_element *current = e->client->send_queue_start;
        
        // Send the remaining portion of the current buffer
        size_t remaining = current->buffer->contents_length - current->buffer_position;
        ssize_t n = send(e->client->fd, current->buffer->contents+current->buffer_position, remaining, 0);
        if (n < 0)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                error_print("send");
                return -1;
            }
            // Can't send any more data, just return
            return 0;
        }
        
        if (n < remaining)
        {
            // Didn't send everything, move the buffer position and try again
            current->buffer_position += n;
            continue;
        }
        
        // Everything was sent, release the current buffer and
        // move to the next one
        send_release_buffer(current->buffer);
        e->client->send_queue_start = current->next;
        free(current);
        
        if (e->client->send_queue_start == NULL)
        {
            // We have processed and deleted the last element in the queue, 
            // but the _end pointer will still be pointing to it
            e->client->send_queue_end = NULL;
        }
    }
    
    return 0;
}
