/* Utilities
 */
#ifndef UTILS_H 
#define UTILS_H
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>

//传递参数的结构体
struct optType{
    int isBlock;
    int isFork;
    char* ip;
    int port;
    int linkNum;//只有client会用到，但是都做了处理
};

//失败则exit(1) argc:参数个数 argv：参数 optType：待填充的类型（如&myOptType） isServer（是否为server） not Server:isClient 
void getOptType(int argc,char** argv,struct optType** myOptTypeAddr,bool isServer);

// 建立deamon的函数
void init_deamon(void);

// 退出的函数
void Die(const char*msg);

#endif // UTILS_H
