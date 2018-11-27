/**********************************************
  HDCMonitor_Example by Ron M. Battle 1-2-2017
  based on Adafruit_HDC1008 temp/humidity board/libraries
  Uses Adafruit mini flexible WiFi external antenna #2308

  Wakes up from SLEEP every 15 min, gets data from sensor then
  publishes to Particle Cloud every hour in JSON format
  You can set up a webhook integration in Particle Console using 'custom JSON'
  for ThingSpeak and update a CHANNEL to monitor readings graphically

  The HDCMonitor_Example.html program runs in a browser and gets published
  Particle Cloud HDCUpdates and shows values on screen using JustGages.

***********************************************/

// Connect Vin to 3-5VDC
// Connect GND to ground
// Connect SCL to I2C clock pin (D1 on Photon/Electron)
// Connect SDA to I2C data pin  (D0 on Photon/Electron)

#include "Adafruit_HDC1000.h"                     

#define ZZZ 900        // seconds of SLEEP  900 = 15 minutes
#define PUBTIME 6000  // millisecs of publish waiting

// Need this to turn on Backup SRAM
STARTUP(System.enableFeature(FEATURE_RETAINED_MEMORY));

// retained variables in Backup SRAM still available after SLEEP mode
retained float humidity = 0, tempC = 0, tempF = 0, mintemp = 77, maxtemp = 32;
retained int batteryLOW = 0;   // TRUE (1)  if <2.8V
retained int count = 0;       // counter for Publishing

retained unsigned int HoursAlive = 0;  // hours alive counter (approximate!)

const char * key = "YourChannelWriteApiKey"; // Thingspeak api channel write key
char json[255] ;                            // published string in JSON format

// new instance
Adafruit_HDC1000 hdc = Adafruit_HDC1000();

// active Publish delay
void spinWait(uint32_t timeout) {
  uint32_t start = millis();
  while (millis() - start < timeout) Particle.process() ;
}

void setup() {
  WiFi.selectAntenna(ANT_EXTERNAL);  // use more powerful external antenna
  hdc.begin();
  delay(25);        // let the chip initialize
}

void loop() {
  ++count ;                         // get reading every 15 min, publish hourly
  tempC = hdc.readTemperature() ;  // Celsius
  tempF = (tempC * 1.8) + 32 ;    // Fahrenheit
  humidity = hdc.readHumidity() ;
  batteryLOW = hdc.batteryLOW();

  if (tempF > maxtemp)  maxtemp = tempF;
  if (tempF < mintemp)  mintemp = tempF;

  if (count == 4) {
    count = 0;      // will publish every 4th wakeup
    ++HoursAlive;  // increment another hour

    // format string does not work unless quotes escaped: \"
    // key needs quotes around it for ThingSpeak
    sprintf(json,"{\"1\":%i,\"2\":%u,\"3\":%.1f,\"4\":%.1f,\"5\":%.1f,\"6\":%.1f,\"k\":\"%s\"}",
      batteryLOW,HoursAlive,humidity,maxtemp,mintemp,tempF,key);

    // Publish results to Cloud
    Particle.publish("HDCUpdate", json, PRIVATE);
    spinWait(PUBTIME);   // wait some time to publish

    if (batteryLOW) {
      // Need separate publish for IFTTT service; they can't parse JSON
      // Could set up an applet to send you an SMS text to "CHARGE BATTERY!"
      // You will get a lot of texts unless you get that battery charged!
      Particle.publish("HDCBattery","LOW!",PRIVATE);
      spinWait(PUBTIME);   // wait some time to publish
    }
  }
  System.sleep(D2, RISING, ZZZ);    // go to sleep  uses less power
 }
