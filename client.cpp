#include "client.h"

using namespace std;
const int writeMaxn = 100000;
const int readMaxn = 50;
const bool isServer = false;

const int readLenArr[] = {5, 3, 5, 9, 3};
const int readCmpLen[] = {5, 3, 5, 3, 3};
const char * readCmpContent[] = {"StuNo", "pid", "TIME", "str", "end"};

int myRead(int cfd, char* & bufferRead, int depth)
{
        static int recbytes;
        static long long readTmp = 0;
        static long long readCnt = 0;
	int readLen = readLenArr[depth];
	memset(bufferRead, 0, sizeof(bufferRead));
        if(0 >= (recbytes = read(cfd,bufferRead,readLenArr[depth])))
        {
                if(recbytes == 0)
                        cout << "读结束" << endl;
                else if(errno == EINTR)
                        cerr << "中断错误" << endl;
                else
                        cerr << "其他连接错误" << endl;
		return 0;
        }
	if(recbytes != readLen || strncmp(bufferRead, readCmpContent[depth], readCmpLen[depth]))
		return 0;
	if(depth == 3)
	{
		for(int i = 3; i < 8; ++i)
			if(!isdigit(bufferRead[i]))
				return 0;
		int tmp = atoi(bufferRead+3);
		cout << "随机数:" << tmp << endl;
		if(tmp < 32768 || tmp > 99999)
			return 0;
	}			
	return 1;
}

void getTime(char* & bufferWrite)
{
	//Get formatted time into bufferWrite
	time_t now = time(NULL);
	tm t;
	localtime_r(&now, &t);
	strftime(bufferWrite, 20, "%Y-%m-%d %H:%M:%S", &t);
}

void generateRandomString(char * bufferWrite, const int & len)
{
	for(int i = 0; i < len; ++i)
		bufferWrite[i] = rand() % 256;
}

const int writeLenArr[] = {4, 4, 19, -1};
int myWrite(int cfd, char* & bufferWrite, int depth, bool isFork, ofstream & fout, char * & bufferRead)
{
	static int writebytes;
        static long long writeTmp = 0;
        static long long writeCnt = 0;
	int writeLen = writeLenArr[depth];
	static int Stu = 1652335;
	int tmp;
	static int pid;
	char* write_content;
	memset(bufferWrite, 0, sizeof(bufferWrite));
	switch(depth)
	{
		case 0:
			fout << Stu << "\n";
			tmp = htonl(Stu);
			write_content = (char*)&tmp;
			writeLen = sizeof(Stu);
			break;
		case 1:
			pid = int(isFork ? getpid() : ((getpid() << 16) + cfd));
			fout << pid << "\n"; 
			pid = htonl(pid);
			write_content = (char*)&pid;
			writeLen = sizeof(pid);
			break;
		case 2:
			getTime(bufferWrite);
			write_content = bufferWrite;
			fout << bufferWrite << "\n";
			break;
		case 3:
			generateRandomString(bufferWrite, atoi(bufferRead+3));
			fout.write(bufferWrite, atoi(bufferRead+3));
			write_content = bufferWrite;
			writeLen = atoi(bufferRead + 3);
			break;
		default:
			//cerr << "Switch error" << endl;
			break;
	}
	
		
        if(-1 == (writebytes = write(cfd,write_content,writeLen)))
        {       
                // cout << "errno:" << errno << "\n";
		printf("write fail!\r\n");
		return 0;
        }
	return 1;
}

void createFile(ofstream & fout)
{
	//create .txt file
	char pidChar[50];
	sprintf(pidChar, "%d", getpid());
	string fileName = string("1652335.") + string(pidChar) + string(".pid.txt");
	fout.open(fileName.c_str(), ios::out | ios::trunc);
}

void bindAndConnect(int & cfd, sockaddr_in & s_add, unsigned short & portnum,  optType * & myOptType)
{
	//nonblock
	cfd = socket(AF_INET, SOCK_STREAM, 0);
        if(-1 == cfd)
        {
            printf("socket fail ! \r\n");
            exit(EXIT_FAILURE);
        }

        bzero(&s_add,sizeof(struct sockaddr_in));
        s_add.sin_family=AF_INET;
        s_add.sin_addr.s_addr= inet_addr(myOptType->ip);
        s_add.sin_port=htons(portnum);
        printf("s_addr = %#x ,port : %#x\r\n",s_add.sin_addr.s_addr,s_add.sin_port);

        if(-1 == connect(cfd,(struct sockaddr *)(&s_add), sizeof(struct sockaddr)))
        {
            printf("connect fail !\r\n");
            exit(EXIT_FAILURE);
        }
        printf("connect ok !\r\n");	
}

void clientFork(const int & isFork, const int & linkNum)
{
	/*
 		If isFork then fork linkNum times
		else return
 	*/
	if(!isFork)
		return;
	pid_t pid;
	for(int i = 0; i < linkNum; ++i)
	{
		pid = fork();
		if(pid == 0)
			return;
	}
	exit(0);		
}


void interact(int cfd, char * bufferRead, char * bufferWrite, bool isFork, ofstream& fout)
{
	/*
 		Interact between Server and Client depLim times
	*/
	int depLim = sizeof(readLenArr) / sizeof(int);
	for(int i = 0; i < depLim; ++i)
	{
		if(myRead(cfd, bufferRead, i))
			myWrite(cfd, bufferWrite, i, isFork, fout, bufferRead);
		else
			break;	
	}
}

int main(int argc, char* argv[])
{
	init_deamon();
	
	optType *myOptType;
	getOptType(argc, argv, &myOptType, isServer);

	clientFork(myOptType->isFork, myOptType->linkNum);

	ofstream fout;
	createFile(fout);

	int cfd;
	struct sockaddr_in s_add;
	unsigned short portnum = myOptType->port;  //port number
	
	bindAndConnect(cfd, s_add, portnum, myOptType);
	
	char* bufferRead = new char[readMaxn];
        char* bufferWrite = new char[writeMaxn];
	srand(time(NULL));
	
       	interact(cfd, bufferRead, bufferWrite, myOptType->isFork, fout); 

        delete[] bufferRead;
        delete[] bufferWrite;

	fout.close();
	close(cfd);
	return 0;
}
