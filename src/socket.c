#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <asm-generic/socket.h>

#include "common.h"
#ifdef USE_EPOLL
    #include <sys/epoll.h>
#else
    #include <sys/select.h>
#endif

#include "socket.h"
#include "client.h"
#include "util.h"

int socket_create_and_bind(in_addr_t addr, unsigned short port)
{
    // Get an INET+STREAM socket (TCP/IP)
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd < 0)
    {
        return error_print("socket");
    }
    
    int reuse_addr = 1;
    setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr));
    
    struct sockaddr_in socket_addr;
    memset(&socket_addr, 0, sizeof(socket_addr));
    socket_addr.sin_family = AF_INET;
    socket_addr.sin_addr.s_addr = addr;
    socket_addr.sin_port = htons(port);
    
    if (bind(socketfd, (struct sockaddr *) &socket_addr, sizeof(socket_addr)) < 0)
    {
        return error_print("bind");
    }
    
    return socketfd;    
}

int socket_set_nonblocking(int socketfd)
{
    // Get old flags
    int flags = fcntl(socketfd, F_GETFL);
    if (flags < 0)
    {
        return error_print("fcntl(F_GETFL)");
    }
    // Add nonblocking flag
    flags |= O_NONBLOCK;
    if (fcntl(socketfd, F_SETFL, flags) < 0)
    {
        return error_print("fcntl(F_SETFL)");
    }
    
    return 0;    
}

int socket_listen(int socketfd, int backlog)
{
    // Default size for backlog is the maximum possible
    if (backlog < 0)
    {
        backlog = SOMAXCONN;
    }
    
    if (listen(socketfd, backlog) < 0)
    {
        return error_print("listen");
    }
    
    return 0;
}

#ifdef USE_EPOLL
int socket_epoll_ctl(int socketfd, int epollfd, client_data *client)
{
    client->fd = socketfd;
    
    struct epoll_event event;
    memset(&event, 0, sizeof(event));
    event.data.ptr = client;
    event.events = EPOLLIN | EPOLLET;
    
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, socketfd, &event) < 0)
    {
        return error_print("epoll_ctl");
    }
    
    return 0;
}

int socket_epoll_create_and_setup(int socketfd)
{
    int epollfd = epoll_create1(0);
    if (epollfd < 0)
    {
        return error_print("epoll_create1");
    }
    
    client_data *client = client_allocate_new();
    client->server = 1;
    if (socket_epoll_ctl(socketfd, epollfd, client) < 0)
    {
        return error_print("socket_epoll_ctl");
    }
    
    return epollfd;
}
#endif

int socket_accept_client(int serverfd){
    struct sockaddr client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    int clientfd = accept(serverfd, &client_addr, &client_addr_len);
    if (clientfd < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            /* All waiting incoming connections have been processed */
            return 0;
        }
        error_print("accept");
        return -1;
    }

    if (socket_set_nonblocking(clientfd) < 0)
    {
        error_print_exit("socket_set_nonblocking");
    }

    return clientfd;
}