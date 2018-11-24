#include "init_demo.h"
void init_deamon(void)
{
    int pid;
    int i;
    /* 处理SIGCHLD信号。�?�理SIGCHLD信号并不�?必须的。但对于某些进程，特�?�?服务器进程往往在�?�求到来时生成子进程处理请求。�?�果父进程不等待子进程结束，子进程将成为僵尸进程（zombie）从而占用系统资源�?*/
    if(signal(SIGCHLD,SIG_IGN) == SIG_ERR){
        printf("Cant signal in init_daemon.");
        exit(1);
    }
    if(pid=fork())
        exit(0);//�?父进程，结束父进�?
    else if(pid< 0){
        perror("fail to fork1");
        exit(1);//fork失败，退�?
    }
    //�?�?一子进程，后台继续执�??
    setsid();//�?一子进程成为新的会话组长和进程组长
    //并与控制终�??分�??
    if(pid=fork())
        exit(0);//�?�?一子进程，结束�?一子进�?
    else if(pid< 0)
        exit(1);//fork失败，退�?
    //�?�?二子进程，继�?
    //�?二子进程不再�?会话组长
    //for(i=0;i< getdtablesize();++i)//关闭打开的文件描述�??
       //close(i);
    chdir("./");//改变工作�?录到/tmp
    umask(0);//重�?�文件创建掩�?
    return;
}