#include "common.h"

#ifdef USE_SELECT
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/select.h>

#include "socket.h"
#include "event.h"
#include "util.h"
#include "client.h"

void event_start_loop_select(int serverfd)
{
    fd_set readfds;
    fd_set writefds;
    fd_set exceptfds;
    struct timeval timeout;
    int maxfd;
    
    // Lookup table: fd -> client_data*
    client_data *clients [FD_SETSIZE];
    clients[serverfd] = NULL;
    
    while (event_shutting_down == 0)
    {
        FD_ZERO(&readfds);
        FD_ZERO(&writefds);
        FD_ZERO(&exceptfds);
        
        // Add server socket
        maxfd = serverfd;
        FD_SET(serverfd, &readfds);
        FD_SET(serverfd, &exceptfds);
        
        // Add client sockets
        client_data *client = client_get_first();
        while (client != NULL)
        {
            FD_SET(client->fd, &readfds);
            FD_SET(client->fd, &exceptfds);
            
            // Only add to write set when there is something waiting in
            // the send queue
            if (client->send_queue_start != NULL)
            {
                FD_SET(client->fd, &writefds);
            }
            
            // Put to lookup table, might already be there
            // but we have to iterate over all of them anyway
            clients[client->fd] = client;
            
            // Find max fd for select
            if (client->fd > maxfd)
            {
                maxfd = client->fd;
            }
            
            client = client->next;
        }
        
        timeout.tv_sec = TIMEOUT / 1000;
        timeout.tv_usec = TIMEOUT % 1000;
        
        int n = select(maxfd+1, &readfds, &writefds, &exceptfds, &timeout);
        if (n < 0)
        {
            /* Interrupted - probably got SIGINT */
            if (errno == EINTR)
            {
                continue;
            }
            
            error_print_exit("select");
        }
        
        if (n == 0)
        {
            /* No events, timed-out */
            event_dispatch_event(event_flags_timeout, NULL);            
        }
        else
        {            
            // TODO: Maybe iterate over the client list, in case the
            // fds are sparse
            for(int i = 0; i <= maxfd && n > 0; i++)
            {
                int event_count = FD_ISSET(i, &readfds) + FD_ISSET(i, &writefds) + FD_ISSET(i, &exceptfds);
                if (event_count == 0)
                {
                    continue;
                }
                
                // 1-3 less events to search for
                n -= event_count;
                
                client_data *client_event_data = clients[i];
                
                event_callback_data callback_data;
                callback_data.client = client_event_data;
                callback_data.client_new = NULL;
                callback_data.buffer = NULL;
                callback_data.buffer_length = 0;
                
                if (FD_ISSET(i, &exceptfds))
                {
                    if (i == serverfd)
                    {
                        error_print_exit("Error on server socket");
                    }
                    
                    event_disconnect_client(client_event_data, &callback_data);
                    continue;
                }
                
                if (FD_ISSET(i, &readfds))
                {
                    /* Client waiting to connect */
                    if (i == serverfd)
                    {
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
                            client_new->fd = clientfd;
                            
                            debug_print_format("Accepted new connection, fd: %d", clientfd);
                        
                            callback_data.client_new = client_new;
                            event_dispatch_event(event_flags_connect, &callback_data);
                        }
                    }
                    else
                    {
                        /* Data waiting */
                        event_read_available(client_event_data, &callback_data);
                    }
                }
                
                if (FD_ISSET(i, &writefds))
                {
                    if (event_dispatch_event(event_flags_data_out, &callback_data) < 0)
                    {
                        event_disconnect_client(client_event_data, &callback_data);
                    }
                }
                
                if (client_event_data != NULL && client_event_data->quitting)
                {
                    client_delete(client_event_data);
                }
                
            }
        }
    }
    
    close(serverfd);
}
#endif