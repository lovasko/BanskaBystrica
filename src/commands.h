#ifndef COMMANDS_H
#define	COMMANDS_H

#include "message.h"
#include "send.h"

void command_register_handlers();

// Registration
int command_nick(message_callback_data *e);
int command_user(message_callback_data *e);
// Quit
int command_quit(message_callback_data *e);

// Messaging
int command_privmsg(message_callback_data *e);

// Channels
int command_join(message_callback_data *e);
int command_part(message_callback_data *e);
int command_who(message_callback_data *e);


#define command_user_reply(e,message) send_enqueue_client((e)->event_data->client, send_create_buffer_format((message)))
#define command_user_reply_format(e,format,...) send_enqueue_client((e)->event_data->client, send_create_buffer_format((format), __VA_ARGS__))

#endif	/* COMMANDS_H */

