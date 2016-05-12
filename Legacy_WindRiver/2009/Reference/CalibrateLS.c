//*!!Sensor,    S1,          touchSensor, sensorTouch,      ,                    !!*//
//*!!Sensor,    S2,          lightSensor, sensorLightActive,      ,              !!*//
//*!!                                                                            !!*//
//*!!Start automatically generated configuration code.                           !!*//
const tSensors touchSensor          = (tSensors) S1;   //sensorTouch        //*!!!!*//
const tSensors lightSensor          = (tSensors) S2;   //sensorLightActive  //*!!!!*//
//*!!CLICK to edit 'wizard' created sensor & motor configuration.                !!*//

task main()
{

	int lightValue;
	int darkValue;
	int sumValue;
	int thresholdValue;

	while(SensorValue(touchSensor) == 0)
	{
		nxtDisplayStringAt(0,31,"Read Light Now");
	}

	lightValue = SensorValue(lightSensor);

	wait1Msec(1000);

	while(SensorValue(touchSensor) == 0)
	{
		nxtDisplayStringAt(0,31,"Read Dark Now");
	}

	darkValue = SensorValue(lightSensor);

	sumValue = lightValue + darkValue;
	thresholdValue = sumValue/2;

	ClearTimer(T1);

	while(time1[T1] < 3000)
	{

		if(SensorValue(lightSensor) < thresholdValue)
		{

			motor[motorC] = 0;
			motor[motorB] = 80;

		}

		else
		{

			motor[motorC] = 80;
			motor[motorB] = 0;

		}

	}

	motor[motorC] = 0;
	motor[motorB] = 0;

}
