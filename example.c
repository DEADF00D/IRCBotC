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
