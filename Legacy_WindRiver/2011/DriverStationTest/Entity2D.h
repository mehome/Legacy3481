#pragma once

inline void NormalizeRotation(double &Rotation)
{
	const double Pi2=M_PI*2.0;
	//Normalize the rotation
	if (Rotation>M_PI)
		Rotation-=Pi2;
	else if (Rotation<-M_PI)
		Rotation+=Pi2;
}

class Entity1D
{
	private:
		friend class Entity1D_Properties;

		Framework::Base::EventMap* m_eventMap;
		double m_Dimension;
		double m_Position;
		std::string m_Name;
	public:
		Entity1D(const char EntityName[]);

		//This allows the game client to setup the ship's characteristics
		virtual void Initialize(Framework::Base::EventMap& em, const Entity1D_Properties *props=NULL);
		virtual ~Entity1D(); //Game Client will be nuking this pointer
		const std::string &GetName() const {return m_Name;}
		virtual void TimeChange(double dTime_s);
		PhysicsEntity_1D &GetPhysics() {return m_Physics;}
		const PhysicsEntity_1D &GetPhysics() const {return m_Physics;}
		virtual double GetDimension() const {return m_Dimension;}
		virtual void ResetPos();
		// This is where both the entity and camera need to align to, by default we use the actual position
		virtual const double &GetIntendedPosition() const {return m_Position;}
		Framework::Base::EventMap* GetEventMap(){return m_eventMap;}

		virtual double GetPos_m() const {return m_Position;}
		//This is used when a sensor need to correct for the actual position
		void SetPos_m(double value) {m_Position=value;}
	protected: 
		PhysicsEntity_1D m_Physics;
};


class Ship_Tester;
//This contains everything the AI needs for game play; Keeping this encapsulated will help keep a clear division
//of what Entity3D looked like before applying AI with goals

//Note Entity2D should not know anything about an actor
class Entity2D
{
	public:
		typedef Framework::Base::Vec2d Vec2D;

	private:
		friend class Ship_Tester;
		friend class Entity_Properties;

		struct PosAtt
		{
			Vec2D m_pos_m;
	
			//2d Orientation:
			double m_att_r;  //a.k.a heading
			//measurement in radians where 0 points north, pi/2 = east,  pi=south, and -pi/2 (or pi + pi/2) = west
			//from this a normalized vector can easily be computed by
			//x=sin(heading) y=cos(heading)
			//We can keep the general dimensions of the entity
			//Note: we do not need pitch or roll axis so this keeps things much simpler
		} m_PosAtt_Buffers[2];
	
		//All read cases use the read pointer, all write cases use the write pointer followed by an interlocked exchange of the pointers
		PosAtt *m_PosAtt_Read,*m_PosAtt_Write; 
		void UpdatePosAtt();
	
		Framework::Base::EventMap* m_eventMap;
	
		Vec2D m_Dimensions;
		std::string m_Name;

	public:
		Entity2D(const char EntityName[]);

		//This allows the game client to setup the ship's characteristics
		virtual void Initialize(Framework::Base::EventMap& em, const Entity_Properties *props=NULL);
		virtual ~Entity2D(); //Game Client will be nuking this pointer
		const std::string &GetName() const {return m_Name;}
		virtual void TimeChange(double dTime_s);
		FlightDynamics_2D &GetPhysics() {return m_Physics;}
		const FlightDynamics_2D &GetPhysics() const {return m_Physics;}
		virtual const Vec2D &GetDimensions() const {return m_Dimensions;}
		virtual void ResetPos();
		// This is where both the vehicle entity and camera need to align to, by default we use the actual orientation
		virtual const double &GetIntendedOrientation() const {return m_PosAtt_Read->m_att_r;}
		Framework::Base::EventMap* GetEventMap(){return m_eventMap;}

		//from EntityPropertiesInterface
		virtual const Vec2D &GetPos_m() const {return m_PosAtt_Read->m_pos_m;}
		virtual double GetAtt_r() const {return m_PosAtt_Read->m_att_r;}
	protected: 
		FlightDynamics_2D m_Physics;
};

