#include "rotctld_communication.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <cstring>
#include <stdlib.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>

int rotctld_connect(const char *rotctld_host, const char *rotctld_port)
{
    struct addrinfo hints, *servinfo, *servinfop;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int rotctld_socket = 0;
    int retval = getaddrinfo(rotctld_host, rotctld_port, &hints, &servinfo);
    if (retval != 0) {
        fprintf(stderr, "getaddrinfo failure\n");
        return -1;
    }

    for (servinfop = servinfo; servinfop != NULL; servinfop = servinfop->ai_next) {
        if ((rotctld_socket = socket(servinfop->ai_family, servinfop->ai_socktype,
            servinfop->ai_protocol)) == -1) {
            continue;
        }
        if (connect(rotctld_socket, servinfop->ai_addr, servinfop->ai_addrlen) == -1) {
            close(rotctld_socket);
            continue;
        }

        break;
    }
    if (servinfop == NULL) {
        fprintf(stderr, "servinfop failure\n");
        return -1;
    }
    freeaddrinfo(servinfo);
    return rotctld_socket;
}

int rotctld_sock_readline(int sockd, char *message, size_t bufsize)
{
    size_t len=0, pos=0;
    char c='\0';

    if (message!=NULL) {
        message[bufsize-1]='\0';
    }

    do {
        len = recv(sockd, &c, 1, MSG_WAITALL);
        if (len <= 0) {
            break;
        }
        if (message!=NULL) {
            message[pos]=c;
            message[pos+1]='\0';
        }
        pos+=len;
    } while (c!='\n' && pos<bufsize-2);

    return pos;
}

void rotctld_set_position(int socket, double azimuth, double elevation)
{
    char message[256];
    sprintf(message, "P %.2f %.2f\n", azimuth, elevation);
    send(socket, message, strlen(message), MSG_NOSIGNAL);
}

void rotctld_get_position(int socket, float *azimuth, float *elevation)
{
    //ask rotctld for position
    char message[256];
    sprintf(message, "p\n");
    send(socket, message, strlen(message), MSG_NOSIGNAL);

    //read back position
    rotctld_sock_readline(socket, message, sizeof(message));
    sscanf(message, "%f\n", azimuth);
    rotctld_sock_readline(socket, message, sizeof(message));
    sscanf(message, "%f\n", elevation);
}

void rotctld_stop(int socket)
{
    char message[256];
    sprintf(message, "S\n");
    send(socket, message, strlen(message), MSG_NOSIGNAL);
}
