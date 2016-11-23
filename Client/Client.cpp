//
// Client.cpp
//
// Extremely simple, stream client example.
// Works in conjunction with Server.cpp.
//
// The program attempts to connect to the server and port
// specified on the command line. The Server program prints
// the needed information when it is started. Once connected,
// the program sends data to the server, waits for a response
// and then exits.
//
// Compile and link with wsock32.lib
//
// Pass the server name and port number on the command line.
//
// Example: Client MyMachineName 2000
//
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <winsock.h>
#include <iostream>

using namespace std;

// Function prototype
void StreamClient(char *szServer, short nPort);

// Helper macro for displaying errors
#define PRINTERROR(s)	\
	fprintf(stderr, "\n%: %d\n", s, WSAGetLastError())

////////////////////////////////////////////////////////////

void main(int argc, char **argv)
{
	WORD wVersionRequested = MAKEWORD(1, 1);
	WSADATA wsaData;
	int nRet;
	short nPort;

	//
	// Check for the host and port arguments
	//
	if (argc != 3)
	{
		fprintf(stderr, "\nSyntax: client ServerName PortNumber\n");
		return;
	}

	nPort = atoi(argv[2]);

	//
	// Initialize WinSock and check the version
	//
	nRet = WSAStartup(wVersionRequested, &wsaData);
	if (wsaData.wVersion != wVersionRequested)
	{
		fprintf(stderr, "\n Wrong version\n");
		return;
	}

	//
	// Go do the stuff a stream client does
	//
	StreamClient(argv[1], nPort);  //argv[1]����J���Ĥ@�ӰѼ�(IP address)�AnPort=port number

	//
	// Release WinSock
	//
	WSACleanup();

	system("pause");
}

////////////////////////////////////////////////////////////

void StreamClient(char *szServer, short nPort)
{
	printf("\nStream Client connecting to server: %s on port: %d",   //�L�X�s�������T��
		szServer, nPort);

	//
	// Find the server
	//
	LPHOSTENT lpHostEntry;

	lpHostEntry = gethostbyname(szServer);   //�Q��IP address�M��server
	if (lpHostEntry == NULL)    //�ˬd�O�_�����~
	{
		PRINTERROR("gethostbyname()");
		return;
	}

	//
	// Create a TCP/IP stream socket
	//
	SOCKET	theSocket;                      //�إߤ@��socket

	theSocket = socket(AF_INET,				// Address family   //�ѼƤ��ݧ���
		SOCK_STREAM,			// Socket type
		IPPROTO_TCP);		// Protocol
	if (theSocket == INVALID_SOCKET)    //�ˬd�O�_�����~
	{
		PRINTERROR("socket()");
		return;
	}

	//
	// Fill in the address structure
	//
	SOCKADDR_IN saServer;   //socket address���ܼ�

	saServer.sin_family = AF_INET;           //�ѼƤ��ݧ���
	saServer.sin_addr = *((LPIN_ADDR)*lpHostEntry->h_addr_list);
	// ^ Server's address
	saServer.sin_port = htons(nPort);	// Port number from command line

	//
	// connect to the server
	//
	int nRet;

	nRet = connect(theSocket,				// Socket   //�Mserver���s��
		(LPSOCKADDR)&saServer,	// Server address
		sizeof(struct sockaddr));// Length of server address structure
	if (nRet == SOCKET_ERROR)      //�ˬd�O�_�����~
	{
		PRINTERROR("socket()");
		closesocket(theSocket);
		return;
	}

	//
	// Send data to the server
	//
	char szBuf[256];
	printf("\n");
	while (true)
	{
		//
		// Wait for a reply
		//
		memset(szBuf, 0, sizeof(szBuf));
		nRet = recv(theSocket,				// Connected socket  //���ݱ����T���A�ç⦬�����T���s�Jbuffer
			szBuf,					// Receive buffer
			sizeof(szBuf),			// Size of receive buffer
			0);						// Flags
		if (nRet == SOCKET_ERROR)           // �ˬd�O�_�����~
		{
			PRINTERROR("recv()");
			closesocket(theSocket);
			return;
		}

		//
		// Display the received data
		//
		printf("\nData received: %s\n", szBuf);	// �Nbuffer�����T���L�X

		if (strcmp(szBuf, "Bye! Bye!") == 0)
		{
			break;
		}

		memset(szBuf, 0, sizeof(szBuf));
		cin >> szBuf;
		nRet = send(theSocket,				// Connected socket   //�o�ebuffer�����T��
			szBuf,					// Data buffer
			strlen(szBuf),			// Length of data
			0);						// Flags
		if (nRet == SOCKET_ERROR)           // �ˬd�O�_�����~
		{
			PRINTERROR("send()");
			closesocket(theSocket);
			return;
		}
	}

	closesocket(theSocket);    // ����csocket
	return;
}