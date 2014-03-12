#include "../commands.h"
#include "../channel.h"
#include "../send.h"
#include "../util.h"

int command_join(message_callback_data *e)
{
    if (e->event_data->client->registered != 1)
    {
        command_user_reply(e, "451 :You have not registered");
        return -1;
    }
    
    if (e->message_data->argc < 1)
    {
        command_user_reply(e, "461 JOIN :Not enough parameters");
        return -1;
    }
    if (e->message_data->argc > 1)
    {
        command_user_reply(e, "405 :Too many channels");
        return -1;
    }
    
    if (e->message_data->argv[0][0] != '#')
    {
        command_user_reply_format(e, "403 %s :No such channel", e->message_data->argv[0]);
        return -1;
    }
    
    channel_data *channel = channel_hashtable_find(e->message_data->argv[0]);
    if (channel == NULL)
    {
        /* Create new channel */
        debug_print_format("Creating new channel: %s", e->message_data->argv[0]);
        channel = channel_create_new(e->message_data->argv[0]);
    }
    
    /* Avoid duplicates */
    channel_client *c_client = channel->clients;
    while(c_client != NULL)
    {
        if (c_client->client == e->event_data->client)
        {
            /* Already in this channel, ignore */
            return -1;
        }
        
        c_client = c_client->next;
    }
    
    // Add client to the channel
    channel_client_join(channel, e->event_data->client);
    
    // Broadcast the join event to all channel members (including the joining one)
    send_message_buffer *buffer = send_create_buffer_format(":%s JOIN %s", e->event_data->client->nickname, e->message_data->argv[0]);
    send_enqueue_channel(channel, buffer);
    
    // Send the channel topic and user list
    command_user_reply_format(e, ":server 331 %s :No topic is set", channel->name);
    c_client = channel->clients;
    while(c_client != NULL)
    {
        command_user_reply_format(e, ":server 353 %s = %s :%s", e->event_data->client->nickname, channel->name, c_client->client->nickname);
        c_client = c_client->next;
    }
    command_user_reply_format(e, ":server 366 %s %s :End of /NAMES list", e->event_data->client->nickname, channel->name);
    
    return 0;
}

int command_part(message_callback_data *e)
{
    if (e->event_data->client->registered != 1)
    {
        command_user_reply(e, "451 :You have not registered");
        return -1;
    }
    
    if (e->message_data->argc < 1)
    {
        command_user_reply(e, "461 PART :Not enough parameters");
        return -1;
    }
    
    channel_data *channel = channel_hashtable_find(e->message_data->argv[0]);
    if (channel == NULL)
    {
        command_user_reply_format(e, "403 %s :No such channel", e->message_data->argv[0]);
        return -1;
    }
    
    send_message_buffer *buffer = send_create_buffer_format(":%s PART %s", e->event_data->client->nickname, e->message_data->argv[0]);
    send_enqueue_channel(channel, buffer);
    
    /* Remove if on channel, otherwise return error */
    int res = channel_client_part(channel, e->event_data->client);
    if (res == 0)
    {
        command_user_reply_format(e, "442 %s :Not on channel", e->message_data->argv[0]);
        return -1;
    }
    
    return 0;
}

int command_who(message_callback_data *e)
{
    if (e->event_data->client->registered != 1)
    {
        command_user_reply(e, "451 :You have not registered");
        return -1;
    }
    
    if (e->message_data->argc < 1)
    {
        command_user_reply(e, "461 WHO :Not enough parameters");
        return -1;
    }
    
    channel_data *channel = channel_hashtable_find(e->message_data->argv[0]);
    if (channel == NULL)
    {
        command_user_reply_format(e, "403 %s :No such channel", e->message_data->argv[0]);
        return -1;
    }
    
    channel_client *c_client = channel->clients;
    while(c_client != NULL)
    {
        command_user_reply_format(e, ":server 352 %s %s user host server %s H :0 name", e->event_data->client->nickname, channel->name, c_client->client->nickname);
        c_client = c_client->next;
    }
    command_user_reply_format(e, ":server 315 %s %s :End of /WHO list", e->event_data->client->nickname, channel->name);
    
    return 0;
}