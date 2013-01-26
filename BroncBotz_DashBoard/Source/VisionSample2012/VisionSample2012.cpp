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

class UDP_Listener;
static void UDP_Listener_Receive_Task(UDP_Listener *instance);

class UDP_Listener
{
	public: 
	//http://www.chiefdelphi.com/forums/showthread.php?p=1024930
	//has info about dashboard to robot using 1130 and robot to dashboard using 1140
	UDP_Listener(UDP_Listener_Interface *client,int portno=1130) : 
		m_task ("UDP_Listener", (FUNCPTR)UDP_Listener_Receive_Task),
		m_Client(client),m_Error(false),m_IsRunning(true)
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
			//if we made it this far we can start the new task
			if (!m_task.Start((INT32)this))
				throw 2;
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
			case 2:
				ErrorMsg="ERROR on starting recieve task";
				break;
			};
			if (ErrorMsg)
				printf("ErrorMsg=%s\n",ErrorMsg);
			m_Error=true;
		}
	}
	~UDP_Listener()
	{
		m_IsRunning=false;
	     close(m_sockfd);
	}
	void TimeChange()
	{
		if (m_Error) return;
	     char buffer[256];
	     bzero(buffer,256);
	     struct sockaddr_in fromAddr; 
	     int fromSize = sizeof(fromAddr);
   		 int n=recvfrom(m_sockfd, buffer, 255, 0, (struct sockaddr *) &fromAddr, &fromSize);

   		 //Note: typically this would fail if the ioctl(m_sockfd, FIONBIO,(int) &mode) mode was set to 1, but since this is in its own thread recvfrom() can use its
   		 //internal blocking (i.e. critical section/mutex) and generally return successful
	     if (n>0)
	     {
	    	 m_Client->ProcessPacket(buffer,n);
	     }
	     else
	 		Wait(0.005);  //avoid busy wait
	}
	
	bool IsRunning() const {return m_IsRunning;}

	private:
	Task m_task;
	UDP_Listener_Interface *m_Client; //delegate packet to client code
	int m_sockfd;
	bool m_Error;
	bool m_IsRunning;
};

//I generally do not recommend using threads at all for robot code, but we have a case that we cannot control.  Currently VxWorks 6.3 has a problem where if UDP packets are sent without
//a listener to empty them the buffers can overflow and flood into legit TCP/IP packets, and cause repeated loss of communication with the driver station.  Therefore as soon as the robot
//powers on this thread needs to start up immediately, and it will continue to ensure all packets are received.  It can also do so using its own internal blocking 
//(i.e. critical section / mutex) intact.  This means that it will not busy wait anytime, and for the rare case of an error we will sleep for 5 ms. 
static void UDP_Listener_Receive_Task(UDP_Listener *instance)
{
	while (instance->IsRunning())
	{
		instance->TimeChange();  //handles sleep implicitly
	}
}

class coodinate_manager : public UDP_Listener_Interface
{
	protected:
		//You could expose these instead to public
		__inline double GetXpos() const {return m_Xpos;}
		__inline double GetYpos() const {return m_Ypos;}
		__inline bool IsUpdated() const {return m_Updated;}
		void ResetUpdate() {m_Updated=false;}
	public:
		coodinate_manager() : m_UDP(this)
		{
			
		}

		//This demonstrates how to interface with the listener for updates without needing critical sections by use of the bool... using bool is an automic operation where there
		//can never be any real danger of a race condition, the worse that can happen is that the update happens on the next iteration, but in our case the thread runs every 33ms
		//while our time change runs about every 10ms, so it is a non-issue
		void TimeChange(double dTime_s)
		{
			if (IsUpdated())
			{
				ResetUpdate();
				//This shows how well the coordinate notifications stay in sync with the UDP_Listener thread updates
				SmartDashboard::PutNumber("Auton X",GetXpos());
				SmartDashboard::PutNumber("Auton Y",GetYpos());
			}
		}
	protected: //from UDP_Listener_Interface
		
		// This is call from within the UDP_Listener task/thread.  The convergence between this thread and client code can happen with need of any blocking by use of an atomic operation 
		//of the bool (think of it as volatile, but simple enough logic to not warrent the need for declaration).  
		virtual void ProcessPacket(char *pkt,size_t pkt_size)
		{
			if (pkt_size==16)
			{
				typedef unsigned long DWORD;
				long *ptr=(long *)pkt;
				DWORD sync=_byteswap_ulong(ptr[0] );
				long XInt=(long)_byteswap_ulong(ptr[1]);
				long YInt=(long)_byteswap_ulong(ptr[2]);
				long checksum=(long)_byteswap_ulong(ptr[3]);
				if (sync==0xabacab)
				{
					if (checksum==XInt+YInt)
					{
						m_Updated=true;
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
		bool m_Updated; //true if we received a packet for this slice of time
};

class VisionSample2012 : public SimpleRobot
{
	coodinate_manager app;  //This (i.e. UDP listenter thread) needs to start immediately!!
	RobotDrive myRobot; // robot drive system
	Joystick stick; // only joystick

public:
	VisionSample2012(void):
		//Note I've put these in a higher channel since I've been testing with a kit that uses 1 and 2 for servos
		myRobot(3, 4),	// these must be initialized in the same order
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

