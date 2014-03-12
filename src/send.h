#ifndef SEND_H
#define	SEND_H

#include <unistd.h>
#include <stdarg.h>

typedef struct send_message_buffer
{
    char *contents;
    size_t contents_length;
    
    // Number of send queues this buffer is in (reference counting for GC)
    int gc_count;
} send_message_buffer;

typedef struct send_queue_element
{
    send_message_buffer *buffer;
    // Position to the first byte that has yet to be send to this client.
    // Since we might not write everything in a single EPOLLOUT event (data_out)
    // we need to continue where we ended, might be different for each client.
    size_t buffer_position;
    struct send_queue_element *next;
} send_queue_element;

void send_delete_queue(send_queue_element *queue_element);
void send_release_buffer(send_message_buffer *buffer);

struct client_data;
struct channel_data;
struct event_callback_data;

void send_enqueue_client(struct client_data *client, send_message_buffer *buffer);
void send_enqueue_channel(struct channel_data *channel, send_message_buffer *buffer);

send_message_buffer *send_create_buffer(char *message);
send_message_buffer *send_create_buffer_copy(char *message);
send_message_buffer *send_create_buffer_format(const char *format, ...);

// Helper function, creates a copy of message,
// caller must call free or use compile-time string
void send_message_client(struct client_data *client, const char *message);

int send_callback_data_out(struct event_callback_data *e);
#endif	/* SEND_H */