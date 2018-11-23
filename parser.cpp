#include "parser.h"
void Die(char *mess);
void tip();
void Die(char *mess)
{
	perror(mess);
	exit(1);
}
void tip(bool isServer)
{
    printf("usage:\n");
    printf("\t--ip\t��������Ϊserver�˵�ip\n");
    printf("\t--port\t��������Ϊserver�˵Ķ˿�\n");
    printf("\t--block\t����Ϊ����ģʽ����nonblock��ͻ��\n");
    printf("\t--nonblock\t����Ϊ������ģʽ����block��ͻ��\n");
    printf("\t--fork\tfork��ʽִ�ж����ӣ���nofork��ͻ��\n");
    printf("\t--nofork\t��������ʽִ�ж����ӣ���fork��ͻ��\n");
    if(!isServer)
    {
        printf("\t--num\t�������ӵĸ���(client)\n");
        printf("\tʾ��: ./test --ip 192.168.6.1 --port 8080 --block --fork --num 13\n");
    }
    else
        printf("\tʾ��: ./test --ip 192.168.6.1 --port 8080 --block --fork\n");
    exit(1);
}
void getOptType(int argc,char** argv,struct optType** myOptTypeAddr,bool isServer)
{
    struct optType* myOptType;
    int opt;
    int option_index=0;
    opterr=0;
    myOptType=(struct optType*)malloc(sizeof(struct optType));
    (*myOptTypeAddr)=myOptType;
    memset((myOptType),0xff,sizeof(struct optType));
    myOptType->ip=(char*)malloc(30*sizeof(char));
    strcpy(myOptType->ip,"null");
    static struct option long_options[]={
        {"ip",required_argument,NULL,0},
        {"port",required_argument,NULL,0},
        {"block",no_argument,NULL,0},
        {"nonblock",no_argument,NULL,0},
        {"fork",no_argument,NULL,0},
        {"nofork",no_argument,NULL,0},
        {"num",required_argument,NULL,0},
        {NULL,    no_argument   ,NULL, 0},
    };
    while((opt =getopt_long(argc,argv,"",long_options,&option_index))!= -1)
    {
        if(opt!=0)
            tip(isServer);
        switch(option_index){
            case 0:
                strcpy(myOptType->ip,optarg);
            break;
            case 1:
                myOptType->port=atoi(optarg);
                if(myOptType->port==0)
                    tip(isServer);
            break;
            case 2: 
                if(myOptType->isBlock==-1)
                {
                    myOptType->isBlock=1;
                }
                else
                    tip(isServer);
            break;
            case 3: 
                if(myOptType->isBlock==-1)
                    myOptType->isBlock=0;
                else
                    tip(isServer);
            break;
            case 4: 
                if(myOptType->isFork==-1)
                    myOptType->isFork=1;
                else
                    tip(isServer);
            break;
            case 5: 
                if(myOptType->isFork==-1)
                    myOptType->isFork=0;
                else
                    tip(isServer);
            break;
            case 6: 
                if(optarg==NULL)
                    tip(isServer);
                if(myOptType->linkNum==-1)
                {
                    myOptType->linkNum=atoi(optarg);
                    if(myOptType->linkNum==0)
                        tip(isServer);
                }
                else
                    tip(isServer);
            break;
            default:
            break;
        }
    }
    if(strcmp(myOptType->ip,"null")==0)
        if(isServer)
            strcpy(myOptType->ip,"0.0.0.0");
        else
            tip(isServer);
    if(myOptType->port==-1)
        tip(isServer);
    if(myOptType->linkNum==-1)
        myOptType->linkNum=100;
    if(myOptType->isBlock==-1)
        myOptType->isBlock=0;
    if(myOptType->isFork==-1)
        myOptType->isFork=0;
    return ;
}
