#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include "signal.h"
#include "socket.h"
#include "util.h"
#include "event.h"
#include "client.h"
#include "message.h"
#include "common.h"
#include "commands.h"
#include "options.h"

int main(int argc, char** argv)
{
	struct options opt;
	int serverfd;

	if (options_parse(argc, argv, &opt) != OK)
	{
		return EXIT_FAILURE;
	}

	serverfd = socket_create_and_bind(addr.s_addr, port);
	if (serverfd < 0)
	{
		error_print_exit("socket_create_and_bind");
	}

	if (socket_set_nonblocking(serverfd) < 0)
	{
		error_print_exit("socket_set_nonblocking");
	}

	if (socket_listen(serverfd, -1) < 0)
	{
		error_print_exit("socket_listen");
	}

	event_register_handlers();
	command_register_handlers();
	signal_register_handlers();

	event_start_loop(serverfd);
	return EXIT_SUCCESS;
}

