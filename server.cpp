#include "server.h"
#include "clientModel.h"


/*函数：设置sock为non-blocking mode*/
void setSockNonBlock(int sock)
{
	int flags;
	flags = fcntl(sock, F_GETFL, 0);
	if (flags < 0)
	{
		Die("fcntl(F_GETFL) failed");
	}
	if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0)
	{
		Die("fcntl(F_SETFL) failed");
	}
}

/*初始化listen socket*/
int initSocket(optType &myOptType)
{
	int serversock, flag;
	sockaddr_in echoserver;
	if ((serversock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		Die("Failed to create socket");
	}
	if (!myOptType.isBlock) //非阻塞设置
		setSockNonBlock(serversock);
	memset(&echoserver, 0, sizeof(echoserver));			  /* Clear struct */
	echoserver.sin_family = AF_INET;					  /* Internet/IP */
	echoserver.sin_addr.s_addr = inet_addr(myOptType.ip); /* Incoming addr */
	echoserver.sin_port = htons(myOptType.port);		  /* server port */
	if (setsockopt(serversock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int)) == -1)
	{
		Die("set socket error");
	}
	/* Bind the server socket */
	if (bind(serversock, (struct sockaddr *)&echoserver, sizeof(echoserver)) < 0)
	{
		Die("Failed to bind the server socket");
	}
	/* Listen on the server socket */
	if (listen(serversock, MAXPENDING) < 0)
	{
		Die("Failed to listen on server socket");
	}
	return serversock;
}

void ClientWorkPlace(int clientsock, int isBlock)
{
	signal(SIGPIPE, SIG_IGN);
	fd_set readfds, readfdsbak;
	struct timeval timeout;
	timeout.tv_sec = recvTle; //等下select用到这个
	timeout.tv_usec = 0;
	FD_ZERO(&readfds);
	FD_SET(clientsock, &readfds);
	ClientModel myClient;
	myClient.init(clientsock, isBlock);
	if (!isBlock)
		setSockNonBlock(clientsock);
	if (myClient.sendC(strlen("StuNo"), "StuNo") < 0)
	{
		printf("sendFailed\n");
		myClient.closeC();
		return;
	}
	while (1)
	{
		if (!isBlock)
		{
			int ret;
			readfdsbak = readfds;
			if ((ret = select(clientsock + 1, &readfdsbak, NULL, NULL, &timeout)) < 0) //超时处理
			{
				if ((errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN))
					continue;
				else
					Die("select failed");
			}
			else if (ret == 0)
			{
				printf("TLE\n");
				myClient.setStatus(errorSta);
			}
		}
		char buffer[recvBufferSize], MyTime[30];
		int ret;
		char SnoBuffer[10];
		int MySnoInt;
		int MyPidInt;
		char PidBuffer[10];
		int cRand;
		switch (myClient.getStatus())
		{
		case sendStnoSta:
			myClient.setRequiredSize(4);
			myClient.setStatus(recvStnoSta);
		case recvStnoSta:
			ret = myClient.recvC(buffer);
			if (ret < 0)
			{
				printf("recv error\n");
				myClient.setStatus(errorSta);
				break;
			}
			if (myClient.getRequiredSize() > 0) //错误检查，在此只进行了对于长度的检查
			{
				if ((myClient.getRequiredSize() != 4))
				{
					printf("recv Stno ERROR\n");
					myClient.setStatus(errorSta);
				}
			}
			else
			{
				MySnoInt = ntohl((*(int *)buffer));
				sprintf(SnoBuffer, "%d", MySnoInt);
				myClient.setClientStuNo(SnoBuffer);
				myClient.setStatus(sendPidSta);
			}
			printf("Sno:\t%d\n",MySnoInt);
			if (myClient.getRequiredSize() > 0)
				break;
		case sendPidSta:
			myClient.setRequiredSize(4);
			myClient.setStatus(recvPidSta);
			if (myClient.sendC(strlen("pid"), "pid") < 0)
			{
				printf("sendPidError\n");
				myClient.setStatus(errorSta);
			}
			break;
		case recvPidSta:
			ret = myClient.recvC(buffer);
			if (ret < 0)
			{
				printf("recv Pid Error\n");
				myClient.setStatus(errorSta);
				break;
			}
			if (myClient.getRequiredSize() > 0) //
			{
				if ((myClient.getRequiredSize() != 4))
				{
					printf("recv Pid Error\n");
					myClient.setStatus(errorSta);
				}
			}
			else
			{
				MyPidInt = ntohl((*(int *)buffer));
				sprintf(PidBuffer, "%d", MyPidInt);
				myClient.setClientPid(PidBuffer);
				myClient.setStatus(sendTimeSta);
			}
			printf("%d pid %d\n",getpid(),MyPidInt);
			if (myClient.getRequiredSize() > 0)
				break;
		case sendTimeSta:
			myClient.setRequiredSize(19);
			myClient.setStatus(recvTimeSta);
			if (myClient.sendC(strlen("TIME") + 1, "TIME") < 0)
			{
				printf("sendError\n");
				myClient.setStatus(errorSta);
			}
			strcpy(MyTime, "");
			break;
		case recvTimeSta:
			ret = myClient.recvC(buffer);
			if (ret < 0)
			{
				printf("recv timeError\n");
				myClient.setStatus(errorSta);
				break;
			}
			buffer[ret] = 0;
			if (myClient.getRequiredSize() > 0) //
			{
				if (((myClient.getRequiredSize() != 19)))
				{
					printf("recv timeError\n");
					myClient.setStatus(errorSta);
				}
			}
			else
			{
				strcat(MyTime, buffer);
				myClient.setClientTime(MyTime);
				myClient.setStatus(sendRandSta);
			}
			printf("%s\n",MyTime);
			if (myClient.getRequiredSize() > 0)
				break;
		case sendRandSta:
			myClient.setFileName();
			if (!myClient.writeFileHead())
			{
				myClient.setStatus(errorSta);
				break;
			}
			cRand = myClient.getRandom();
			myClient.setRequiredSize(cRand);
			myClient.setStatus(recvRandSta);
			sprintf(buffer, "str%d", cRand);
			if (myClient.sendC(strlen(buffer) + 1, buffer) < 0)
			{
				myClient.setStatus(errorSta);
			}
			break;
		case recvRandSta: 
			ret = myClient.recvC(buffer);
			if (ret < 0)
			{
				printf("rand recv error\n");
				myClient.setStatus(errorSta);
				break;
			}
			if (!myClient.writeAppend(buffer, ret))
			{
				printf("rand append error\n");
				myClient.setStatus(errorSta);
				break;
			}
			if (myClient.getRequiredSize() == 0)
			{
				myClient.setStatus(sendEndSta);
			}
			if (myClient.getRequiredSize() > 0)
				break;
		case sendEndSta:
			myClient.setRequiredSize(0);
			myClient.setStatus(waitEnd);
			if (myClient.sendC(strlen("end"), "end") < 0)
			{
				myClient.setStatus(errorSta);
			}
			break;
		case waitEnd:
			if ((ret=myClient.recvC(buffer)) <= 0)
			{
				myClient.closeC();
				return;
			}
			else
			{
				myClient.setStatus(errorSta);
			}
			break;
		default:
			break;
		}
		printf("all ok\n");
		if (myClient.getStatus() == errorSta)
		{
			printf("file removed\n");
			myClient.removeFile();
			myClient.closeC();
			return;
		}
	}
}

void HandleClientForkMode(int serversock, int isBlock)
{
	while (1)
	{
		int clientsock, child_pid;
		sockaddr_in echoclient;
		memset(&echoclient, 0, sizeof(echoclient));
		unsigned int clientlen = sizeof(echoclient);
		if ((clientsock = accept(serversock, (struct sockaddr *)&echoclient, &clientlen)) < 0)
			continue;
		if ((child_pid = fork()) == 0)
		{
			close(serversock);
			ClientWorkPlace(clientsock, isBlock);
			exit(0);
		}
		else if (child_pid > 0)
			close(clientsock);
	}
}

void HandleClientNoForkMode(int serversock, int isBlock)
{
	signal(SIGPIPE, SIG_IGN);
	ClientModel myClients[MAXCLIENT];
	fd_set readfds, readfdsbak;
	FD_ZERO(&readfds);
	FD_SET(serversock, &readfds);
	int maxfds = serversock;
	while (1)
	{
		readfdsbak = readfds;
		if (select(maxfds + 1, &readfdsbak, NULL, NULL, NULL) < 0)
		{
			if ((errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN))
			{
				continue;
			}
			else
			{
				Die("select failed");
			}
		}
		for (int i = 0; i <= maxfds; i++)
		{
			if (!FD_ISSET(i, &readfdsbak)) //留待时间检查处理
			{
				if (FD_ISSET(i, &readfds) && i != serversock)
				{
					if (!myClients[i].judgeTLE())
					{
						myClients[i].removeFile();
						myClients[i].closeC();
						FD_CLR(i, &readfds);
					}
				}
				continue;
			}
			if (i == serversock)
			{
				int clientsock;
				sockaddr_in echoclient;
				memset(&echoclient, 0, sizeof(echoclient));
				unsigned int clientlen = sizeof(echoclient);
				if ((clientsock = accept(serversock, (struct sockaddr *)&echoclient, &clientlen)) < 0)
					continue;
				if (!isBlock)
					setSockNonBlock(clientsock);
				myClients[clientsock].init(clientsock, isBlock);
				if (myClients[clientsock].sendC(strlen("StuNo"), "StuNo") < 0)
				{
					myClients[clientsock].closeC();
					continue;
				}
				if (clientsock > maxfds)
					maxfds = clientsock;
				FD_SET(clientsock, &readfds);
			}
			else
			{
				int ret;
				char buffer[recvBufferSize];
				char MyTime[30];
				char SnoBuffer[10];
				int MySnoInt;
				int MyPidInt;
				char PidBuffer[10];
				int cRand;
				while (1)
				{
					switch (myClients[i].getStatus())
					{
					case sendStnoSta:
						myClients[i].setRequiredSize(4);
						myClients[i].setStatus(recvStnoSta);
						continue;
					case recvStnoSta: //收4字节网络序学号，发“pid”
						ret = myClients[i].recvC(buffer);
						if (ret < 0)
						{
							myClients[i].setStatus(errorSta);
							break;
						}
						if (myClients[i].getRequiredSize() > 0) //假设其一次必然能够接受全部
						{
							if ((myClients[i].getRequiredSize() != 4))
							{
								myClients[i].setStatus(errorSta);
							}
						}
						else
						{
							MySnoInt = ntohl((*(int *)buffer));
							sprintf(SnoBuffer, "%d", MySnoInt);
							myClients[i].setClientStuNo(SnoBuffer);
							myClients[i].setStatus(sendPidSta);
							continue;
						}
						break;
					case sendPidSta:
						myClients[i].setRequiredSize(4);
						myClients[i].setStatus(recvPidSta);
						if (myClients[i].sendC(strlen("pid"), "pid") < 0)
						{
							myClients[i].setStatus(errorSta);
							break;
						}
						continue;
					case recvPidSta: //收4字节网络序int pid，发TIME
						ret = myClients[i].recvC(buffer);
						if (ret < 0)
						{
							myClients[i].setStatus(errorSta);
							break;
						}
						if (myClients[i].getRequiredSize() > 0) //
						{
							if ((myClients[i].getRequiredSize() != 4))
							{
								myClients[i].setStatus(errorSta);
							}
						}
						else
						{
							MyPidInt = ntohl((*(int *)buffer));
							sprintf(PidBuffer, "%d", MyPidInt);
							myClients[i].setClientPid(PidBuffer);
							myClients[i].setStatus(sendTimeSta);
							continue;
						}
						break;
					case sendTimeSta:
						myClients[i].setRequiredSize(19);
						myClients[i].setStatus(recvTimeSta);
						if (myClients[i].sendC(strlen("TIME") + 1, "TIME") < 0)
						{
							myClients[i].setStatus(errorSta);
							break;
						}
						strcpy(MyTime, "");
						continue;
					case recvTimeSta: //收19字节时间字符串，发str+五位随机数
						ret = myClients[i].recvC(buffer);
						if (ret < 0)
						{
							myClients[i].setStatus(errorSta);
							break;
						}
						buffer[ret] = 0;
						if (myClients[i].getRequiredSize() > 0) //时间检查处理
						{
							if (((myClients[i].getRequiredSize() != 19)))
							{
								myClients[i].setStatus(errorSta);
							}
						}
						else
						{
							strcat(MyTime, buffer);
							myClients[i].setClientTime(MyTime);
							myClients[i].setStatus(sendRandSta);
							continue;
						}
						break;
					case sendRandSta:
						myClients[i].setFileName();
						if (!myClients[i].writeFileHead())
						{
							myClients[i].setStatus(errorSta);
							break;
						}
						cRand = myClients[i].getRandom();
						myClients[i].setRequiredSize(cRand);
						myClients[i].setStatus(recvRandSta);
						sprintf(buffer, "str%d", cRand);
						if (myClients[i].sendC(strlen(buffer) + 1, buffer) < 0)
						{
							myClients[i].setStatus(errorSta);
							break;
						}
						continue;
					case recvRandSta: //收5位数的随机数个数据，然后发end
						ret = myClients[i].recvC(buffer);
						if (ret < 0)
						{
							myClients[i].setStatus(errorSta);
							break;
						}
						if (myClients[i].getRequiredSize() > 0) //时间检查处理
						{
							if (!myClients[i].judgeTLE())
							{
								myClients[i].setStatus(errorSta);
								break;
							}
						}
						if (!myClients[i].writeAppend(buffer, ret))
						{
							myClients[i].setStatus(errorSta);
							break;
						}
						if (myClients[i].getRequiredSize() == 0)
						{
							myClients[i].setStatus(sendEndSta);
							continue;
						}
						break;
					case sendEndSta:
						myClients[i].setRequiredSize(0);
						myClients[i].setStatus(waitEnd);
						if (myClients[i].sendC(strlen("end"), "end") < 0)
						{
							myClients[i].setStatus(errorSta);
						}
						break;
					case waitEnd:
						if (myClients[i].recvC(buffer) <=0)
						{
							myClients[i].closeC();
							FD_CLR(i, &readfds);
						}
						else
							myClients[i].setStatus(errorSta);
						break;
					default:
						break;
					}
					break;
				}
				if (myClients[i].getStatus() == errorSta)
				{
					myClients[i].removeFile();
					myClients[i].closeC();
					FD_CLR(i, &readfds);
				}
			}
		}
	}
}
void HandleClient(int serversock, optType &myOptType)
{
	if (myOptType.isFork)
		HandleClientForkMode(serversock, myOptType.isBlock);
	else
		HandleClientNoForkMode(serversock, myOptType.isBlock);
}

int main(int argc, char *argv[])
{
	/* init daemon */
	init_deamon();
	/*处理子进程的HLD信号，避免僵尸进程的出现,同时要避免发送失败的时候直接挂掉的情况出现*/
	signal(SIGCHLD, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	/* Server terminal's command parser*/
	optType *myOptType;
	getOptType(argc, argv, &myOptType, true);
	/*init mylisten socket*/
	int serversock;
	serversock = initSocket(*myOptType);
	/*handle client connect*/
	HandleClient(serversock, *myOptType);
	return 0;
}
