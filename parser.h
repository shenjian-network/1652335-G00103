#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
struct optType{
    int isBlock;
    int isFork;
    char* ip;
    int port;
    int linkNum;
};
void Die(char *mess);
void tip(bool isServer);
bool getOptType(int argc,char** argv,struct optType** myOptTypeAddr,bool isServer);//not Server:isClient

