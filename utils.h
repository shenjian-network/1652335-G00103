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

//���ݲ����Ľṹ��
struct optType{
    int isBlock;
    int isFork;
    char* ip;
    int port;
    int linkNum;//ֻ��client���õ������Ƕ����˴���
};

//ʧ����exit(1) argc:�������� argv������ optType�����������ͣ���&myOptType�� isServer���Ƿ�Ϊserver�� not Server:isClient 
void getOptType(int argc,char** argv,struct optType** myOptTypeAddr,bool isServer);

// ����deamon�ĺ���
void init_deamon(void);

// �˳��ĺ���
void Die(const char*msg);

#endif // UTILS_H
