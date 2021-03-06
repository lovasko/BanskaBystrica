#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>

#include "options.h"
#include "defaults.h"
#include "common.h"

int options_parse(int argc, char **argv, struct options *opt)
{
	int option;

	opt->port = DEFAULT_PORT;
	opt->addr.s_addr = DEFAULT_ADDRESS;
	opt->modules_file = DEFAULT_MODULES_FILE;

	while ((option = getopt(argc, argv, "va:p:m:")) != -1)
	{
		switch(option)
		{
		case 'v': /* Verbose */
			debug_set_verbose(1);
		break;

		case 'a': /* Address */
			if (inet_aton(optarg, &opt->addr) == 0)
			{
				fprintf(stderr, "%s: invalid address -- '%s'\n", argv[0], optarg);
				return FAIL;
			}
		break;

		case 'p': /* Port */
			errno = 0;
			long int input = strtol(optarg, NULL, 10);

			if (errno != 0)
			{
				if (errno == EINVAL)
				{
					fprintf(stderr, "%s: unable to convert -- '%s'\n", argv[0], optarg);
				}
				else if (errno == ERANGE)
				{
					fprintf(stderr, "%s: out of range -- '%s'\n", argv[0], optarg);
				}
				else
				{
					fprintf(stderr, "%s: unknown problem\n", argv[0]);
				}
				
				return FAIL;
			}

			if (input < 0)
			{
				fprintf(stderr, "%s: port number negative -- '%s'\n", argv[0], optarg);
				return FAIL;
			}

			if (input > 65535)
			{
				fprintf(stderr, "%s: port number too high -- '%s'\n", argv[0], optarg);
				return FAIL;
			}

			opt->port = (unsigned short)input;
		break;

		case 'm':
			opt->modules_file = strdup(optarg);
		break;

		case '?':
			fprintf(stderr, "%s: invalid option -- '%c'\n", argv[0], optopt);
			fprintf(stderr, "Usage: %s [-v] [-a addr] [-p port]\n", argv[0]);
		return FAIL;

		default:
			return FAIL;
		}
	}

	return OK;
}

