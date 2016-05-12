task main()
{
  int offset, magnetic_value;

  SetSensorHTMagnet (MAGNET);
  offset = SensorHTMagnet (Magnet, 0);
  Wait(1000);
 while(true)
 {
    magnetic_value = SensorHTMagnet(Magnet, offset);


}
