#include "client.h"
const int writeMaxn = 100000;
const int readMaxn = 50;
const bool isServer = false;

const int readLenArr[] = {5, 3, 5, 9, 3};
const int readCmpLen[] = {5, 3, 5, 3, 3};
const char *readCmpContent[] = {"StuNo", "pid", "TIME", "str", "end"};

int sendS(int mySocket, int size, const char *buffer) //必须发送完足够的字节之后才会返回，除非对端挂掉或者网络异常
{
	while (size > 0)
	{
		int SendSize = send(mySocket, buffer, size, 0);
		if (SendSize < 0)
		{
			if ((errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN))
				continue;
			else
				return -1;
		}
		size = size - SendSize; //用于循环发送且退出功能
		buffer += SendSize;		//用于计算已发buffer的偏移量
	}
	return 0;
}

int myRead(int cfd, char *&bufferRead, int depth)
{
	static int recbytes;
	static long long readTmp = 0;
	static long long readCnt = 0;
	int readLen = readLenArr[depth];
	memset(bufferRead, 0, sizeof(bufferRead));
	if (0 >= (recbytes = read(cfd, bufferRead, readLenArr[depth])))
	{
		if (recbytes == 0)
			cout << "读结束" << endl;
		else if (errno == EINTR)
			cerr << "中断错误" << endl;
		else
			cerr << "其他连接错误" << endl;
		return 0;
	}
	if (recbytes != readLen || strncmp(bufferRead, readCmpContent[depth], readCmpLen[depth]))
		return 0;
	if (depth == 3)
	{
		for (int i = 3; i < 8; ++i)
			if (!isdigit(bufferRead[i]))
				return 0;
		int tmp = atoi(bufferRead + 3);
		if (tmp < 32768 || tmp > 99999)
			return 0;
	}
	return 1;
}

void getTime(char *&bufferWrite)
{
	//Get formatted time into bufferWrite
	time_t now = time(NULL);
	tm t;
	localtime_r(&now, &t);
	strftime(bufferWrite, 20, "%Y-%m-%d %H:%M:%S", &t);
}

void generateRandomString(char *bufferWrite, const int &len)
{
	for (int i = 0; i < len; ++i)
		bufferWrite[i] = rand() % 256;
}

const int writeLenArr[] = {4, 4, 19, -1};
bool myWrite(int cfd, char *&bufferWrite, int depth, bool isFork, ofstream &fout, char *&bufferRead)
{
	static int writebytes;
	static long long writeTmp = 0;
	static long long writeCnt = 0;
	int writeLen = writeLenArr[depth];
	static int Stu = 1652335;
	int tmp;
	static int pid;
	char *write_content;
	memset(bufferWrite, 0, sizeof(bufferWrite));
	switch (depth)
	{
	case 0:
		fout << Stu << "\n";
		tmp = htonl(Stu);
		write_content = (char *)&tmp;
		writeLen = sizeof(Stu);
		break;
	case 1:
		pid = int(isFork ? getpid() : ((getpid() << 16) + cfd));
		fout << pid << "\n";
		pid = htonl(pid);
		write_content = (char *)&pid;
		writeLen = sizeof(pid);
		break;
	case 2:
		getTime(bufferWrite);
		write_content = bufferWrite;
		fout << bufferWrite << "\n";
		break;
	case 3:
		generateRandomString(bufferWrite, atoi(bufferRead + 3));
		fout.write(bufferWrite, atoi(bufferRead + 3));
		write_content = bufferWrite;
		writeLen = atoi(bufferRead + 3);
		break;
	case 4:
		close(cfd);
		cout << "收到end" << endl;
		return true;
	default:
		cerr << "How do you enter this?" << endl;
		break;
	}
	fd_set writefds;
	int nready;
	timeval tv;
	tv.tv_sec = 3; // 3s不可写那么超时，重来
	while (1)
	{
		FD_ZERO(&writefds);
		FD_SET(cfd, &writefds);
		nready = select(FD_SETSIZE, NULL, &writefds, NULL,NULL);

		// 超时
		if (nready == 0)
		{
			return false;
		}

		if (nready < 0)
		{
			// 不是中断的话，那么报错
			if (errno != EINTR)
				return false;
		}

		if (FD_ISSET(cfd, &writefds))
		{
			// 发送报错，也返回上层处理
			if (-1 == sendS(cfd, writeLen, write_content))
			{
				return false;
			}
			else
			{
				break; // 发送成功，return true;
			}
		}
	}

	return true;
}

void createFile(ofstream &fout)
{
	//create .txt file
	char pidChar[50];
	sprintf(pidChar, "%d", getpid());
	string fileName = string("1652335.") + string(pidChar) + string(".pid.txt");
	fout.open(fileName.c_str(), ios::out | ios::trunc);
}

bool bindAndConnect(int &cfd, sockaddr_in &s_add, unsigned short &portnum, optType *&myOptType)
{
	//nonblock
	cfd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == cfd)
	{
		printf("socket fail ! \r\n");
		return false;
	}

	bzero(&s_add, sizeof(struct sockaddr_in));
	s_add.sin_family = AF_INET;
	s_add.sin_addr.s_addr = inet_addr(myOptType->ip);
	s_add.sin_port = htons(portnum);
	//printf("s_addr = %#x ,port : %#x\r\n", s_add.sin_addr.s_addr, s_add.sin_port);

	if (-1 == connect(cfd, (struct sockaddr *)(&s_add), sizeof(struct sockaddr)))
	{
		printf("connect fail !\r\n");
		return false;
	}
	printf("connect ok !\r\n");
	return true;
}

void clientFork(const int &isFork, const int &linkNum)
{
	/*
 		If isFork then fork linkNum times
		else return
 	*/
	if (!isFork)
		return;
	pid_t pid;
	for (int i = 0; i < linkNum; ++i)
	{
		pid = fork();
		if (pid == 0)
			return;
	}
	exit(0);
}

bool interact(int cfd, char *bufferRead, char *bufferWrite, bool isFork, ofstream &fout)
{
	/*
 		Interact between Server and Client depLim times
	*/
	int depLim = sizeof(readLenArr) / sizeof(int);
	for (int i = 0; i < depLim; ++i)
	{
		if (myRead(cfd, bufferRead, i)){
			if(!myWrite(cfd, bufferWrite, i, isFork, fout, bufferRead)){
				return false;
			}
		}
		else
		{
			char pidChar[50];
			sprintf(pidChar, "%d", getpid());
			string fileName = string("1652335.") + string(pidChar) + string(".pid.txt");
			// remove(fileName.c_str());
			fout.close();
			fout.open(fileName.c_str(), ios::out | ios::trunc);
			return false;
		}
	}

	return true;
}

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

int createSocket(optType &myOptType)
{
	int clientsock, flag;
	if ((clientsock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		return -1;
	}
	if (!myOptType.isBlock) //非阻塞设置
		setSockNonBlock(clientsock);
	return clientsock;
}

int ServerConnect(int sock_fd, struct sockaddr_in echoserver)
{
	/* Establish connection */
	struct sockaddr_in echoserverbak = echoserver;
	if (connect(sock_fd, (struct sockaddr *)&echoserver, sizeof(echoserver)) == 0)
	{
		return 0;
	}
	else if (errno == EINPROGRESS)
	{
		fd_set rfds, wfds;
		/*printf("errno = %d\n", errno);*/
		FD_ZERO(&rfds);
		FD_ZERO(&wfds);
		FD_SET(sock_fd, &rfds);
		FD_SET(sock_fd, &wfds);
		int selres = select(sock_fd + 1, &rfds, &wfds, NULL, NULL);
		switch (selres)
		{
		case -1:
			printf("select error\n");
			break;
		case 0:
			printf("select time out\n");
			break;
		default:
			if (FD_ISSET(sock_fd, &rfds) || FD_ISSET(sock_fd, &wfds))
			{
				int error;
				socklen_t len = sizeof(error);
				if (getsockopt(sock_fd, SOL_SOCKET, SO_ERROR, &error, &len) == -1)
				{
					break;
					//printf("getsockopt return -1.\n");
				}
				else if (error != 0)
				{
					break;
					//printf("getsockopt return errinfo = %d.\n", error);
				}
				else
				{
					printf("connect success\n");
					return 0;
				}
			}
			break;
		}
	}
	return -1;
}

void HandleClientNoForkMode(optType &myOptType) //不做写阻塞的处理
{
	ClientModel myClients[MAXCLIENT];
	fd_set rfds, wfds, rfdsbak, wfdsbak;
	int maxfds = -1;
	struct sockaddr_in echoserver;
	int flags[2000];
	char tmpLarge[100010];
	char buffer[1000];
	char *myTime = new char[100];
	memset(flags,0,sizeof(flags));
	FD_ZERO(&rfds);
	memset(&echoserver, 0, sizeof(echoserver));			  /* Clear struct */
	echoserver.sin_family = AF_INET;					  /* Internet/IP */
	echoserver.sin_addr.s_addr = inet_addr(myOptType.ip); /* IP address */
	echoserver.sin_port = htons(myOptType.port);		  /* server port */
	for (int i = 0; i < myOptType.linkNum; i++)
	{
		int clientSocket = createSocket(myOptType);
		myClients[clientSocket].init(clientSocket, myOptType.isBlock);
		while (ServerConnect(clientSocket, echoserver) < 0)
		{
			continue;
		}
		myClients[clientSocket].setToStatus(SrecvStnoSta);
		FD_SET(clientSocket, &rfds);
		if (clientSocket > maxfds)
			maxfds = clientSocket;
	}
	printf("ok\n");
	rfdsbak = rfds;
	int num = myOptType.linkNum;
	while (num > 0)
	{
		int retS, retR;
		int mypid = (getpid() << 16);
		char *tmpC;
		int tmpInt;
		char myPidStr[30];
		rfdsbak = rfds;
		int myRandNum;
		int SocketRe;
		if (select(maxfds + 1, &rfdsbak, NULL, NULL, NULL) < 0)
			continue;
		for (int i = 0; i <= maxfds; i++)
		{
			if (!FD_ISSET(i, &rfdsbak))
				continue;
			while (1)
			{
				switch (myClients[i].getToStatus())
				{
				case Sidle:
					//printf("1\n");
					myClients[i].removeFile();
					myClients[i].closeC();
					FD_CLR(i, &rfds);
					SocketRe = createSocket(myOptType);
					myClients[SocketRe].init(SocketRe, myOptType.isBlock);
					while (ServerConnect(SocketRe, echoserver) < 0)
						continue;
					myClients[SocketRe].setToStatus(SrecvStnoSta);
					FD_SET(SocketRe, &rfds);
					if (SocketRe > maxfds)
						maxfds = SocketRe;
					break;
				case SrecvStnoSta:
					//printf("2\n");
					myClients[i].setRequiredSize(5);
					retR = myClients[i].recvC(buffer);
					if (retR > 0)
						buffer[retR] = 0;
					myClients[i].setToStatus(SrecvPidSta);
					tmpInt = htonl(1652335);
					tmpC = (char *)(&(tmpInt));
					retS = myClients[i].sendC(4, tmpC);
					if (retR <= 0 || strcmp(buffer, "StuNo") != 0 || retS < 0)
					{
						myClients[i].setToStatus(Sidle);
						continue;
					}
					myClients[i].setClientStuNo("1652335");
					break;
				case SrecvPidSta:
					//printf("3\n");
					myClients[i].setRequiredSize(3);
					retR = myClients[i].recvC(buffer);
					if (retR > 0)
						buffer[retR] = 0;
					myClients[i].setToStatus(SrecvTimeSta);
					tmpInt = htonl(mypid + i);
					tmpC = (char *)(&tmpInt);
					retS = myClients[i].sendC(4, tmpC);
					if (retR <= 0 || strcmp(buffer, "pid") != 0 || retS < 0)
					{
						myClients[i].setToStatus(Sidle);
						continue;
					}
					sprintf(myPidStr, "%d", (mypid + i));
					myClients[i].setClientPid(myPidStr);
					break;
				case SrecvTimeSta:
					myClients[i].setRequiredSize(5);
					retR = myClients[i].recvC(buffer);
					myClients[i].setToStatus(SrecvRandSta);
					getTime(myTime);
					retS = myClients[i].sendC(19, myTime);
					if (retR <= 0 || strcmp(buffer, "TIME") != 0 || retS < 0)
					{
						myClients[i].setToStatus(Sidle);
						continue;
					}
					myClients[i].setClientTime(myTime);
					break;
				case SrecvRandSta:
					myClients[i].setRequiredSize(9);
					retR = myClients[i].recvC(buffer);
					myClients[i].setToStatus(SrecvEndSta);
					myRandNum = (atoi(buffer + 3));
					generateRandomString(tmpLarge, myRandNum);
					retS = myClients[i].sendC(myRandNum, tmpLarge);
					if (retR <= 0 || retS < 0 || !(myRandNum >= 32768 && myRandNum < 100000))
					{
						myClients[i].setToStatus(Sidle);
						continue;
					}
					myClients[i].writeFileHead();
					myClients[i].writeAppend(tmpLarge, myRandNum);
					break;
				case SrecvEndSta:
					myClients[i].setRequiredSize(3);
					retR = myClients[i].recvC(buffer);
					myClients[i].setToStatus(SsuccessSta);
					if (retR > 0)
						buffer[retR] = 0;
					if (retR <= 0 || strcmp(buffer, "end") != 0)
					{
						myClients[i].setToStatus(Sidle);
						continue;
					}
					//myClients[i].closeC();
					flags[i]=1;
					FD_CLR(i, &rfds);
					num--;
					printf("recieve: end\n");
					break;
				default:
					break;
				}
				break;
			}
		}
	}
	for(int i=0;i<2000;i++)
	{
		if(flags[i])
			close(i);
	}
}

int main(int argc, char *argv[])
{
	init_deamon();
	signal(SIGPIPE, SIG_IGN);
	signal(SIGCHLD, SIG_IGN);
	optType *myOptType;
	getOptType(argc, argv, &myOptType, isServer);
	if (!myOptType->isFork)
	{
		HandleClientNoForkMode(*myOptType);
		return 0;
	}
	clientFork(myOptType->isFork, myOptType->linkNum);

	ofstream fout;
	createFile(fout);

	int cfd;
	struct sockaddr_in s_add;
	unsigned short portnum = myOptType->port; //port number

	while (1)
	{
		while (!bindAndConnect(cfd, s_add, portnum, myOptType))
			continue;

		char *bufferRead = new char[readMaxn];
		char *bufferWrite = new char[writeMaxn];
		srand(time(NULL));

		if (!interact(cfd, bufferRead, bufferWrite, myOptType->isFork, fout))
			continue;

		delete[] bufferRead;
		delete[] bufferWrite;

		fout.close();
		close(cfd);

		break;
	}
	return 0;
}
