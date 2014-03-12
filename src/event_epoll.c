#include "common.h"

#ifdef USE_EPOLL
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>

#include "socket.h"
#include "event.h"
#include "util.h"
#include "client.h"

void event_start_loop_epoll(int serverfd)
{
    info_print("Creating and setting up epoll descriptor");
    int epollfd = socket_epoll_create_and_setup(serverfd);
    if (epollfd < 0)
    {
        error_print_exit("socket_epoll_create_and_setup");
    }
    
    struct epoll_event *events = calloc(MAXEVENTS, sizeof(struct epoll_event));    

    while (event_shutting_down == 0)
    {
        int n = epoll_wait(epollfd, events, MAXEVENTS, TIMEOUT);
        if (n < 0)
        {
            /* Interrupted - probably got SIGINT */
            if (errno == EINTR)
            {
                continue;
            }
            
            error_print_exit("epoll_wait");
        }
        
        if (n == 0)
        {
            /* No events, timed-out */
            event_dispatch_event(event_flags_timeout, NULL);
        }
        else
        {
            for(int i = 0; i < n; i++)
            {
                client_data *client_event_data = (client_data *) events[i].data.ptr;
                
                event_callback_data callback_data;
                callback_data.client = client_event_data;
                callback_data.client_new = NULL;
                callback_data.buffer = NULL;
                callback_data.buffer_length = 0;
                
                if (events[i].events & EPOLLERR || events[i].events & EPOLLHUP || !(events[i].events & (EPOLLIN | EPOLLOUT)))
                {
                    /* An error occurred on this socket (or disconnected) */
                    info_print_format("Socket error at event %d, fd: %d", i, client_event_data->fd);
                    close(client_event_data->fd);
                }
                else if (client_event_data->fd == serverfd)
                {
                    /* Event at the server descriptor - new incoming connection(s) */
                    while (1)
                    {
                        int clientfd = socket_accept_client(serverfd);
                        
                        if (clientfd < 0)
                        {
                            error_print("socket_accept_client");
                            break;
                        }
                        else if (clientfd == 0)
                        {
                            /* All waiting incoming connections have been processed */
                            break;
                        }
                        
                        client_data *client_new = client_allocate_new();
                        if (socket_epoll_ctl(clientfd, epollfd, client_new) < 0)
                        {
                            error_print_exit("socket_epoll_ctl");
                        }
                        
                        debug_print_format("Accepted new connection, fd: %d", clientfd);
                        
                        callback_data.client_new = client_new;
                        event_dispatch_event(event_flags_connect, &callback_data);
                    }
                }
                else
                {
                    if (events[i].events & EPOLLIN)
                    {
                        event_read_available(client_event_data, &callback_data);
                    }
                }
                
                if (client_event_data->quitting)
                {
                    client_delete(client_event_data);
                }
            }
        }
        
        // Write send queues, if any
        client_data *client_event_data = client_get_first();
        while (client_event_data != NULL)
        {
            if (!client_event_data->quitting && client_event_data->send_queue_start != NULL)
            {
                event_callback_data callback_data;
                callback_data.client = client_event_data;
                callback_data.client_new = NULL;
                callback_data.buffer = NULL;
                callback_data.buffer_length = 0;
                
                // Disconnect on error
                if (event_dispatch_event(event_flags_data_out, &callback_data) < 0)
                {
                    event_disconnect_client(client_event_data, &callback_data);
                }
            }
            
            client_event_data = client_event_data->next;
        }
    }
    
    close(epollfd);
    close(serverfd);
}
#endif
