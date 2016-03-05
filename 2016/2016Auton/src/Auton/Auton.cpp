#include <WPILib.h>
#include "Auton.h"

void Auton::Start(){

	/*SmartDashboard::PutString("", "0: XDefense\n1: YDefense\n2:ZDefense");
	int auton = SmartDashboard::GetNumber("Defense", 0);
	if(auton == 0){
		xDefense->Start();
	}else if(auton == 1){
		yDefense->Start();
	}else if(auton == 2){
<<<<<<< HEAD
		zDefense->Start();
	}
=======
		zDefense.Start();
	}*/

	int p_OriginalYaw = ahrs->GetAngle();
	int OriginalYaw = ahrs->GetAngle();

	  while(OriginalYaw == p_OriginalYaw)
	       OriginalYaw = ahrs->GetAngle();

	 // DriverStation::ReportError("OrgyYay="+OriginalYaw);
	  	//Wait(5);

	  Config::Instance()->GetSolenoid(CommonName::gearShift())->Set(DoubleSolenoid::kForward);

	    setRightDrive(-.75);
	    setLeftDrive(.75);
	    Wait(2.8);
	    int NewYaw = 0;
	    double range = 99999;

	    while(NewYaw > OriginalYaw+.1 || NewYaw < OriginalYaw-.1)
	    {
	        NewYaw = ahrs->GetAngle();
	        SmartDashboard::PutNumber("Orgy Yaw", OriginalYaw);
	        SmartDashboard::PutNumber("New (Rounded) Yaw", NewYaw);

	        if ( !ahrs ) return;

	       /* bool reset_yaw_button_pressed = DriverStation::GetInstance().GetStickButton(0,.5);
	        if ( reset_yaw_button_pressed ) {
	            ahrs->ZeroYaw();
	        }*/

	        SmartDashboard::PutBoolean( "IMU_Connected",        ahrs->IsConnected());
	        SmartDashboard::PutNumber(  "IMU_Yaw",              ahrs->GetYaw());
	        SmartDashboard::PutNumber(  "IMU_Pitch",            ahrs->GetPitch());
	        SmartDashboard::PutNumber(  "IMU_Roll",             ahrs->GetRoll());
	        SmartDashboard::PutNumber(  "IMU_CompassHeading",   ahrs->GetCompassHeading());
	        SmartDashboard::PutNumber(  "IMU_Update_Count",     ahrs->GetUpdateCount());
	        SmartDashboard::PutNumber(  "IMU_Byte_Count",       ahrs->GetByteCount());

	        /* These functions are compatible w/the WPI Gyro Class */
	        SmartDashboard::PutNumber(  "IMU_TotalYaw",         ahrs->GetAngle());
	        SmartDashboard::PutNumber(  "IMU_YawRateDPS",       ahrs->GetRate());

	        SmartDashboard::PutNumber(  "IMU_Accel_X",          ahrs->GetWorldLinearAccelX());
	        SmartDashboard::PutNumber(  "IMU_Accel_Y",          ahrs->GetWorldLinearAccelY());
	        SmartDashboard::PutBoolean( "IMU_IsMoving",         ahrs->IsMoving());
	        SmartDashboard::PutNumber(  "IMU_Temp_C",           ahrs->GetTempC());
	        SmartDashboard::PutBoolean( "IMU_IsCalibrating",    ahrs->IsCalibrating());

	        SmartDashboard::PutNumber(  "Velocity_X",           ahrs->GetVelocityX() );
	        SmartDashboard::PutNumber(  "Velocity_Y",           ahrs->GetVelocityY() );
	        SmartDashboard::PutNumber(  "Displacement_X",       ahrs->GetDisplacementX() );
	        SmartDashboard::PutNumber(  "Displacement_Y",       ahrs->GetDisplacementY() );

	        /* Display Raw Gyro/Accelerometer/Magnetometer Values                       */
	        /* NOTE:  These values are not normally necessary, but are made available   */
	        /* for advanced users.  Before using this data, please consider whether     */
	        /* the processed data (see above) will suit your needs.                     */

	        SmartDashboard::PutNumber(  "RawGyro_X",            ahrs->GetRawGyroX());
	        SmartDashboard::PutNumber(  "RawGyro_Y",            ahrs->GetRawGyroY());
	        SmartDashboard::PutNumber(  "RawGyro_Z",            ahrs->GetRawGyroZ());
	        SmartDashboard::PutNumber(  "RawAccel_X",           ahrs->GetRawAccelX());
	        SmartDashboard::PutNumber(  "RawAccel_Y",           ahrs->GetRawAccelY());
	        SmartDashboard::PutNumber(  "RawAccel_Z",           ahrs->GetRawAccelZ());
	        SmartDashboard::PutNumber(  "RawMag_X",             ahrs->GetRawMagX());
	        SmartDashboard::PutNumber(  "RawMag_Y",             ahrs->GetRawMagY());
	        SmartDashboard::PutNumber(  "RawMag_Z",             ahrs->GetRawMagZ());
	        SmartDashboard::PutNumber(  "IMU_Temp_C",           ahrs->GetTempC());
	        /* Omnimount Yaw Axis Information                                           */
	        /* For more info, see http://navx-mxp.kauailabs.com/installation/omnimount  */
	        AHRS::BoardYawAxis yaw_axis = ahrs->GetBoardYawAxis();
	        SmartDashboard::PutString(  "YawAxisDirection",     yaw_axis.up ? "Up" : "Down" );
	        SmartDashboard::PutNumber(  "YawAxis",              yaw_axis.board_axis );

	        /* Sensor Board Information                                                 */
	        SmartDashboard::PutString(  "FirmwareVersion",      ahrs->GetFirmwareVersion());

	        /* Quaternion Data                                                          */
	        /* Quaternions are fascinating, and are the most compact representation of  */
	        /* orientation data.  All of the Yaw, Pitch and Roll Values can be derived  */
	        /* from the Quaternions.  If interested in motion processing, knowledge of  */
	        /* Quaternions is highly recommended.                                       */
	        SmartDashboard::PutNumber(  "QuaternionW",          ahrs->GetQuaternionW());
	        SmartDashboard::PutNumber(  "QuaternionX",          ahrs->GetQuaternionX());
	        SmartDashboard::PutNumber(  "QuaternionY",          ahrs->GetQuaternionY());
	        SmartDashboard::PutNumber(  "QuaternionZ",          ahrs->GetQuaternionZ());

	        if(NewYaw > OriginalYaw)
	        {
	            setLeftDrive(.4);
	            setRightDrive(.4);
	        }
	        else if(NewYaw < OriginalYaw)
	        {
	            setRightDrive(-.4);
	            setLeftDrive(-.4);
	        }
	        else
	        {
	            setRightDrive(0);
	            setLeftDrive(0);
	            SmartDashboard::PutString("Target ", "Acquired");
	        }
	    }

	    while(range > 30)
	    {
	        range = ultra->GetRangeInches();
	        SmartDashboard::PutNumber("Distance from Something ", range);
	        setRightDrive(-.75);
	        setLeftDrive(.75);
	    }
	    // Angle condition to 90 deg from original
	    /*{
	    setRightDrive(.5);
	    setLeftDrive(-.5);
	    }*/

	    Config::Instance()->GetSolenoid(CommonName::gearShift())->Set(DoubleSolenoid::kReverse);
	    fullStop();
>>>>>>> a55406015dc35169154f28bb6cd0a468a2b94dc2
}
