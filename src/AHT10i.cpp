#include <Arduino.h>
#include <Constants.h>
#include <Data.h>
#include <AHT10.h>

AHT10 myAHT10(AHT10_ADDRESS_0X38);

boolean AHT10init()
{
  while (myAHT10.begin() != true)
  {
    Serial.println(F("AHT10 not connected or fail to load calibration coefficient")); //(F()) save string to flash & keeps dynamic memory free
    delay(100);
  }
  return true;
}

void AHT10read()
{
  insideTemp = myAHT10.readTemperature();
  insideHum = myAHT10.readHumidity();
}
