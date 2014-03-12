#include <string.h>

#include "../commands.h"
#include "../client.h"
#include "../channel.h"

int command_privmsg(message_callback_data* e)
{
    if (e->event_data->client->registered != 1)
    {
        command_user_reply(e, "451 :You have not registered");
        return -1;
    }
    
    if (e->message_data->argc < 1)
    {
        command_user_reply(e, "411 :No recipient");
        return -1;
    }
    if (e->message_data->argc != 2)
    {
        command_user_reply(e, "412 :No text to send");
        return -1;
    }
    
    send_message_buffer *buffer = send_create_buffer_format(":%s PRIVMSG %s :%s", e->event_data->client->nickname, e->message_data->argv[0], e->message_data->argv[1]);
    
    if (e->message_data->argv[0][0] == '#')
    {
        /* Channel message */
        channel_data *channel = channel_hashtable_find(e->message_data->argv[0]);
        if (channel == NULL)
        {
            command_user_reply_format(e, "403 %s :No such channel", e->message_data->argv[0]);
            return -1;
        }
        
        // Can't use send_enqueue_channel because we need to skip the sender
        channel_client *c_client = channel->clients;
        while(c_client != NULL)
        {
            if (c_client->client != e->event_data->client)
            {
                send_enqueue_client(c_client->client, buffer);
            }
            c_client = c_client->next;
        }
    }
    else 
    {
        /* User message */
        client_data *target = client_nickname_hashtable_find(e->message_data->argv[0]);
        if (target == NULL)
        {
            command_user_reply(e, "401 :No such nick");
            return -1;
        }
        
        send_enqueue_client(target, buffer);
    }
    
    return 0;
}