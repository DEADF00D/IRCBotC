# IRCBotC
IRCBotC is a very lightweight library written in C to build IRC Bots.
## Documentation:

You must create a IRC_Instance structure at the biggining of your program.\
It will be needed in a lot of IRCBotC functions, and it store all connection informations.\

```
struct IRC_Instance{
    int Socket;
    IRC_AddrType Type;
    struct sockaddr_in AddrIpv4;
    struct sockaddr_in6 AddrIpv6;
}
```

### Structures:

#### IRC_Packet:
```
struct IRC_Packet{
    char **head,*body,*raw;
    int head_count;
};
```
Store a packet like it:
```
:freenode.net NOTICE * :*** Looking up your hostname...
```
in
``` 
head[0]="freenode.net";
head[1]="NOTICE"
head[2]="*"
head_count=3
body="*** Looking up your hostname..."
```

#### IRC_Host:

```
typedef struct{
    char *nick, *user, *host;
} IRC_Host;
```

Store a hostname like:
```
deadfood!deadfood@127.0.0.1
```
in
```
nick="deadfood"
user="deadfood"
host="127.0.0.1"
```

### High level functions:
```int IRC_Connect(IRC_Instance *irc, char *host, int port)``` \
  **Connect to remote irc server.**
  - host: Remote host to connect, can be a dns, and ipv4 address or a ipv6 address.
  - port: Remote port to connect.
  <br/>

```int IRC_Auth(IRC_Instance *irc, char *nick, char *realname)``` \
  **Authenticate your bot to irc server.**
  - nick: Nick of your bot
  - realname: Realname of your bot
  <br/>

```int IRC_GetPacket(IRC_Instance *irc, IRC_Packet *p)``` \
  **Get next packet and parse it in p.**
  - p: IRC_Packet pointer structure, packet parsing will be stored in.
  <br/>

```int IRC_Query(IRC_Instance *irc, char *pname, char *params, char *body);``` \
  **Parse and send query to irc server.**
  - pname: Name of query (ex: PRIVMSG, JOIN, PART)
  - params: Parameters of query (ex: #chan)
  - body: Body of query
  <br/>

```int IRC_PrivMSG(IRC_Instance *irc, char *target, char *format, ...);``` \
  **Parse and send privmsg with a printf like function.**
  - target: Nick or Channel to send the privmsg.
  - format: printf like format (ex: "hello %s, you are %d years old.")
  <br/>

```int IRC_ParseHost(IRC_Host *irch, char *packet)``` \
  **Parse an host like ```deadfood!deadfood@127.0.0.1``` into irch.**
  - irch: IRC_Host structure pointer, where parsing will be stored.
  - packet: Host to paste.
  <br/>

### Internal/Low level functions:
```int IRC_RecvPacket(IRC_Instance *irc, char **packet)``` \
  **Receive the packet and store it in packet** \
  <br/>
```int IRC_SendPacket(IRC_Instance *irc, char *packet)``` \
  **Send the packet stored in packet** \
  <br/>
```int IRC_ConnectTo(IRC_Instance *irc, char *ip, int port, IRC_AddrType type);``` \
  **Connect to remote host but unlike ```IRC_Connect()``` it support only Ipv4 and Ipv6 ip addr.**
  - ip: remote server ip to connect.
  - port: remote irc port.
  - type: type of ip address, can be ADDR_IPV4 or ADDR_IPV6.
  <br/>
```int IRC_GetIPVersion(char *src)``` \
  **Return ip address version of str.**
  <br/>
```int IRC_ResolveHostname(char *host, char *addrstr)``` \
  **Resolve host and strcpy it in addrstr.**
  <br/>

## example.c:
```
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "irc.h"

int main(){
    IRC_Instance irc;
    
    printf("Connection to freenode...\n");
    if(!IRC_Connect(&irc,"chat.freenode.net",6667)){
        printf("Connection error.\n");
        return 1;
    }
    printf("Now connected.\n");
    IRC_Auth(&irc,"IRCBotC","I'm a cool bot built with IRCBotC.");
    printf("Authenticated.\n");
    
    IRC_Query(&irc,"JOIN","#bottest",NULL);
    
    IRC_Packet p;
    while(IRC_GetPacket(&irc,&p)){
        printf("%s\n",p.body);
        if(p.head_count>1 && strcmp(p.head[1],"PRIVMSG")==0){
            if(strcmp(p.body,"!github")==0){
                IRC_PrivMSG(&irc, p.head[2], "https://github.com/DEADF00D/IRCBotC/");
            }
        }
        else if(p.head_count==0 && strcmp(p.head[0],"PING")==0){
            printf("PONG !\n");
            IRC_Query(&irc,"PONG",p.body,NULL);
        }
        else if(p.head_count>=0 && strcmp(p.head[0],"ERROR")==0){
            printf("(error) %s\n",p.body);
        }
    }
    
    return 0;
}
```
