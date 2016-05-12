#include "WPILib.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <ioLib.h>
#include <assert.h>
//We'll declare it locally to avoid needing to include this environment with misc.h
#define COMMON_API

#include "UDP_Listener.h"

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
	
	//TODO provide list in clients in constructor
	UDP_Listener(UDP_Listener_Interface *client,int portno=1130) :
		m_task ("UDP_Listener", (FUNCPTR)UDP_Listener_Receive_Task),
		m_Error(false),m_IsRunning(true)
	{
		if (client)
			m_Client.push_back(client);
		try 
		{
			 //socklen_t clilen;
			int clilen;
			 struct sockaddr_in serv_addr, cli_addr;
			 m_sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if (m_sockfd < 0)	
				 throw 0;

			//Shouldn't need this since we have our own task
			#if 0
			unsigned long mode = 1;
			int test=ioctl(m_sockfd, FIONBIO,(int) &mode);
			if (test!=0)
				printf("Warning unable to set socket to non-blocking");
			#endif

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
		//Note: even if we have no clients we still want to get the packets out of the network so they do not wreck havok on driver station
		if (m_Error) return;
	     char buffer[256];
	     bzero(buffer,256);
	     struct sockaddr_in fromAddr; 
	     int fromSize = sizeof(fromAddr);
	     //By default this will wait for an event signal of receiving a packet, so in the case of success a sleep occurred within this call
   		 int n=recvfrom(m_sockfd, buffer, 255, 0, (struct sockaddr *) &fromAddr, &fromSize);
	     
	     if (n>0)
	     {
	    	 for (size_t i=0;i<m_Client.size();i++)
	    		 m_Client[i]->ProcessPacket(buffer,n);
	     }
	     else
	 		Wait(0.005);  //avoid busy wait
	}
	bool IsRunning() const {return m_IsRunning;}
	
	private:
	Task m_task;

	//Note: this is only populated during construction to avoid needing critical sections
	std::vector <UDP_Listener_Interface *> m_Client; //delegate packet to client code
	int m_sockfd;
	bool m_Error;
	bool m_IsRunning;
};

static void UDP_Listener_Receive_Task(UDP_Listener *instance)
{
	while (instance->IsRunning())
	{
		instance->TimeChange();  //handles sleep implicitly
	}
}



//TODO move UDP listener out as we'll have multiple instances of this to be submitted within the constructor
class UDP_coodinate_manager : public coodinate_manager
{
	public:
		UDP_coodinate_manager() : m_UDP(this)
		{
			
		}
	protected: //from UDP_Listener_Interface
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
};

class SmartDashboard_coordinate_manager : public coodinate_manager_Interface
{
	private:
		mutable double m_Xpos,m_Ypos;
	protected:
		virtual void TimeChange(double dTime_s) {}

		//Note: We just get it... the put is happening from our SmartCppDashboard and will automatically be reflected in the java client
		virtual double GetXpos() const
		{
			return m_Xpos;
		}
		virtual double GetYpos() const
		{
			return m_Ypos;
		}
		//use simple flood control here
		virtual bool IsUpdated() const
		{
			double lastXpos=m_Xpos,lastYpos=m_Ypos;
			m_Xpos=SmartDashboard::GetNumber("X Position");
			m_Ypos=SmartDashboard::GetNumber("Y Position");
			return ((m_Xpos!=lastXpos)||(m_Ypos!=lastYpos));
		}

};

coodinate_manager_Interface *coodinate_manager_Interface::CreateInstance(ListeningPlatform listeningPlatform)
{
	coodinate_manager_Interface *ret=NULL;

	switch(listeningPlatform)
	{
	case eListeningPlatform_UDP:
		ret=new UDP_coodinate_manager;
		break;
	case eListeningPlatform_TCPIP:
		ret=new SmartDashboard_coordinate_manager;
		//ensure the variables are initialized before calling get
		SmartDashboard::PutNumber("X Position",0.0);
		SmartDashboard::PutNumber("Y Position",0.0);
		break;
	}
	assert(ret);
	return ret;
}

void coodinate_manager_Interface::DestroyInstance(coodinate_manager_Interface *instance)
{
	delete instance;
}
