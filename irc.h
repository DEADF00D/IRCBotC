#ifndef _IRC_H_
#define _IRC_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <sys/socket.h>
#include <arpa/inet.h>


typedef enum{ADDR_IPV4, ADDR_IPV6} IRC_AddrType;


typedef struct{
    int Socket;
    IRC_AddrType Type;
    struct sockaddr_in AddrIpv4;
    struct sockaddr_in6 AddrIpv6;
} IRC_Instance;

typedef struct{
    char **head,*body,*raw;
    int head_count;
} IRC_Packet;

typedef struct{
    char *nick, *user, *host;
} IRC_Host;


int IRC_ConnectTo(IRC_Instance *irc, char *ip, int port, IRC_AddrType type);
int IRC_RecvPacket(IRC_Instance *irc, char **packet);
int IRC_GetPacket(IRC_Instance *irc, IRC_Packet *p);
int IRC_SendPacket(IRC_Instance *irc, char *packet);
int IRC_Query(IRC_Instance *irc, char *pname, char *params, char *body);
int IRC_Auth(IRC_Instance *irc, char *nick, char *realname);
int IRC_ParseHost(IRC_Host *irch, char *packet);
int IRC_Connect(IRC_Instance *irc, char *host, int port);
int IRC_PrivMSG(IRC_Instance *irc, char *target, char *format, ...);


//Internal functions:
int IRC_GetIPVersion(char *src);
int IRC_ResolveHostname(char *host, char *addrstr);

#endif
