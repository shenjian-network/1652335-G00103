#include "clientModel.h"

void ClientModel::init(int socket, bool IsBlock)
{
    mySocket = socket;
    isBlock = IsBlock;
    strcpy(fileName, "");
    strcpy(clientPid, "");
    strcpy(clientStuNo, "");
    strcpy(clientTime, "");
    requiredSize = 0;
    t = time(0);
    myStatus=sendStnoSta;
}

void ClientModel::setRequiredSize(int size)
{
    requiredSize = size;
}

void ClientModel::setClientStuNo(char *stuNo)
{
    strcpy(clientStuNo, stuNo);
}

void ClientModel::setClientPid(char *pid)
{
    strcpy(clientPid, pid);
}

void ClientModel::setClientTime(char *timer)
{
    strcpy(clientTime, timer);
}

void ClientModel::setFileName()
{
    strcpy(fileName, "./");
    strcat(fileName, clientStuNo);
    strcat(fileName, ".");
    strcat(fileName, clientPid);
    strcat(fileName, ".pid.txt");
}

/*在对方断开的时候返回-1，否则返回0*/
int ClientModel::sendC(int size,const char *buffer) //必须发送完足够的字节之后才会返回，除非对端挂掉或者网络异常
{
    while (size > 0)
    {
        int SendSize; 
        if(isBlock)
            SendSize=send(mySocket, buffer, size, 0);
        else
            SendSize=send(mySocket,buffer,size,MSG_DONTWAIT);
        if (SendSize <= 0)
        {
            if ((errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN))
                continue;
            else
                return -1;
        }
        size = size - SendSize; //用于循环发送且退出功能
        buffer += SendSize;     //用于计算已发buffer的偏移量
    }
    t = time(0);
    return 0;
}

/*该部分在对端断开时，均返回-1*/
/*该部分在阻塞模式下，倘若10s未得到回应，则返回-1，在收到超出要求的大小时，也返回-1，其他情况下返回接受的字节（等于requiredSize）*/
/*该部分在非阻塞模式下，只会recv一次,倘若非正常错误，返回-1，正常情况但是并未接受满或者未能接受，返回成功接受的字节数*/
int ClientModel::recvC(char *buffer) //非阻塞接受的情况下，考虑未收完，收到内容超过要求值的情况 阻塞情况也要考虑未收完的情况，和收过多的情况 同时考虑对端断开的情况 每次接受更新时间戳
{
    int befSize=requiredSize;
    while (requiredSize > 0) //剩余部分大于0
    {
        int RecvSize;
        if (isBlock)
        {
            /*该部分做阻塞超时的特殊处理*/
            int selectRet;
            fd_set rfd; // 描述符集 这个将用来测试有没有一个可用的连接
            struct timeval timeout;
            FD_ZERO(&rfd);       //总是这样先清空一个描述符集
            timeout.tv_sec = recvTle; //等下select用到这个
            timeout.tv_usec = 0;
            FD_SET(mySocket,&rfd); 
            if((selectRet=select(mySocket+1,&rfd,NULL,NULL, &timeout))==0)
            {
                return -1;
            } 
            else if(selectRet<0)
            {
                if ((errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN))//该部分不确定是否正确，但一般情况不会返回负数
                    continue;               
            }
            RecvSize = recv(mySocket, buffer, recvBufferSize, 0);
        }
        else
        {
            RecvSize = recv(mySocket, buffer, recvBufferSize, MSG_DONTWAIT);
        }
        if (RecvSize < 0)
        {
            if ((errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN))
            {
                if(isBlock)
                    continue;
                else
                    break;
            }         
            return -1;   
        }
        else if (RecvSize == 0)
        {
            return -1;
        }
        requiredSize = requiredSize - RecvSize;
        buffer += RecvSize;
        if(!isBlock)
            break;
    }
    if(requiredSize<0)
        return -1;
    if(befSize!=requiredSize)
        t=time(0);
    return (befSize-requiredSize);
}

void ClientModel::closeC()
{
    close(mySocket);
}

bool ClientModel::writeFileHead()
{
    setFileName();
    char buffer[50];
    int writeSize;
    FILE* myClientFile;
    myClientFile=fopen(fileName,"w+");
    if(myClientFile==NULL)
    {
        return false;
    }
    strcpy(buffer,clientStuNo);
    strcat(buffer,"\n");
    writeSize=fwrite(buffer,sizeof(char),strlen(buffer),myClientFile);
    if(writeSize<0)
    {
        fclose(myClientFile);
        return false;
    }
    strcpy(buffer,clientPid);
    strcat(buffer,"\n");
    writeSize=fwrite(buffer,sizeof(char),strlen(buffer),myClientFile);
    if(writeSize<0)
    {
        fclose(myClientFile);
        return false;
    }
    strcpy(buffer,clientTime);
    strcat(buffer,"\n");
    writeSize=fwrite(buffer,sizeof(char),strlen(buffer),myClientFile);
    if(writeSize<0)
    {
        fclose(myClientFile);
        return false;
    }
    fclose(myClientFile);
    return true;
}

bool ClientModel::writeAppend(char* buffer,int size)//带尾0
{
    setFileName();
    int writeSize;
    FILE* myClientFile;
    myClientFile=fopen(fileName,"a+");
    if(myClientFile==NULL)
        return false;  
    writeSize=fwrite(buffer,sizeof(char),size,myClientFile);
    fclose(myClientFile);
    if(writeSize<0)
    {
        return false;
    }
    return true;         
}

void ClientModel::removeFile()
{
    remove(fileName);
}

bool ClientModel::judgeTLE()
{
    time_t now=time(0);
    if(now-t>=recvTle)
        return false;
    return true;
}

int ClientModel::getRandom()
{
    return (rand()%(99999-32768)+32768);
}

ClientStatus ClientModel::getStatus()
{
    return myStatus;
}
void ClientModel::setStatus(ClientStatus sta)
{
    myStatus=sta;
}
ClientModel::ClientModel()
{
    mySocket = -1;
    isBlock = 0;
    strcpy(fileName, "");
    strcpy(clientPid, "");
    strcpy(clientStuNo, "");
    strcpy(clientTime, "");
    requiredSize = 0;
    t = time(0);
    myStatus=sendStnoSta;
}

int ClientModel::getRequiredSize()
{
    return requiredSize;
}

