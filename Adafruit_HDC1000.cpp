/***************************************************
  This is a library for the HDC1000 Humidity & Temp Sensor

  Designed specifically to work with the HDC1008 sensor from Adafruit
  ----> https://www.adafruit.com/products/2635

  These sensors use I2C to communicate, 2 pins are required to
  interface
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  BSD license, all text above must be included in any redistribution

  Modified for Photon  needs application.h for types  RMB
 ****************************************************/
#include "application.h"
#include "Adafruit_HDC1000.h"


Adafruit_HDC1000::Adafruit_HDC1000() {
}


boolean Adafruit_HDC1000::begin(uint8_t addr) {
  _i2caddr = addr;

  Wire.begin();

  reset();
  if (read16(HDC1000_MANUFID) != 0x5449) return false;
  if (read16(HDC1000_DEVICEID) != 0x1000) return false;
  return true;
}



void Adafruit_HDC1000::reset(void) {
  // reset,combined temp/humidity measurement,and select 14 bit temp & humidity resolution
  // heater not needed for accurate humidity readings     RMB
  uint16_t config = HDC1000_CONFIG_RST | HDC1000_CONFIG_MODE | HDC1000_CONFIG_TRES_14 | HDC1000_CONFIG_HRES_14 ;

  Wire.beginTransmission(_i2caddr);
  Wire.write(HDC1000_CONFIG);   // set pointer register to configuration register   RMB
  Wire.write(config>>8);        // now write out 2 bytes MSB first    RMB
  Wire.write(config&0xFF);
  Wire.endTransmission();
  delay(15);
}


float Adafruit_HDC1000::readTemperature(void) {
  // does not set private variable
  float temp = (read32(HDC1000_TEMP, 20) >> 16);

  temp /= 65536;
  temp *= 165;
  temp -= 40;

  return temp;
}


float Adafruit_HDC1000::readHumidity(void) {
  // reads both temp and humidity but masks out temp in highest 16 bits
  // does not set private variable
  float hum = (read32(HDC1000_TEMP, 20) & 0xFFFF);

  hum /= 65536;
  hum *= 100;

  return hum;
}

void Adafruit_HDC1000::ReadTempHumidity(void) {
  // HDC1008 setup to measure both temperature and humidity in one sequential conversion
  // this is a different way to access data in ONE read
  // this sets internal private variables that can be accessed by Get() functions

  uint32_t rt,rh ;  // working variables

  rt = read32(HDC1000_TEMP, 20);    // get temp and humidity reading together
  rh = rt;                          // save a copy for humidity processing

  // important to use ( ) around temp so private variable set and float cast done
  float (temp = (rt >> 16));        // convert to temp first
  temp /= 65536;
  temp *= 165;
  temp -= 40;

  // important to use ( ) around humidity so private variable set and float cast done
  float (humidity = (rh & 0xFFFF));   // now convert to humidity
  humidity /= 65536;
  humidity *= 100;
}

float Adafruit_HDC1000::GetTemperature(void) {
  // getter function to access private temp variable

  return temp ;
}

float Adafruit_HDC1000::GetHumidity(void) {
  // getter function to access private humidity variable

  return humidity ;
}

// Add ability to test battery voltage, useful in remote monitoring, TRUE if <2.8V
// usually called after Temp/Humid reading  RMB
// Thanks to KFricke for micropython-hdc1008 example on GitHub
boolean Adafruit_HDC1000::batteryLOW(void)  {
  // set private variable, don't need delay to read Config register
  battLOW = (read16(HDC1000_CONFIG, 0));

  battLOW &= HDC1000_CONFIG_BATT;   // mask off other bits, bit 11 will be 1 if voltage < 2.8V

  if (battLOW > 0) return true;
  return  false;
}



/*********************************************************************/

uint16_t Adafruit_HDC1000::read16(uint8_t a, uint8_t d) {
  Wire.beginTransmission(_i2caddr);
  Wire.write(a);
  Wire.endTransmission();
  delay(d);
  Wire.requestFrom((uint8_t)_i2caddr, (uint8_t)2);
  uint16_t r = Wire.read();
  r <<= 8;
  r |= Wire.read();
  //Serial.println(r, HEX);
  return r;
}

uint32_t Adafruit_HDC1000::read32(uint8_t a, uint8_t d) {
  Wire.beginTransmission(_i2caddr);
  Wire.write(a);
  Wire.endTransmission();
  // delay was hardcoded as 50, should use d   RMB
  delay(d);
  Wire.requestFrom((uint8_t)_i2caddr, (uint8_t)4);
  uint32_t r = Wire.read();
  // assembles temp into highest 16 bits, humidity into lowest 16 bits
  r <<= 8;
  r |= Wire.read();
  r <<= 8;
  r |= Wire.read();
  r <<= 8;
  r |= Wire.read();
  //Serial.println(r, HEX);
  return r;
}
