#pragma once

class COMMON_API UDP_Listener_Interface
{
	public:
		virtual ~UDP_Listener_Interface() {}  
		virtual void ProcessPacket(char *pkt,size_t pkt_size)=0;
};

class COMMON_API coodinate_manager_Interface
{
	public:
		enum ListeningPlatform
		{
			eListeningPlatform_UDP,
			eListeningPlatform_TCPIP,
		};
		static coodinate_manager_Interface *CreateInstance(ListeningPlatform listeningPlatform);
		static void DestroyInstance(coodinate_manager_Interface *instance);

		virtual ~coodinate_manager_Interface() {}
		//virtual void TimeChange(double dTime_s)=0;
		void ResetUpdate() {}

		virtual double GetXpos() const =0;
		virtual double GetYpos() const =0;
		virtual bool IsUpdated() const =0;
};

class COMMON_API coodinate_manager :	public UDP_Listener_Interface,
							public coodinate_manager_Interface
{
protected:
	double m_Xpos,m_Ypos;
	bool m_Updated; //true if we received a packet for this slice of time
protected:  //from coodinate_manager_Interface
	//virtual void TimeChange(double dTime_s)=0;
	void ResetUpdate() {m_Updated=false;}
	
	virtual double GetXpos() const {return m_Xpos;}
	virtual double GetYpos() const {return m_Ypos;}
	virtual bool IsUpdated() const {return m_Updated;}
};
