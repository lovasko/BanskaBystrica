#include <signal.h>
#include <stdlib.h>

#include "signal.h"
#include "event.h"
#include "util.h"

void signal_hard_handler(int signum)
{
    info_print("Forcing shutdown");
    exit(EXIT_FAILURE);
}

void signal_soft_handler(int signum)
{
    info_print("Initiating shutdown, send another SIGINT to force");
    signal(SIGINT, signal_hard_handler);
    
    event_initiate_shutdown();    
}

void signal_register_handlers()
{
    signal(SIGINT, signal_soft_handler);
}