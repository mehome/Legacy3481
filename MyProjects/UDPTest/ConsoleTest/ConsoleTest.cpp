
#include "stdafx.h"
#include <math.h>
//#include <boost/smart_ptr.hpp>

//using namespace std;

//http://forums.devshed.com/c-programming-42/winsock2-udp-881514.html

/*************************************************************************/
/* udp_client.c                                                          */
/* David C. Wise, 2004                                                   */
/* Email: dwise1@aol.com                                                 */
/* This program is intended for training purposes only.  It is not nor   */
/*      was it ever intended for commercial or production use.           */
/* I disclaim any and all responsibility should you decide to use it.    */
/*************************************************************************/
/* Simple UDP Echo Client Example                                        */
/* Sends a single message to the server                                  */
/* Obtains the message from the command-line arguments                   */
/*                                                                       */
/* Usage: udp_client <Server IP> <Echo Message> [<Echo Port>]            */
/*      Server IP must be in dotted-decimal format                       */
/*              does not do DNS                                          */
/*      Echo Message is the message to be sent.  Enclose in quotes if    */
/*              it consists of multiple words                            */
/*      Echo Port is optional; will default to 7 if not included         */
/*                                                                       */
/* Basic Sequence of Operations:                                         */
/*      1. Create a socket.                                              */
/*      2. Send (sendto()) the message to the server.                    */
/*      3. Receive (recvfrom()) the server's response.                   */
/*      4. Not done. (repeate send and receive)                          */
/*      5. Close the socket.                                             */
/*************************************************************************/
/* Based on and modified from code examples from                         */
/*      "The Pocket Guide to TCP/IP Sockets: C Version"                  */
/*                 by Michael J. Donahoo and Kenneth L. Calvert:         */
/*         UDPEchoClient.c                                               */
/*         DieWithError.c                                                */
/*                                                                       */
/* The original UNIX source code is freely available from their web site */
/*      at http://cs.baylor.edu/~donahoo/PocketSocket/textcode.html      */
/*      and the Winsock version of the code at                           */
/*          http://cs.baylor.edu/~donahoo/PocketSocket/winsock.html      */
/*                                                                       */
/* Please read the authors' disclaimer on their web site at              */
/*    http://cs.ecs.baylor.edu/~donahoo/practical/CSockets/textcode.html */
/* In particular note that "the authors and the Publisher DISCLAIM ALL   */
/*      EXPRESS AND IMPLIED WARRANTIES, including warranties of          */
/*      merchantability and fitness for any particular purpose.          */
/* Your use or reliance upon any sample code or other information in     */
/*      [their] book will be at your own risk.                           */
/* No one should use any sample code (or illustrations) from [their]     */
/*      book in any software application without first obtaining         */
/*      competent legal advice."                                         */
/*                                                                       */
/*************************************************************************/
/* This program will compile under Windows or UNIX/Linux depending on    */
/*      the setting of the WINSOCK_EXAMPLE define                        */
/* The Winsock version will require the Winsock library to be linked in  */
/*      (exact method and library name are compiler-dependent)           */
/*************************************************************************/

/* #define for Windows; #undef for UNIX/Linux */
#define WINSOCK_EXAMPLE

#include <stdlib.h>     /* for exit() */
#include <stdio.h>      /* for printf(), fprintf() */
#include <string>     /* for string functions  */
#include <iostream>
#ifdef WINSOCK_EXAMPLE
#include <winsock2.h>    /* for socket(),... */
#else
#include <unistd.h>     /* for close() */
#include <sys/socket.h> /* for socket(),... */
#include <netinet/in.h> /* for socket(),... */
#include <arpa/inet.h>  /* for inet_addr() */
#endif

#pragma comment( lib, "Ws2_32" )


class UDP_Client
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
			DWORD *buffer=(DWORD *)queryBuffer;
			buffer[0]=0xabacab; //a sync 
			buffer[1]= (DWORD)(X * 10000000.0);
			buffer[2]= (DWORD)(Y * 10000000.0);
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


#define ECHOMAX 255     /* Longest string to echo */
#define ECHO_PORT   7    /* Default standard port number for echo   */

void ReportError(std::string errorMessage);   /* Error handling function (no exit) */
void DieWithError(std::string errorMessage);  /* Fatal Error handling function     */

/********************************************************************/
/* main -- like opinions, every program has one.                    */
/********************************************************************/
int main(int argc, char *argv[])
{
	{
		UDP_Client test;
		test(1.0,1.1);
		Sleep(1000);
	}
	return 0;
    int sock;                        /* Socket descriptor */
    struct sockaddr_in echoServAddr; /* Echo server address */
    struct sockaddr_in fromAddr;     /* Source address of echo */
    unsigned short echoServPort;     /* Echo server port */
    int fromSize;           /* In-out of address size for recvfrom() */
    char *servIP;                    /* IP address of server */
    char *echoString;                /* String to send to echo server */
    char echoBuffer[ECHOMAX];        /* Buffer for echo string */
    int echoStringLen;               /* Length of string to echo */
    int respStringLen;               /* Length of response string */
#ifdef WINSOCK_EXAMPLE
    WORD wVersionRequested;          /* Version of Winsock to load */
    WSADATA wsaData;                 /* Winsock implementation details */
#endif

	#if 0
    /* First process the command-line arguments. */
    if ((argc < 3) || (argc > 4))
    {
        fprintf(stderr,"Usage: %s <Server IP> <Echo Message> [<Echo Port>]\n", argv[0]);
        exit(1);
    }

    servIP = argv[1];           /* first arg: server IP address (dotted quad)*/
    echoString = argv[2];       /* second arg: string to echo */
	#else
	servIP = "10.28.1.2";
	echoString = "Test";
	#endif

    if ((echoStringLen = strlen(echoString) + 1) > ECHOMAX)  /* Check input length */
        DieWithError("Echo word too long");

	#if 0
	if (argc == 4)
        echoServPort = atoi(argv[3]);  /* Use given port, if any */
    else
        echoServPort = ECHO_PORT;  /* otherwise, use the default port number */
	#else
		echoServPort = 1130;
	#endif

#ifdef WINSOCK_EXAMPLE
    /* Winsock DLL and library initialization  */
    wVersionRequested = MAKEWORD(2, 0);   /* Request Winsock v2.0 */
    if (WSAStartup(wVersionRequested, &wsaData) != 0) /* Load Winsock DLL */
    {
        fprintf(stderr,"WSAStartup() failed");
        exit(1);
    }
#endif

/* 1. Create a socket. */
    /* Create a best-effort datagram socket using UDP */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        DieWithError("socket() failed");

	//example IP="208.68.90.171" port=27015

    /* Construct the server address structure */
    memset(&echoServAddr, 0, sizeof(echoServAddr));    /* Zero out structure */
    echoServAddr.sin_family = AF_INET;                 /* Internet address family */
    echoServAddr.sin_addr.s_addr = inet_addr(servIP);  /* Server IP address */
    echoServAddr.sin_port   = htons(echoServPort);     /* Server port */

/* 2. Send (sendto()) the message to the server. */
    /* Send the string, including the null terminator, to the server */
    //char queryBuffer[] = "\xFF\xFF\xFF\xFF\x69";
	char queryBuffer[] = "Test";
    if (sendto(sock, queryBuffer, strlen(queryBuffer), 0, (struct sockaddr *)
               &echoServAddr, sizeof(echoServAddr)) != echoStringLen)
        DieWithError("sendto() sent a different number of bytes than expected");

#if 0
/* 3. Receive (recvfrom()) the server's response. */
    fromSize = sizeof(fromAddr);
    if ((respStringLen = recvfrom(sock, echoBuffer, ECHOMAX, 0, (struct sockaddr *) &fromAddr,
                 &fromSize)) != echoStringLen)
        DieWithError("recvfrom() failed");

    /* Verify that the response came from the server and not from somewhere else */
    if (echoServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr)
    {
        fprintf(stderr,"Error: received a packet from unknown source.\n");
        exit(1);
    }

    /* Process the received message */
    if (echoBuffer[respStringLen-1])  /* Do not printf unless it is terminated */
        printf("Received an unterminated string\n");
    else
        printf("Received: %s\n", echoBuffer);    /* Print the echoed arg */
#endif
/* 4. Repeat the send and receive as required. (each of which will be seen by the server as a separate communication) */
/* Step 4 does not happen in the case of a UDP echo client */


	Sleep(1000);
/* 5. Close the socket. */
#ifdef WINSOCK_EXAMPLE
    /* Winsock requires a special function for sockets */
    closesocket(sock);    /* Close client socket */

    WSACleanup();  /* Cleanup Winsock */
#else
    close(sock);    /* Close client socket */
#endif

    return 0;
}



/********************************************************/
/* DieWithError                                         */
/*    Separate function for handling errors             */
/*    Reports an error and then terminates the program  */
/********************************************************/
void DieWithError(std::string errorMessage)
{
    ReportError(errorMessage);
    exit(1);
}

/**************************************************************************/
/* ReportError                                                            */
/*    Displays a message that reports the error                           */
/*    Encapsulates the difference between UNIX and Winsock error handling */
/* Winsock Note:                                                          */
/*    WSAGetLastError() only returns the error code number without        */
/*    explaining what it means.  A list of the Winsock error codes        */
/*    is available from various sources, including Microsoft's            */
/*    on-line developer's network library at                              */
/*  http://msdn.microsoft.com/library/default.asp?url=/library/en-us/winsock/winsock/windows_sockets_error_codes_2.asp */
/**************************************************************************/
void ReportError(std::string errorMessage)
{
#ifdef WINSOCK_EXAMPLE
    std::cout << "error: " << errorMessage << WSAGetLastError() << std::endl;
#else
    perror(errorMessage);
#endif
}
