//
// Server.cpp
//
// Extremely simple, stream server example.
// Works in conjunction with Client.cpp.
//
// The program sets itself up as a server using the TCP
// protoocl. It waits for data from a client, displays
// the incoming data, sends a message back to the client
// and then exits.
//
// Compile and link with wsock32.lib
//
// Pass the port number that the server should bind() to
// on the command line. Any port number not already in use
// can be specified.
//
// Example: Server 2000
//
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <winsock.h>   //socket�{���һݪ��禡�w
#include <time.h>

// Function prototype
void StreamServer(short nPort);
int SendMessagesToClient(char *messages, SOCKET remoteSocket);
bool CheckIsBingo(int randomNumber, int guessedNumber);
int GetRandomNumber();
void PrintErrorAndCloseSocket(char *errorMessage, SOCKET remoteSocket, SOCKET listenSocket);

// Helper macro for displaying errors
#define PRINTERROR(s)   \
	fprintf(stderr, "\n%: %d\n", s, WSAGetLastError())

int RandomNumber;

////////////////////////////////////////////////////////////

void main(int argc, char **argv)
{
	WORD wVersionRequested = MAKEWORD(1, 1);   //��w�����s��
	WSADATA wsaData;          //winsock���@�ظ�Ƶ��c
	int nRet;
	short nPort;              //port number

	//
	// Check for port argument
	//
	if (argc != 2)
	{
		fprintf(stderr, "\nSyntax: server PortNumber\n");
		return;
	}

	nPort = atoi(argv[1]);   //�N��J��port number�s�JnPort (atoi=�N�r���ର���)

	//
	// Initialize WinSock and check version (winsock����l��)
	//
	nRet = WSAStartup(wVersionRequested, &wsaData);
	if (wsaData.wVersion != wVersionRequested)
	{
		fprintf(stderr, "\n Wrong version\n");
		return;
	}

	//
	// Do the stuff a stream server does
	//
	StreamServer(nPort); //�إߤ@��server�A�禡���e�b�U��

	//
	// Release WinSock
	//
	WSACleanup();

	system("pause");
}

////////////////////////////////////////////////////////////

void StreamServer(short nPort)
{
	//
	// Create a TCP/IP stream socket to "listen" with
	//
	SOCKET	listenSocket;    //�ŧi�@��socket�ܼ�

	listenSocket = socket(AF_INET,			// Address family //��Jsocket�һݪ��ѼơA�������]�w���ݧ���
						SOCK_STREAM,		// Socket type
						IPPROTO_TCP);		// Protocol		http://msdn.microsoft.com/en-us/library/windows/desktop/ms740506(v=vs.85).aspx
	if (listenSocket == INVALID_SOCKET)
	{
		PRINTERROR("socket()");
		return;
	}


	//
	// Fill in the address structure
	//
	SOCKADDR_IN saServer;		//socket address���ܼơA�������Ψ�]�w���ݧ���
	/*
	struct sockaddr_in
	{
	short   sin_family;
	u_short sin_port;
	struct  in_addr sin_addr;
	char    sin_zero[8];
	};
	sin_family	�Ψӻ���socket�ҨϥΪ��w�}�Ҧ��A�b�������]�� AF_INET�A���internet domain��socket�C
	sin_port	�ΨӪ��TCP/IP��port umber�A�]�w sin_port ���ݨϥ�htons��Ƨ@�줸�ƧǪ��ʧ@ !!!!!!!!!!!!!!!!
	sin_addr	�O�@��in_addr�����c�ܼơA�ΨӪ��ip��}�C

	AF_INET:    http://topic.csdn.net/t/20040803/10/3236925.html
	�N�O�����a�}�� TCP/IP ��ĳ�ڪ��a�}
	#define   AF_UNIX                   1        //   local   to   host   (pipes,   portals)
	#define   AF_INET                   2        //   internetwork:   UDP,   TCP,   etc.
	#define   AF_IMPLINK                3        //   arpanet   imp   addresses
	#define   AF_PUP                    4        //   pup   protocols:   e.g.   BSP
	.......

	INADDR_ANY          (0.0.0.0)   ��ܥ���i?�w���a�}�F

	htons		http://msdn.microsoft.com/en-us/library/windows/desktop/ms738557(v=vs.85).aspx
	The htons function converts a u_short from host to TCP/IP network byte order (which is big-endian).
	*/
	saServer.sin_family = AF_INET;
	saServer.sin_addr.s_addr = INADDR_ANY;	// Let WinSock supply address
	saServer.sin_port = htons(nPort);		// Use port from command line

	//
	// bind the name to the socket
	//
	int nRet;
	/*
	�NIPv4 socket�w�}���c�s����ҫإߪ�socket
	�A�H����ʥ]��F���������ɡALinux�֤߫K�|�N�o�ӫʥ]�ɦV���s����socket�C
	*/
	nRet = bind(listenSocket,				// Socket   //�إߤ@��socket
				(LPSOCKADDR)&saServer,		// Our address
				sizeof(struct sockaddr));	// Size of address structure		http://msdn.microsoft.com/en-us/library/windows/desktop/ms737550(v=vs.85).aspx
	if (nRet == SOCKET_ERROR)    //�ˬd�O�_�إߦ��\
	{
		PRINTERROR("bind()");
		closesocket(listenSocket);
		return;
	}

	//
	// This isn't normally done or required, but in this 
	// example we're printing out where the server is waiting
	// so that you can connect the example client.
	//
	int nLen;
	nLen = sizeof(SOCKADDR);
	char szBuf[256];

	nRet = gethostname(szBuf, sizeof(szBuf));  //���o�����W��
	if (nRet == SOCKET_ERROR)
	{
		PRINTERROR("gethostname()");
		closesocket(listenSocket);
		return;
	}

	//
	// Show the server name and port number
	//
	printf("\nServer named %s waiting on port %d\n",    //�L�X���\�إߪ��r��
		szBuf, nPort);

	//
	// Set the socket to listen
	//
	printf("\nlisten()");
	nRet = listen(listenSocket,					// Bound socket   //���W���إߪ�socket�i�Jlisen���A(���ݧO�H�ӳs��)
					SOMAXCONN);					// Number of connection request queue		http://msdn.microsoft.com/en-us/library/windows/desktop/ms739168(v=vs.85).aspx
	if (nRet == SOCKET_ERROR)         //�ˬd�O�_�����~
	{
		PRINTERROR("listen()");
		closesocket(listenSocket);
		return;
	}

	//
	// Wait for an incoming request
	//
	SOCKET	remoteSocket;       //�إߤ@�ӷs��socket�A�ΨөM�t�@�ݶǰe��� 
	//(�`�N�G�@�}�l��socket�u�t�d���ݳs�u�A�ݳХt�@��socket�Mclient�s�u�A�t�d�ǰe���)

	printf("\nBlocking at accept()");
	remoteSocket = accept(listenSocket,			// Listening socket  //�N�s��socket����listening socket�A���ݳs�u
						NULL,					// Optional client address
						NULL);
	if (remoteSocket == INVALID_SOCKET)   //�ˬd�O�_�����~
	{
		PRINTERROR("accept()");
		closesocket(listenSocket);
		return;
	}

	//
	// We're connected to a client
	// New socket descriptor returned already
	// has clients address

	//
	// Receive data from the client
	//
	bool isBingo = false, isNewGame = true;
	while (true)
	{
		if (isNewGame)
		{
			nRet = SendMessagesToClient("�w��Ӫ��q�Ʀr�I\n�п�J 1��9 ���@��Ʀr�G\n", remoteSocket);
			if (nRet == SOCKET_ERROR)           // �ˬd�O�_�����~
			{
				PrintErrorAndCloseSocket("send()", remoteSocket, listenSocket);
				return;
			}

			RandomNumber = GetRandomNumber();
			printf("\n�o���C�������׬O�G%d�C\n", RandomNumber);
			isNewGame = false;
		}

		memset(szBuf, 0, sizeof(szBuf));			//�Nbuffer(szBuf)�̪��ȥ����]��0 (��l��)
		nRet = recv(remoteSocket,					// Connected client   //������誺�T���A�ç�T���s�Jbuffer��
					szBuf,							// Receive buffer
					sizeof(szBuf),					// Lenght of buffer
					0);								// Flags
		if (nRet == INVALID_SOCKET)					//�ˬd�O�_�����~
		{
			PrintErrorAndCloseSocket("recv()", remoteSocket, listenSocket);
			return;
		}

		//
		// Display received data
		//
		printf("\nData received: %s", szBuf);      //�Nbuffer�����e�L�X��

		//
		// Send data back to the client
		//
		if (isBingo == false && strlen(szBuf) != 1)
		{
			nRet = SendMessagesToClient("�Э��s��J 1��9 ���@��Ʀr�G\n", remoteSocket);
			if (nRet == SOCKET_ERROR)           // �ˬd�O�_�����~
			{
				PrintErrorAndCloseSocket("send()", remoteSocket, listenSocket);
				return;
			}
			continue;
		}

		if (isBingo == true)
		{
			if (szBuf[0] == 'Y' || szBuf[0] == 'y')
			{
				isBingo = false;
				isNewGame = true;
				continue;
			}
			break;
		}

		if (isBingo = CheckIsBingo(RandomNumber, atoi(szBuf)))
		{
			nRet = SendMessagesToClient("Bingo!\n�O�_�n�~�򪱡H\t[Y]es, [N]o\n", remoteSocket);
			if (nRet == SOCKET_ERROR)           // �ˬd�O�_�����~
			{
				PrintErrorAndCloseSocket("send()", remoteSocket, listenSocket);
				return;
			}
		}
		else
		{
			nRet = SendMessagesToClient("Fail!\n", remoteSocket);
			if (nRet == SOCKET_ERROR)           // �ˬd�O�_�����~
			{
				PrintErrorAndCloseSocket("send()", remoteSocket, listenSocket);
				return;
			}
		}
	}

	nRet = SendMessagesToClient("Bye! Bye!", remoteSocket);
	if (nRet == SOCKET_ERROR)           // �ˬd�O�_�����~
	{
		PrintErrorAndCloseSocket("send()", remoteSocket, listenSocket);
		return;
	}

	//
	// Close BOTH sockets before exiting
	//
	closesocket(remoteSocket);    //����remoteSocket
	closesocket(listenSocket);    //����listenSocket
	return;
}

int SendMessagesToClient(char *messages, SOCKET remoteSocket)
{
	return send(remoteSocket,				// Connected socket  //�o�ebuffer�����T��
				messages,					// Data buffer
				strlen(messages),			// Lenght of data
				0);
}

bool CheckIsBingo(int randomNumber, int guessedNumber)
{
	return guessedNumber == randomNumber;
}

int GetRandomNumber()
{
	srand(time(NULL));
	return rand() % 9 + 1;
}

void PrintErrorAndCloseSocket(char *errorMessage, SOCKET remoteSocket, SOCKET listenSocket)
{
	PRINTERROR(errorMessage);
	closesocket(remoteSocket);
	closesocket(listenSocket);
}