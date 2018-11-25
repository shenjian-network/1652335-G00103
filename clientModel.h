#ifndef CLIENTMODEL_1652335
#define CLIENTMODEL_1652335

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <iostream>
#include <string>
#include <ctime>

#include "utils.h"

using namespace std;
#define recvBufferSize 100000
#define recvTle 5


/*Server端对应的Client的收发状态表*/
enum ClientStatus{
    sendStnoSta,
    recvStnoSta,
    sendPidSta,
    recvPidSta,
    sendTimeSta,
    recvTimeSta,
    sendRandSta,
    recvRandSta,
    sendEndSta,
    waitEnd,
    errorSta
};


/*Client端模型*/
class ClientModel{
private:
    ClientStatus myStatus;
    int mySocket;
    bool isBlock;
    char clientStuNo[30];//带尾0
    char clientPid[30];//带尾0
    char clientTime[30];//带尾0
    char fileName[100];//带尾0
    int requiredSize;
    //可以设置一个时间戳，从而避免非阻塞接受的情况下，收不够而一直占用资源
    time_t t;
public:
    ClientModel();
    void init(int socket,bool IsBlock);
    void setRequiredSize(int size);
    void setClientStuNo(char* stuNo);
    void setClientPid(char* pid);
    void setClientTime(char* timer);
    void setFileName();
    /*在对方断开的时候返回-1，否则返回0*/
    int sendC(int size,const char* content);//发阻塞暂时不考虑,同时考虑对端断开的情况  每次发送更新时间戳
    /*该部分在对端断开时，均返回-1*/
    /*该部分在阻塞模式下，倘若10s未得到回应，则返回-1，在收到超出要求的大小时，也返回-1，其他情况下返回接受的字节（等于requiredSize）*/
    /*该部分在非阻塞模式下，只会recv一次,倘若非正常错误，返回-1，正常情况但是并未接受满或者未能接受，返回成功接受的字节数*/
    int recvC(char* recvBuffer);//非阻塞接受的情况下，考虑未收完，收到内容超过要求值的情况 阻塞情况也要考虑未收完的情况，和收过多的情况 同时考虑对端断开的情况 每次接受更新时间戳
    void closeC();//错误情况关闭对端连接
    bool writeFileHead();
    bool writeAppend(char* content,int size);//content无格式要求
    void removeFile();
    bool judgeTLE();
    int getRandom();
    ClientStatus getStatus();
    void setStatus(ClientStatus sta);
    int getRequiredSize();
};
#endif
