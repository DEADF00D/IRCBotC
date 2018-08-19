#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "irc.h"

int IRC_GetIPVersion(char *src) {
    char buf[16];
    if (inet_pton(AF_INET, src, buf)) {
        //printf("is ipv4\n");
        return ADDR_IPV4;
    } else if (inet_pton(AF_INET6, src, buf)) {
        //printf("is ipv6\n");
        return ADDR_IPV6;
    }
    //printf("is NATHINGGG\n");
    return -1;
}

int IRC_ResolveHostname(char *host, char *addrstr){
    struct addrinfo hints, *res;
    char addr[100];
    int errcode;
    void *ptr;

    memset (&hints, 0, sizeof (hints));
    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags |= AI_CANONNAME;

    errcode = getaddrinfo (host, NULL, &hints, &res);
    if (errcode != 0){ return 0; }

    inet_ntop (res->ai_family, res->ai_addr->sa_data, addr, 100);
    
    switch (res->ai_family){
        case AF_INET:
            ptr = &((struct sockaddr_in *) res->ai_addr)->sin_addr;
            break;
        case AF_INET6:
            ptr = &((struct sockaddr_in6 *) res->ai_addr)->sin6_addr;
            break;
    }
    inet_ntop (res->ai_family, ptr, addr, 100);
    
    strcpy(addrstr,addr);
        
    return 1;
}

int IRC_ConnectTo(IRC_Instance *irc, char *ip, int port, IRC_AddrType type){
    if(type==ADDR_IPV4){
        irc->Socket=socket(AF_INET, SOCK_STREAM, 0);
        if(irc->Socket==-1){return -1;}
        
        irc->AddrIpv4.sin_family=AF_INET;
        irc->AddrIpv4.sin_addr.s_addr=inet_addr(ip);
        irc->AddrIpv4.sin_port=htons(port);
        
        irc->Type=ADDR_IPV4;
        
        if(connect(irc->Socket,(struct sockaddr*)&(irc->AddrIpv4),sizeof(irc->AddrIpv4))<0){return -2;}
    }
    else if(type==ADDR_IPV6){
        irc->Socket=socket(AF_INET6, SOCK_STREAM, 0);
        if(irc->Socket==-1){return -1;}
        
        irc->AddrIpv6.sin6_family=AF_INET6;
        irc->AddrIpv6.sin6_port=htons(port);
        inet_pton(AF_INET6, ip, &(irc->AddrIpv6.sin6_addr));
        
        irc->Type=ADDR_IPV6;
        
        if(connect(irc->Socket, (struct sockaddr*)&(irc->AddrIpv6), sizeof(irc->AddrIpv6))<0){return -2;}
    }
    
    return 0;
}

int IRC_Connect(IRC_Instance *irc, char *host, int port){
    IRC_AddrType type;
    if((type=IRC_GetIPVersion(host))==-1){
        char ip[100];
        memset(ip,0,100);
        if(!IRC_ResolveHostname(host,ip)){return 0;}
        //printf("Resolved %s as %s\n",host,ip);
        if(IRC_ConnectTo(irc,ip,port,IRC_GetIPVersion(ip))<0){return 0;}
        //printf("now connected.\n");
    }else{
        if(IRC_ConnectTo(irc,host,port,type)<0){return 0;}
    }
    return 1;
}


int IRC_RecvPacket(IRC_Instance *irc, char **packet){
    *packet=(char*)calloc(1,sizeof(char));
    int recv_size=0,y=0;
    char c[1];
    
    while((recv_size=recv(irc->Socket,c,1,0))>0){
        if(c[0]=='\r' || c[0]=='\n'){break;}
        *packet=(char*)realloc(*packet,sizeof(char)*(y+3));
        (*packet)[y]=c[0];
        (*packet)[y+1]=0;
        ++y;
    }
    
    recv(irc->Socket,c,1,0);
    
    if(recv_size==0 || recv_size==-1){ return 0; }
    
    return 1;
}

int IRC_GetPacket(IRC_Instance *irc, IRC_Packet *p){
    char *packet;
    
    if(!IRC_RecvPacket(irc,&packet)){return 0;}
    
    char *head,*body;
    int i=0;
    
    if(packet[0]==':'){++i;}
    
    head=(char*)calloc(1,sizeof(char));
    for(int y=0;i<strlen(packet) && !(packet[i]==' ' && packet[i+1]==':');++i,++y){
        head=(char*)realloc(head,sizeof(char)*(y+3));
        head[y]=packet[i];
        head[y+1]=0;
    }
    
    i+=2;
    
    p->body=(char*)calloc(1,sizeof(char));
    for(int y=0;i<strlen(packet);++i,++y){
        p->body=(char*)realloc(p->body,sizeof(char)*(y+3));
        (p->body)[y]=packet[i];
        (p->body)[y+1]=0;
    }
    
    
    
    int y=0,id=0;
    (p->head)=(char**)calloc(1,sizeof(char*));
    (p->head)[0]=(char*)calloc(1,sizeof(char));
    
    for(i=0;i<strlen(head);++i){
        if(head[i]==' '){
            p->head=(char**)realloc(p->head,sizeof(char*)*(id+3));
            (p->head)[id+1]=(char*)calloc(1,sizeof(char));
            ++id;
            y=0;
        }else{
            (p->head)[id]=(char*)realloc((p->head)[id],sizeof(char)*(y+3));
            (p->head)[id][y]=head[i];
            (p->head)[id][y+1]=0;
            ++y;
        }
    }
    
    p->head_count=id;
    p->raw=packet;
    
    /*
    printf("%s\n",packet);
    printf("%s | %s\n",head,p->body);
    for(i=0;i<p->head_count+1;++i){
        printf("%s\n",(p->head)[i]);
    }
    printf("\n");
    */
    
    return 1;
}

int IRC_SendPacket(IRC_Instance *irc, char *packet){
    return send(irc->Socket,packet,strlen(packet),0);
}

int IRC_Query(IRC_Instance *irc, char *pname, char *params, char *body){
    int size=strlen(pname)+strlen(params);
    if(body!=NULL){size+=strlen(body);}
    
    char *packet=(char*)calloc(size+10,sizeof(char));
    sprintf(packet,"%s %s",pname,params);
    
    if(body!=NULL){
        sprintf(packet,"%s :%s",packet,body);
    }
    
    sprintf(packet,"%s\r\n",packet);
    
    //printf("%s\n",packet);
    
    IRC_SendPacket(irc,packet);
    
    free(packet);
}

int IRC_Auth(IRC_Instance *irc, char *nick, char *realname){
    char *nickuser=(char*)calloc(strlen(nick)+strlen(" * *")+2,sizeof(char));
    sprintf(nickuser,"%s * *",nick);
    IRC_Query(irc, "NICK", nick, NULL);
    IRC_Query(irc, "USER", nickuser, realname);
    free(nickuser);
    return 1;
}

int IRC_ParseHost(IRC_Host *irch, char *packet){
    irch->nick=(char*)calloc(1,sizeof(char));
    for(int i=0, y=0;i<strlen(packet) && packet[i]!='!';++i,++y){
        irch->nick=(char*)realloc(irch->nick,sizeof(char)*(y+3));
        irch->nick[y]=packet[i];
        irch->nick[y+1]=0;
    }
    
    irch->user=(char*)calloc(1,sizeof(char));
    for(int i=0, y=0;i<strlen(packet) && packet[i]!='@';++i,++y){
        irch->user=(char*)realloc(irch->user,sizeof(char)*(y+3));
        irch->user[y]=packet[i];
        irch->user[y+1]=0;
    }
    
    irch->host=(char*)calloc(1,sizeof(char));
    for(int i=0, y=0;i<strlen(packet);++i,++y){
        irch->host=(char*)realloc(irch->host,sizeof(char)*(y+3));
        irch->host[y]=packet[i];
        irch->host[y+1]=0;
    }
}

int IRC_PrivMSG(IRC_Instance *irc, char *target, char *format, ...){
    void *ptr;
    
    va_list vargs;
    va_start(vargs, format);
    
    int size=strlen(format);
    for(int i=0;i<strlen(format);++i){
        if(format[i]=='%'){
            if(format[i+1]=='s'){
                char* s=va_arg(vargs,char*);
                if(s!=NULL){
                    //printf("Added sizeof '%s' (%d)\n",s,strlen(s));
                    size+=strlen(s);
                }
            }
            else if(format[i+1]=='d' || format[i+1]=='f' || format[i+1]=='u' || format[i+1]=='p' || format[i+1]=='l'){
                //printf("Added sizeof integer like, +=64\n");
                size+=64;
                va_arg(vargs,void*);
            }else{
                size+=16;
                va_arg(vargs,void*);
            }
        }
    }
    size+=2;
    
    va_end(vargs);
    
    char *dest=(char*)calloc(size,sizeof(char));
    va_start(vargs, format);
    vsprintf(dest,format,vargs);
    IRC_Query(irc,"PRIVMSG",target,dest);
    va_end(vargs);
    return size;
}
