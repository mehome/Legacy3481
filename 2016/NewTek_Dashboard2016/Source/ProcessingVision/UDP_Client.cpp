#include "stdafx.h"
#include <stdlib.h>     /* for exit() */
#include <stdio.h>      /* for printf(), fprintf() */
#include <string>     /* for string functions  */
#include <iostream>
#include <winsock2.h>    /* for socket(),... */

#include "ProcessingVision.h"

#pragma comment( lib, "Ws2_32" )


class UDP_Client : public UDP_Client_Interface
{
public:
	UDP_Client(char *servIP= "10.28.1.2",unsigned short echoServPort=1130) : m_Error(false)
	{
		WORD wVersionRequested;          // Version of Winsock to load 
		WSADATA wsaData;                 // Winsock implementation details 

		try
		{
			/* Winsock DLL and library initialization  */
			wVersionRequested = MAKEWORD(2, 0);   /* Request Winsock v2.0 */
			if (WSAStartup(wVersionRequested, &wsaData) != 0) /* Load Winsock DLL */
				throw 0;
			/* 1. Create a socket. */
			/* Create a best-effort datagram socket using UDP */
			if ((m_Sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
				throw 1;

			/* Construct the server address structure */
			memset(&m_DestServer, 0, sizeof(m_DestServer));    /* Zero out structure */
			m_DestServer.sin_family = AF_INET;                 /* Internet address family */
			m_DestServer.sin_addr.s_addr = inet_addr(servIP);  /* Server IP address */
			m_DestServer.sin_port   = htons(echoServPort);     /* Server port */

		}
		catch (int ErrorCode)
		{
			const char *ErrorMsg=NULL;
			switch (ErrorCode)
			{
			case 0:
				ErrorMsg="WSAStartup() failed";
				break;
			case 1:
				ErrorMsg="socket() failed";
				break;
			};
			if (ErrorMsg)
				printf("ErrorMsg=%s\n",ErrorMsg);
			m_Error=true;

		}
	}

	void operator() (double X,double Y)
	{
		if (m_Error) return;
		// 2. Send (sendto()) the message to the server. 
		// Send the buffer
		char queryBuffer[16];
		long *buffer=(long *)queryBuffer;
		buffer[0]=0xabacab; //a sync 
		buffer[1]= (long)(X * 10000000.0);
		buffer[2]= (long)(Y * 10000000.0);
		buffer[3]=buffer[1]+buffer[2];
		int cbSent_ = sendto(m_Sock, queryBuffer, 16, 0, (struct sockaddr *)	&m_DestServer, sizeof(m_DestServer));
		if (cbSent_ < 0)
			printf("sendto failed with error code %d\n",cbSent_);
	}

	~UDP_Client()
	{
		// 5. Close the socket. 
		// Winsock requires a special function for sockets 
		closesocket(m_Sock);    // Close client socket 
		WSACleanup();  // Cleanup Winsock 
	}
private:
	int m_Sock;
	struct sockaddr_in m_DestServer; // Echo server address 

	bool m_Error;
};


UDP_Client_Interface *UDP_Client_Interface::GetNewInstance(char *servIP,unsigned short echoServPort)
{
	return new UDP_Client(servIP,echoServPort);
}
