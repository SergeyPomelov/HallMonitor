#include <Arduino.h>
#include <pins_arduino.h>
#include <Constants.h>
#include <Data.h>
#include <I2C.h>
#include "Ticker.h"
#include <LCD.h>
#include <BME280.h>
#include <AHT10i.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <Ntp.h>

long lastDataSend = 0;
boolean status = true;

Ticker updateTicker = Ticker();
Ticker ntpTiker = Ticker();

void update()
{
  ESP.wdtFeed();
  AHT10read();
  Serial.println(timeStr + String(" Temp ") + insideTemp +  " Hum " + insideHum);
  textToLcd(0U, timeStr + " " + String(insideTemp, 2) + "C " + String(insideHum, 2) + "%");
  textToLcd(1U, str1);
  textToLcd(2U, str2);
  textToLcd(3U, str3);
}

void initDevice(const String deviceName, const uint8_t displayLine, const boolean watchdogDisable, boolean (*init)())
{
  ESP.wdtFeed();
  if (watchdogDisable) {
    ESP.wdtEnable(0U);
  }
  Serial.print(deviceName + "... ");
  if (deviceName != "LCD")
  {
    textToLcd(displayLine, deviceName);
  }
  if (init())
  {
    if (deviceName == "LCD") {
      textToLcd(displayLine, "LCD");
    }
    textToLcd(displayLine, deviceName + " OK");
    Serial.println(F("OK"));
  }
  else
  {
    textToLcd(displayLine, deviceName + " ERROR");
    Serial.println(F("ERROR"));
    status = false;
  }
  ESP.wdtFeed();
}

void initDevice(const String deviceName, const uint8_t displayLine, boolean (*init)()) {
  initDevice(deviceName, displayLine, false, init);
}

void setup()
{
  Serial.begin(74880);
  Serial.setDebugOutput(true);
  ESP.wdtDisable();
  scanI2C();
  initDevice(String("LCD"), 0U, initLcd);
  initDevice(String("AHT10"), 1U, AHT10init);
  initDevice(String("WiFi"), 2U, true, WiFiconnect);
  updateFromServer();
  initDevice(String("NTP"), 3U, ntpInit);
  serverInit();
  ntpTiker.attach_ms(INTERVAL_UPDATE_MS, updateTime);
  updateTicker.attach_ms(INTERVAL_UPDATE_MS, update);
}

void loop(void)
{
  unsigned long elapsedTime = millis() - lastDataSend;
  if (elapsedTime > INTERVAL_DATA_SEND_MS)
  {
    sendData();
    lastDataSend = millis();
  }
  handleClient();
}
