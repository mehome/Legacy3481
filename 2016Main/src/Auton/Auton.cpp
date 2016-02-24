#include <WPILib.h>
#include "Auton.h"

void Auton::Start()
{
	/*while(1)
	{
		double range = ultra->GetRangeInches();
		SmartDashboard::PutNumber("Range", range);
	}*/
}
///*! Runs the autonomous portion of the program.*/
//void Auton::Start()
//{
//	int OriginalYaw = 1;
//	setRightDrive(.5);
//	setLeftDrive(.5);
//	Wait(3);
//	int NewYaw = 0;
//
//
//	while(1)
//	{
//		if(OriginalYaw==1)
//		OriginalYaw = ahrs->GetAngle();
//
//		NewYaw = ahrs->GetAngle();
//		SmartDashboard::PutNumber("Orgy Yaw", OriginalYaw);
//		SmartDashboard::PutNumber("New (Rounded) Yaw", NewYaw);
//
//		int range = ultra->GetRangeInches();
//
//		if ( !ahrs ) return;
//
//		bool reset_yaw_button_pressed = DriverStation::GetInstance().GetStickButton(0,.5);
//		if ( reset_yaw_button_pressed ) {
//			ahrs->ZeroYaw();
//		}
//
//		SmartDashboard::PutBoolean( "IMU_Connected",        ahrs->IsConnected());
//		SmartDashboard::PutNumber(  "IMU_Yaw",              ahrs->GetYaw());
//		SmartDashboard::PutNumber(  "IMU_Pitch",            ahrs->GetPitch());
//		SmartDashboard::PutNumber(  "IMU_Roll",             ahrs->GetRoll());
//		SmartDashboard::PutNumber(  "IMU_CompassHeading",   ahrs->GetCompassHeading());
//		SmartDashboard::PutNumber(  "IMU_Update_Count",     ahrs->GetUpdateCount());
//		SmartDashboard::PutNumber(  "IMU_Byte_Count",       ahrs->GetByteCount());
//
//        /* These functions are compatible w/the WPI Gyro Class */
//		SmartDashboard::PutNumber(  "IMU_TotalYaw",         ahrs->GetAngle());
//		SmartDashboard::PutNumber(  "IMU_YawRateDPS",       ahrs->GetRate());
//
//		SmartDashboard::PutNumber(  "IMU_Accel_X",          ahrs->GetWorldLinearAccelX());
//		SmartDashboard::PutNumber(  "IMU_Accel_Y",          ahrs->GetWorldLinearAccelY());
//		SmartDashboard::PutBoolean( "IMU_IsMoving",         ahrs->IsMoving());
//		SmartDashboard::PutNumber(  "IMU_Temp_C",           ahrs->GetTempC());
//		SmartDashboard::PutBoolean( "IMU_IsCalibrating",    ahrs->IsCalibrating());
//
//		SmartDashboard::PutNumber(  "Velocity_X",           ahrs->GetVelocityX() );
//		SmartDashboard::PutNumber(  "Velocity_Y",           ahrs->GetVelocityY() );
//		SmartDashboard::PutNumber(  "Displacement_X",       ahrs->GetDisplacementX() );
//		SmartDashboard::PutNumber(  "Displacement_Y",       ahrs->GetDisplacementY() );
//
//        /* Display Raw Gyro/Accelerometer/Magnetometer Values                       */
//        /* NOTE:  These values are not normally necessary, but are made available   */
//        /* for advanced users.  Before using this data, please consider whether     */
//        /* the processed data (see above) will suit your needs.                     */
//
//		SmartDashboard::PutNumber(  "RawGyro_X",            ahrs->GetRawGyroX());
//		SmartDashboard::PutNumber(  "RawGyro_Y",            ahrs->GetRawGyroY());
//		SmartDashboard::PutNumber(  "RawGyro_Z",            ahrs->GetRawGyroZ());
//		SmartDashboard::PutNumber(  "RawAccel_X",           ahrs->GetRawAccelX());
//		SmartDashboard::PutNumber(  "RawAccel_Y",           ahrs->GetRawAccelY());
//		SmartDashboard::PutNumber(  "RawAccel_Z",           ahrs->GetRawAccelZ());
//		SmartDashboard::PutNumber(  "RawMag_X",             ahrs->GetRawMagX());
//		SmartDashboard::PutNumber(  "RawMag_Y",             ahrs->GetRawMagY());
//		SmartDashboard::PutNumber(  "RawMag_Z",             ahrs->GetRawMagZ());
//		SmartDashboard::PutNumber(  "IMU_Temp_C",           ahrs->GetTempC());
//        /* Omnimount Yaw Axis Information                                           */
//        /* For more info, see http://navx-mxp.kauailabs.com/installation/omnimount  */
//        AHRS::BoardYawAxis yaw_axis = ahrs->GetBoardYawAxis();
//		SmartDashboard::PutString(  "YawAxisDirection",     yaw_axis.up ? "Up" : "Down" );
//		SmartDashboard::PutNumber(  "YawAxis",              yaw_axis.board_axis );
//
//        /* Sensor Board Information                                                 */
//		SmartDashboard::PutString(  "FirmwareVersion",      ahrs->GetFirmwareVersion());
//
//        /* Quaternion Data                                                          */
//        /* Quaternions are fascinating, and are the most compact representation of  */
//        /* orientation data.  All of the Yaw, Pitch and Roll Values can be derived  */
//        /* from the Quaternions.  If interested in motion processing, knowledge of  */
//        /* Quaternions is highly recommended.                                       */
//		SmartDashboard::PutNumber(  "QuaternionW",          ahrs->GetQuaternionW());
//		SmartDashboard::PutNumber(  "QuaternionX",          ahrs->GetQuaternionX());
//		SmartDashboard::PutNumber(  "QuaternionY",          ahrs->GetQuaternionY());
//		SmartDashboard::PutNumber(  "QuaternionZ",          ahrs->GetQuaternionZ());
//
//		if(NewYaw > OriginalYaw)
//		{
//			setLeftDrive(.5);
//			setRightDrive(-.5);
//		}
//		else if(NewYaw < OriginalYaw)
//		{
//			setRightDrive(.5);
//			setLeftDrive(-.5);
//		}
//		else
//		{
//			setRightDrive(0);
//			setLeftDrive(0);
//			SmartDashboard::PutString("Target ", "Acquired");
//		}
//	}
//
//	setRightDrive(0);
//	setLeftDrive(0);
//}
