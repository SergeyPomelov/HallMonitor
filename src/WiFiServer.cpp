#include <ESP8266WebServer.h>
#include <Arduino.h>
#include <Constants.h>
#include <Secrets.h>
#include <Data.h>
#include <ArduinoJson.h>

ESP8266WebServer server(80);

void updateMessages()
{
  String postBody = server.arg("plain");
  Serial.println(postBody);
  StaticJsonDocument<300> parsed; // Пул памяти
  // Десериализация документа JSON
  DeserializationError error = deserializeJson(parsed, postBody);
  // Проверьте, удастся ли выполнить синтаксический анализ.
  if (error)
  {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
  else
  { // Вывести если ошибок нет

    Serial.println("There are no errors");
    delay(5000);
  }

  str1 = (const char *) parsed["1"];
  str2 = (const char *) parsed["2"];
  str3 = (const char *) parsed["3"];
}

boolean serverInit()
{
  Serial.print(F("Initialising Server "));
  server.begin();
  server.on("/messages/", HTTP_POST, updateMessages);
  return true;
}

void handleClient(void)
{
  server.handleClient();
}