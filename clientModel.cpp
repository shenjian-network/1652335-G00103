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

/*�ڶԷ��Ͽ���ʱ�򷵻�-1�����򷵻�0*/
int ClientModel::sendC(int size,const char *buffer) //���뷢�����㹻���ֽ�֮��Ż᷵�أ����ǶԶ˹ҵ����������쳣
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
        size = size - SendSize; //����ѭ���������˳�����
        buffer += SendSize;     //���ڼ����ѷ�buffer��ƫ����
    }
    t = time(0);
    return 0;
}

/*�ò����ڶԶ˶Ͽ�ʱ��������-1*/
/*�ò���������ģʽ�£�����10sδ�õ���Ӧ���򷵻�-1�����յ�����Ҫ��Ĵ�Сʱ��Ҳ����-1����������·��ؽ��ܵ��ֽڣ�����requiredSize��*/
/*�ò����ڷ�����ģʽ�£�ֻ��recvһ��,�������������󣬷���-1������������ǲ�δ����������δ�ܽ��ܣ����سɹ����ܵ��ֽ���*/
int ClientModel::recvC(char *buffer) //���������ܵ�����£�����δ���꣬�յ����ݳ���Ҫ��ֵ����� �������ҲҪ����δ�������������չ������� ͬʱ���ǶԶ˶Ͽ������ ÿ�ν��ܸ���ʱ���
{
    int befSize=requiredSize;
    while (requiredSize > 0) //ʣ�ಿ�ִ���0
    {
        int RecvSize;
        if (isBlock)
        {
            /*�ò�����������ʱ�����⴦��*/
            int selectRet;
            fd_set rfd; // �������� ���������������û��һ�����õ�����
            struct timeval timeout;
            FD_ZERO(&rfd);       //�������������һ����������
            timeout.tv_sec = recvTle; //����select�õ����
            timeout.tv_usec = 0;
            FD_SET(mySocket,&rfd); 
            if((selectRet=select(mySocket+1,&rfd,NULL,NULL, &timeout))==0)
            {
                return -1;
            } 
            else if(selectRet<0)
            {
                if ((errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN))//�ò��ֲ�ȷ���Ƿ���ȷ����һ��������᷵�ظ���
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

bool ClientModel::writeAppend(char* buffer,int size)//��β0
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

