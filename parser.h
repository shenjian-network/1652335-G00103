#ifndef PARSE_1652335
#define PARSE_1652335
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
//parser�ӿ�
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
#endif


