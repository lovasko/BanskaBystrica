#include <unistd.h>
#include <stdlib.h>

#include "../commands.h"
#include "../util.h"

int command_quit(message_callback_data *e)
{
    event_disconnect_client(e->event_data->client, e->event_data);
    
    return 0;
}
