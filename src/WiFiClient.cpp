#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <Arduino.h>
#include <Constants.h>
#include <Secrets.h>
#include <Data.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

ESP8266WiFiMulti WiFiMulti;

void hibernate()
{
  ESP.deepSleep(SLEEP_SEC * 1000U * 1000U);
  delay(10);
}

void WiFiinit()
{
  Serial.print(F("Initialising Wifi "));
  WiFiMulti.addAP(AP_SSID, AP_PASS);
  WiFi.mode(WIFI_STA);
  WiFi.persistent(true);
  WiFi.setAutoConnect(true);
  WiFi.setAutoReconnect(true);
}

boolean WiFiconnect()
{
  ESP.wdtFeed();
  if (WiFi.SSID() != AP_SSID)
  {
    WiFiinit();
  }

  unsigned long startTime = millis();
  unsigned long tries = 1;
  while (WiFiMulti.run() != WL_CONNECTED)
  {
    delay(200);
    Serial.print('.');
    ESP.wdtFeed();
    unsigned long elapsedTime = millis() - startTime;
    if (elapsedTime > 15000)
    {
      WiFiinit();
      startTime = millis();
      tries++;
    }
    if (tries > 3)
    {
      hibernate();
    }
  }
  Serial.print(" Connected! IP address: ");
  Serial.println(WiFi.localIP());
  return true;
}

String getDevice(String id, boolean terminal)
{
  ESP.wdtFeed();
  if ((WiFiMulti.run() == WL_CONNECTED))
  {
    String url = "http://" + String(BACKEND_HOST) + ":" + BACKEND_PORT + "/api/states/" + id;
    Serial.printf("GET 1: %s\n", url.c_str());

    WiFiClient client;
    HTTPClient http;
    http.begin(client, url);
    http.setTimeout(5000U);
    Serial.printf("GET 2: %s\n", url.c_str());

    http.addHeader("Accept", "*/*");
    http.addHeader("Host", BACKEND_HOST);
    http.addHeader("Authorization", String("Bearer") + " " + SECURITY_TOKEN);
    http.addHeader("Accept-Encoding", "gzip, deflate");
    http.addHeader("Accept-Language", "en,ru;q=0.9");
    http.addHeader("Cache-Control", "max-age=0");
    http.addHeader("Upgrade-Insecure-Requests", "1");
    http.addHeader("User-Agent", "ESP8266 Weather Station v5");

    if (terminal)
    {
      http.addHeader("Connection", "close");
    }
    else
    {
      http.addHeader("Connection", "keep-alive");
    }

    Serial.printf("GET 3: %s\n", url.c_str());
    ESP.wdtFeed();
    int httpCode = http.GET();
    Serial.printf("GET 5: %d\n", httpCode);
    if (httpCode > 0)
    {
      String payload = http.getString();
      Serial.println(payload);
      if (httpCode == HTTP_CODE_OK)
      {
        DynamicJsonDocument doc(2048);
        deserializeJson(doc, payload);
        return doc["state"];
      }
    }
    else
    {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }
  else
  {
    Serial.printf("[HTTP} Unable to connect\n");
  }
  return "undefined";
}

int postDevice(const String id, const String value, const boolean terminal)
{
  ESP.wdtFeed();
  int httpCode = 0;
  String url = "http://" + String(BACKEND_HOST) + ":" + BACKEND_PORT + "/api/services/virtual/set";
  String payload = String("{\"entity_id\":\"[id]\",\"value\":[value]}");
  payload.replace("[id]", id);
  payload.replace("[value]", value);

  WiFiClient client;
  HTTPClient http;
  http.begin(client, url);
  http.setTimeout(5000U);

  http.addHeader("Host", BACKEND_HOST);
  http.addHeader("Authorization", String("Bearer") + " " + SECURITY_TOKEN);
  http.addHeader("Accept", "application/json");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Upgrade-Insecure-Requests", "1");
  http.addHeader("User-Agent", "ESP8266 Weather Station v5");

  if (terminal)
  {
    http.addHeader("Connection", "close");
  }
  else
  {
    http.addHeader("Connection", "keep-alive");
  }

  Serial.println((char *)url.c_str());
  Serial.println((char *)payload.c_str());

  httpCode = http.POST(payload);
  if (httpCode > 0)
  {

    if (httpCode == HTTP_CODE_OK)
    {
      String payload = http.getString();
      Serial.println(payload);
    }
    else
    {
      Serial.printf("POST code: %d\n", httpCode);
    }
  }
  else
  {
    Serial.printf("Send failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();
  return httpCode;
}

void updateFromServer()
{
  if (WiFiconnect())
  {
    str1 = getDevice(String("input_text.hall_message_1"), false);
    str2 = getDevice(String("input_text.hall_message_2"), false);
    str3 = getDevice(String("input_text.hall_message_3"), true);
  }
  else
  {
    Serial.println("Not connected GET");
  }
}

void sendData()
{
  if (insideTemp != 0 && insideTemp != 255 && insideHum != 0 && insideHum != 255 && WiFiconnect())
  {
    postDevice("sensor.virtual_hall_temperature", String(insideTemp, 2), false);
    postDevice("sensor.virtual_hall_humidity", String(insideHum, 2), true);
  }
  else
  {
    Serial.println("Not connected POST");
  }
}