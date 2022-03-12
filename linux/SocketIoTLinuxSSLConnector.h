#ifndef SocketIoTLinuxSSLConnector_h
#define SocketIoTLinuxSSLConnector_h

#include <iostream>
#include <openssl/ssl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include "SocketIoTDebug.h"


class SocketIoTLinuxConnnector
{
    SSL_CTX *ctx;
    SSL *ssl;
    int sockfd;
    const char *host;
    uint16_t port;

public:
    SocketIoTLinuxConnnector() : ctx(NULL), host(NULL), port(0), ssl(NULL), sockfd(-1)
    {
        SSL_library_init();
        this->ctx = InitCTX();
    }

    SSL_CTX *InitCTX(void)
    {
        const SSL_METHOD *method;
        SSL_CTX *ctx;

        OpenSSL_add_all_algorithms();
        SSL_load_error_strings();

        method = SSLv23_client_method();
        ctx = SSL_CTX_new(method);
    
        return ctx;
    }

    void begin(const char *host, uint16_t port)
    {
        this->host = host;
        this->port = port;
    }

    bool connect()
    {
        struct addrinfo hints;
        struct addrinfo *res = NULL;

        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;

        char strport[8];
        snprintf(strport, sizeof(strport), "%u", port);
        getaddrinfo(host, strport, &hints, &res);

        if (res == NULL)
        {
            LOG1(SF("Cannot Get AddrInfo"));
            return false;
        }

        if ((sockfd = ::socket(res->ai_family, res->ai_socktype, res->ai_protocol)) < 0)
        {
            LOG1(SF("Cannot Create Socket"));
            return false;
        }

        if (::connect(sockfd, res->ai_addr, res->ai_addrlen) < 0)
        {
            LOG1(SF("Connection Failed"));
            return false;
        }
        
        freeaddrinfo(res);

        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof(struct timeval));

        int one = 1;
        setsockopt(sockfd, SOL_TCP, TCP_NODELAY, &one, sizeof(one));

        ssl = SSL_new(ctx);

        if (ssl == NULL)
        {
            LOG1(SF("Cannot Create SSK"));
            return false;
        }

        SSL_set_fd(ssl, sockfd);

        if (SSL_connect(ssl) < 0)
        {
            LOG1(SF("SSL Connection Failed"));
            sockfd = -1;
            return false;
        }

        return true;
    }

    size_t read(void *buff, size_t len)
    {
        return SSL_read(ssl, buff, len);
    }

    size_t write(const void *buff, size_t len)
    {
        return SSL_write(ssl, buff, len);
    }

    bool connected(){
        return sockfd >= 0;
    }

    void disconnect(){
       if(sockfd == -1){
           while(::close(sockfd) < 0){
               usleep(1000);
           }
           sockfd = -1;
           SSL_shutdown(ssl);
           SSL_free(ssl);
       }
    }

    int available() {
        SSL_read(ssl, NULL, 0);
        return SSL_pending(ssl);
    }

    ~SocketIoTLinuxConnnector(){
        close(sockfd);
        SSL_shutdown(ssl);
        SSL_free(ssl);
        SSL_CTX_free(ctx);
    }
};

#endif