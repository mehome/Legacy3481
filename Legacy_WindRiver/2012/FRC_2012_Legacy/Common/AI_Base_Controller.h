#pragma once



class AI_Base_Controller
{
	public:
		typedef Framework::Base::Vec2d Vec2D;
		AI_Base_Controller(Ship_2D &ship);

		///This is the single update point to all controlling of the ship.  The base class contains no goal arbitration, but does implement
		///Whatever goal is passed into it if the UI controller is off line
		virtual void UpdateController(double dTime_s);

		//This is mostly for pass-thru since the timing of this is in alignment during a ship att-pos update
		void UpdateUI(double dTime_s);

		/// I put the word try, as there may be some extra logic to determine if it has permission
		/// This is a bit different than viewing an AI with no controls, where it simply does not
		/// Allow a connection
		/// \return true if it was allowed to bind
		virtual bool Try_SetUIController(UI_Controller *controller);
		virtual void ResetPos() {}

		bool HasAutoPilotRoute() {return true;}
		bool GetCanUserPilot() {return true;}

		void SetShipVelocity(double velocity_mps) {m_ship.SetRequestedVelocity(velocity_mps);}

		/// \param TrajectoryPoint- This is the point that your nose of your ship will orient to from its current position (usually the same as PositionPoint)
		/// \param PositionPoint- This is the point where your ship will be to position to (Usually the same as TrajectoryPoint)
		/// \power- The scaled value multiplied to the ships max speed.  If > 1 it can be interpreted as explicit meters per second speed
		/// \matchVel- You can make it so the velocity of the ship when it reaches the point.  
		/// Use NULL when flying through way-points
		/// Use (0,0) if you want to come to a stop, like at the end of a way-point series
		/// Otherwise, use the velocity of the ship you are targeting or following to keep up with it
		void DriveToLocation(Vec2D TrajectoryPoint,Vec2D PositionPoint, double power, double dTime_s,Vec2D* matchVel,bool LockOrientation=false);
		void SetIntendedOrientation(double IntendedOrientation) {m_ship.SetIntendedOrientation(IntendedOrientation);}
		
		Ship_2D &GetShip() {return m_ship;}
	protected:
		friend class Ship_Tester;
		Goal *m_Goal; //Dynamically set a goal for this controller
	private:
		//TODO determine way to properly introduce UI_Controls here	
		Ship_2D &m_ship;

		friend class UI_Controller;
		UI_Controller *m_UI_Controller;
};

//This will explicitly rotate the ship to a particular heading.  It may be moving or still.
class Goal_Ship_RotateToPosition : public AtomicGoal
{
	public:
		Goal_Ship_RotateToPosition(AI_Base_Controller *controller,double Heading);
		~Goal_Ship_RotateToPosition();
		virtual void Activate();
		virtual Goal_Status Process(double dTime_s);
		virtual void Terminate() {m_Terminate=true;}
	private:
		AI_Base_Controller * const m_Controller;
		double m_Heading;
		Ship_2D &m_ship;
		bool m_Terminate;
};

//TODO get these functions re-factored
//Update Reaction->FlyWayPoints->UpdateIndendedLocation
//FlyToNextLocation->DriveToLocation

struct WayPoint
{
	WayPoint() : Power(0.0), Position(0,0),TurnSpeedScaler(1.0) {}
	double Power;
	Framework::Base::Vec2d Position;
	//osg::Vec2d Position;
	double TurnSpeedScaler;  //This will have a default value if not in script
};

//This is similar to Traverse_Edge in book (not to be confused with its MoveToPosition)
class Goal_Ship_MoveToPosition : public AtomicGoal
{
	public:
		/// \param double safestop_tolerance used to set safe stop tolerance, default one/tenth of a meter is about 4 inches
		Goal_Ship_MoveToPosition(AI_Base_Controller *controller,const WayPoint &waypoint,bool UseSafeStop=true,
				bool LockOrientation=false,double safestop_tolerance=0.10);
		~Goal_Ship_MoveToPosition();
		virtual void Activate();
		virtual Goal_Status Process(double dTime_s);
		virtual void Terminate();

	protected:
		//Similar to FlyWayPoints, except it only checks for the tolerance
		bool HitWayPoint();  

	private:
		WayPoint m_Point;
		AI_Base_Controller * const m_Controller;
		Ship_2D &m_ship;
		double m_SafeStopTolerance;
		bool m_Terminate;
		bool m_UseSafeStop;
		bool m_LockOrientation;
};

class Goal_Ship_FollowPath : public CompositeGoal
{
	public:
		Goal_Ship_FollowPath(AI_Base_Controller *controller,std::list<WayPoint> path,bool LoopMode=false);
		virtual void Activate();
		virtual Goal_Status Process(double dTime_s);
		virtual void Terminate();
	private:
		AI_Base_Controller * const m_Controller;
		std::list<WayPoint> m_Path,m_PathCopy;
		bool m_LoopMode;
};

class Goal_Ship_FollowShip : public AtomicGoal
{
	public:
		typedef Framework::Base::Vec2d Vec2D;
		//typedef osg::Vec2d Vec2D;
		/// \param Trajectory_ForwardOffset This control where the orientation of the following ship will look.  This can vary depending on the size
		/// of the ship.  This should be virtually 0 if the ship has no strafe
		Goal_Ship_FollowShip(AI_Base_Controller *controller,const Ship_2D &Followship,const Vec2D &RelPosition,double Trajectory_ForwardOffset=100.0);
		~Goal_Ship_FollowShip();
		virtual void Activate();
		virtual Goal_Status Process(double dTime_s);
		virtual void Terminate();
		//Allow client to change its relative position dynamically
		void SetRelPosition(const Vec2D &RelPosition);
	private:
		AI_Base_Controller * const m_Controller;
		Vec2D m_RelPosition,m_TrajectoryPosition;
		double m_TrajectoryPosition_ForwardOffset; //The amount forward to extend the trajectory point
		const Ship_2D &m_Followship;
		Ship_2D &m_ship;
		bool m_Terminate;
};

class Goal_Wait : public AtomicGoal
{
	public:
		Goal_Wait(double seconds);
		virtual void Activate();
		virtual Goal_Status Process(double dTime_s);
		virtual void Terminate();
	private:
		double m_TimeAccrued;
		double m_TimeToWait;
};

//This goal simply will fire an event when all goals are complete
class Goal_NotifyWhenComplete : public CompositeGoal
{
	private:
		typedef CompositeGoal __super;
		std::string m_EventName;  //name to fire when complete
		Framework::Base::EventMap &m_EventMap;
	public:
		Goal_NotifyWhenComplete(Framework::Base::EventMap &em,char *EventName);
		//give public access for client to populate goals
		virtual void AddSubgoal(Goal *g) {__super::AddSubgoal(g);}
		//client activates manually when goals are added
		virtual void Activate();
		virtual Goal_Status Process(double dTime_s);
		virtual void Terminate();
};

#if 0
class AI_Controller : public AI_Base_Controller
{
	public:
	private:
};
#endif
