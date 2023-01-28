#include <Arduino.h>
#include <Constants.h>
#include <Data.h>
#include <SPI.h>
#include <LiquidCrystal_I2C.h>

#define MAX_STR_LENGH 20U

LiquidCrystal_I2C lcd(0x3F, MAX_STR_LENGH, 4);

void clearLcd()
{
  lcd.clear();
}

boolean initLcd()
{
  lcd.init();
  clearLcd();
  lcd.backlight();
  return true;
}

String truncate(const String str)
{
  String out = String(str);
  unsigned int length = out.length();
  if (length > MAX_STR_LENGH) {
    return out.substring(0, MAX_STR_LENGH);
  }

  for (unsigned int i = 0U; i < MAX_STR_LENGH - length; i++)
  {
    out.concat(' ');
  }
  return out;
}

void textToLcd(const uint8_t row, const String str)
{
  lcd.setCursor(0, row);
  lcd.print(truncate(str));
}