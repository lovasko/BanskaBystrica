#ifndef OPTIONS_H
#define OPTIONS_H

/**
 * Command-line options package encapsulation.
 */
struct options
{
	unsigned short port; /**< listening port */
	struct in_addr addr; /**< listening address */
	char *modules_file;  /**< location of the modules configuration file*/
};

/**
 * Parse the command-line options with getopt(3).
 * @param[in] argc argument count
 * @param[in] argv argument string vector
 * @param[out] opt options to be written to
 * @return OK/FAIL  
 */
int options_parse(int argc, char **argv, struct options *opt);

#endif

