#ifndef COMMON_H
#define	COMMON_H

/** Used as function success return value. */
#define OK 0

/** Used as function failure return value. */
#define FAIL 1

#ifndef MESSAGE_HANDLER_HASHTABLE_SIZE
    #define MESSAGE_HANDLER_HASHTABLE_SIZE  64
#endif

#ifndef CLIENT_NICKNAME_HASHTABLE_SIZE
    #define CLIENT_NICKNAME_HASHTABLE_SIZE  256
#endif

#ifndef CHANNEL_HASHTABLE_SIZE
    #define CHANNEL_HASHTABLE_SIZE  128
#endif

/* epoll or select */
#ifdef USE_EPOLL
    #ifdef USE_SELECT
        #error "Can't use both epoll and select"
    #endif
#endif

/* Use select by default */
#ifndef USE_EPOLL
    #ifndef USE_SELECT
        #define USE_SELECT
    #endif
#endif

#endif	/* COMMON_H */

