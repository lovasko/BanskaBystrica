#ifndef DEFAULTS_H
#define DEFAULTS_H

#include <netinet/in.h>

#ifndef DEFAULT_PORT 
	#define DEFAULT_PORT 6667
#else
	#warning "Default port already defined!"
#endif

#ifndef DEFAULT_ADDRESS
	#define DEFAULT_ADDRESS INADDR_ANY 
#else
	#warning "Default address already defined!"
#endif

#endif
