#ifndef CLIENTMODEL_1652335
#define CLIENTMODEL_1652335
#include "utils.h"
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

enum ToServerStatus{
    Sidle,
    SrecvStnoSta,
    SrecvPidSta,
    SrecvTimeSta,
    SrecvRandSta,
    SrecvEndSta,
    SsuccessSta
};

/*Client��ģ��*/
class ClientModel{
private:
    ClientStatus myStatus;
    ToServerStatus toStatus;
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
    /*��ʼ������client��*/
    void init(int socket,bool IsBlock);
    /*�ڽ���֮ǰ��ȷ���Լ�Ҫ���ܵ��ֽ���Ŀ*/
    void setRequiredSize(int size);
    void setClientStuNo(const char* stuNo);
    void setClientPid(char* pid);
    void setClientTime(char* timer);
    /*����pid stunoȷ���ļ�����*/
    void setFileName();
    /*�ڶԷ��Ͽ���ʱ�򷵻�-1�����򷵻�0*/
    int sendC(int size,const char* content);//������û�п��� û�п��� û�п���
    /*�ò����ڶԶ˶Ͽ�ʱ��������-1*/
    /*�ò���������ģʽ�£�����10sδ�õ���Ӧ���򷵻�-1�����յ�����Ҫ��Ĵ�Сʱ��Ҳ����-1����������·��ؽ��ܵ��ֽڣ�����requiredSize��*/
    /*�ò����ڷ�����ģʽ�£�ֻ��recvһ��,�������������󣬷���-1������������ǲ�δ����������δ�ܽ��ܣ����سɹ����ܵ��ֽ���*/
    int recvC(char* recvBuffer);//���������ܵ�����£�����δ���꣬�յ����ݳ���Ҫ��ֵ����� �������ҲҪ����δ�������������չ������� ͬʱ���ǶԶ˶Ͽ������ ÿ�ν��ܸ���ʱ���
    void closeC();//�رնԶ�����
    bool writeFileHead();//д�ļ�ͷ
    bool writeAppend(char* content,int size);//������д�Ļ�����׷�������Ŀ�ֽڵĲ���
    void removeFile();//ɾ���ļ�
    bool judgeTLE();//�жϳ�ʱ��client���Բ��ã�server�Ѿ��������
    int getRandom();//��ȡһ�������
    ClientStatus getStatus();//��õ�ǰclient���������ĸ�״̬
    void setStatus(ClientStatus sta);//����client��״̬
    int getRequiredSize();//�����Ҫ���յ��ֽ���Ŀ
    void setToStatus(ToServerStatus sta);
    ToServerStatus getToStatus();
};
#endif
