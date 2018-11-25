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


/*Server�˶�Ӧ��Client���շ�״̬��*/
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


/*Client��ģ��*/
class ClientModel{
private:
    ClientStatus myStatus;
    int mySocket;
    bool isBlock;
    char clientStuNo[30];//��β0
    char clientPid[30];//��β0
    char clientTime[30];//��β0
    char fileName[100];//��β0
    int requiredSize;
    //��������һ��ʱ������Ӷ�������������ܵ�����£��ղ�����һֱռ����Դ
    time_t t;
public:
    ClientModel();
    void init(int socket,bool IsBlock);
    void setRequiredSize(int size);
    void setClientStuNo(char* stuNo);
    void setClientPid(char* pid);
    void setClientTime(char* timer);
    void setFileName();
    /*�ڶԷ��Ͽ���ʱ�򷵻�-1�����򷵻�0*/
    int sendC(int size,const char* content);//��������ʱ������,ͬʱ���ǶԶ˶Ͽ������  ÿ�η��͸���ʱ���
    /*�ò����ڶԶ˶Ͽ�ʱ��������-1*/
    /*�ò���������ģʽ�£�����10sδ�õ���Ӧ���򷵻�-1�����յ�����Ҫ��Ĵ�Сʱ��Ҳ����-1����������·��ؽ��ܵ��ֽڣ�����requiredSize��*/
    /*�ò����ڷ�����ģʽ�£�ֻ��recvһ��,�������������󣬷���-1������������ǲ�δ����������δ�ܽ��ܣ����سɹ����ܵ��ֽ���*/
    int recvC(char* recvBuffer);//���������ܵ�����£�����δ���꣬�յ����ݳ���Ҫ��ֵ����� �������ҲҪ����δ�������������չ������� ͬʱ���ǶԶ˶Ͽ������ ÿ�ν��ܸ���ʱ���
    void closeC();//��������رնԶ�����
    bool writeFileHead();
    bool writeAppend(char* content,int size);//content�޸�ʽҪ��
    void removeFile();
    bool judgeTLE();
    int getRandom();
    ClientStatus getStatus();
    void setStatus(ClientStatus sta);
    int getRequiredSize();
};
#endif
