#include "commands.h"
#include "message.h"

void command_register_handlers()
{
    message_register_handler("nick", command_nick);
    message_register_handler("user", command_user);
    
    message_register_handler("quit", command_quit);
    
    message_register_handler("privmsg", command_privmsg);
    
    message_register_handler("join", command_join);
    message_register_handler("part", command_part);
    message_register_handler("who", command_who);
}
