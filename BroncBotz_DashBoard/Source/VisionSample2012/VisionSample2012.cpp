#include "WPILib.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
class UDP_Listener_Interface
{
	public:
		virtual ~UDP_Listener_Interface() {}  
		virtual void ProcessPacket(char *pkt,size_t pkt_size)=0;
};


unsigned long _byteswap_ulong(unsigned long i)
{
    unsigned int j;
    j =  (i << 24);
    j += (i <<  8) & 0x00FF0000;
    j += (i >>  8) & 0x0000FF00;
    j += (i >> 24);
    return j;
}

class UDP_Listener
{
	public: 
	//http://www.chiefdelphi.com/forums/showthread.php?p=1024930
	//has info about dashboard to robot using 1130 and robot to dashboard using 1140
	UDP_Listener(UDP_Listener_Interface *client,int portno=1130) : m_Client(client),m_Error(false)
	{
		try 
		{
			 //socklen_t clilen;
			int clilen;
			 struct sockaddr_in serv_addr, cli_addr;
			 m_sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			 if (m_sockfd < 0)	
				 throw 0;
			 bzero((char *) &serv_addr, sizeof(serv_addr));
			 serv_addr.sin_family = AF_INET;
			 serv_addr.sin_addr.s_addr = INADDR_ANY;
			 serv_addr.sin_port = htons(portno);
			 if (bind(m_sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
				 throw 1;
			 listen(m_sockfd,5);
			 clilen = sizeof(cli_addr);
		}
		catch (int ErrorCode)
		{
			const char *ErrorMsg=NULL;
			switch (ErrorCode)
			{
			case 0:
				ErrorMsg="ERROR opening socket";
				break;
			case 1:
				ErrorMsg="ERROR on binding";
				break;
			};
			if (ErrorMsg)
				printf("ErrorMsg=%s\n",ErrorMsg);
			m_Error=true;
		}
	}
	~UDP_Listener()
	{
	     close(m_sockfd);
	}
	void TimeChange(double dTime_s)
	{
		if (m_Error) return;
	     char buffer[256];
	     bzero(buffer,256);
	     struct sockaddr_in fromAddr; 
	     int fromSize = sizeof(fromAddr);
   		 int n=recvfrom(m_sockfd, buffer, 255, 0, (struct sockaddr *) &fromAddr, &fromSize);
	     
	     if (n>0)
	     {
	    	 m_Client->ProcessPacket(buffer,n);
	     }
	}
	private:
	UDP_Listener_Interface *m_Client; //delegate packet to client code
	int m_sockfd;
	bool m_Error;
};

class coodinate_manager : public UDP_Listener_Interface
{
	public:
		coodinate_manager() : m_UDP(this)
		{
			
		}
		void TimeChange(double dTime_s)
		{
			m_UDP.TimeChange(dTime_s);
			//TODO smart dashboard display and servo manipulation
		}
	protected: //from UDP_Listener_Interface
		virtual void ProcessPacket(char *pkt,size_t pkt_size)
		{
			if (pkt_size==16)
			{
				typedef unsigned long DWORD;
				DWORD *ptr=(DWORD *)pkt;
				DWORD sync=_byteswap_ulong(ptr[0] );
				DWORD XInt=_byteswap_ulong(ptr[1]);
				DWORD YInt=_byteswap_ulong(ptr[2]);
				DWORD checksum=_byteswap_ulong(ptr[3]);
				if (sync==0xabacab)
				{
					if (checksum==XInt+YInt)
					{
						m_Xpos=(double)XInt / 10000000.0;
						m_Ypos=(double)YInt / 10000000.0;
						//printf("New coordinates %f , %f\n",m_Xpos,m_Ypos);
						SmartDashboard::PutNumber("X Position",m_Xpos);
						SmartDashboard::PutNumber("Y Position",m_Ypos);
					}
					else
						printf("%d + %d != %d\n",(int)XInt,(int)YInt,(int)checksum);
				}
			}
			else
				printf("warning packet size=%d\n",pkt_size);
		}
	private:
	 UDP_Listener m_UDP;
	 double m_Xpos,m_Ypos;
};

class VisionSample2012 : public SimpleRobot
{
	RobotDrive myRobot; // robot drive system
	Joystick stick; // only joystick
	AxisCamera *camera;

public:
	VisionSample2012(void):
		myRobot(1, 2),	// these must be initialized in the same order
		stick(1)		// as they are declared above.
	{
		myRobot.SetExpiration(0.1);
		myRobot.SetSafetyEnabled(false);
	}

	/**
	 * Drive left & right motors for 2 seconds then stop
	 */
	void Autonomous(void)
	{
		coodinate_manager app;
		double tm = GetTime();
		while (IsAutonomous() && !IsDisabled())
		{
			double time=GetTime() - tm;
			tm=GetTime();
			//Framework::Base::DebugOutput("%f\n",time),
			//I'll keep this around as a synthetic time option for debug purposes
			//time=0.020;
			app.TimeChange(time);
			//using this from test runs from robo wranglers code
			Wait(0.010);				
		}

	}

	/**
	 * Runs the motors with arcade steering. 
	 */
	void OperatorControl(void)
	{
		myRobot.SetSafetyEnabled(true);
		while (IsOperatorControl())
		{
			Wait(0.005);				// wait for a motor update time
		}
	}
};

START_ROBOT_CLASS(VisionSample2012);

