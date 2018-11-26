/*
 * Project photon-gcp-greenhouse
 * Description: Greenhouse environment monitor based on Particle devices and Google Cloud Platform for data storage and analysis
 * Author: Zack Huber
 * Date:
 */

// This makes sure the Photon is using the best wireless signal.
STARTUP(WiFi.selectAntenna(ANT_AUTO));
    //=========================================================================
// reset the system after 60 seconds if the application is unresponsive
ApplicationWatchdog wd(60000, System.reset);
// EXAMPLE USAGE
/*
void loop() {
  while (some_long_process_within_loop) {
    wd.checkin(); // resets the AWDT count
  }
}
// AWDT count reset automatically after loop() ends
*/
    //=========================================================================
// This enables retained variables in the Backup RAM (SRAM) 
STARTUP(System.enableFeature(FEATURE_RETAINED_MEMORY));

/*
When adding new retained variables to an existing set of retained variables,
it's a good idea to add them after the existing variables.
This ensures the existing retained data is still valid even with the new code.
*/
retained double L0;
retained double L1;
retained double L2;
retained double L3;
retained double L4;
retained double L5;

retained double wT0;
retained double wT1;
retained double wT2;
retained double wT3;
retained double wT4;
retained double wT5;

retained double gT0; // ??? is this? why is it showing up as a Class method?
retained double gT1;
retained double gT2;
retained double gT3;
retained double gT4;
retained double gT5;

retained double hT0;
retained double hT1;
retained double hT2;
retained double hT3;
retained double hT4;
retained double hT5;

retained double aT0;
retained double aT1;
retained double aT2;
retained double aT3;
retained double aT4;
retained double aT5;

retained double aH0;
retained double aH1;
retained double aH2;
retained double aH3;
retained double aH4;
retained double aH5;

retained uint32_t ts0;
retained uint32_t ts1;
retained uint32_t ts2;
retained uint32_t ts3;
retained uint32_t ts4;
retained uint32_t ts5;

    //=========================================================================

/*
I/O setup:
These made it easy to just 'plug in' my 18B20

D3 - 1-wire ground, or just use regular pin and comment out below.
D4 - 1-wire signal, 2K-10K resistor to D5 (3v3)
D5 - 1-wire power, ditto ground comment.
*/
#include "DS18.h"

DS18 sensor(A3);
    //=========================================================================

#include "Adafruit_HDC1000.h"

Adafruit_HDC1000 hdc = Adafruit_HDC1000();
    //=========================================================================

#include "Adafruit_TSL2561_U.h"

/* This driver uses the Adafruit unified sensor library (Adafruit_Sensor),
   which provides a common 'type' for sensor data and some helper functions.

   To use this driver you will also need to download the Adafruit_Sensor
   library and include it in your libraries folder.

   You should also assign a unique ID to this sensor for use with
   the Adafruit Sensor API so that you can identify this particular
   sensor in any data logs, etc.  To assign a unique ID, simply
   provide an appropriate value in the constructor below (12345
   is used by default in this example).

   Connections
   ===========
   Connect SCL to analog 5 //different pins on Particle Photon
   Connect SDA to analog 4 //different pins on Particle Photon
   Connect VDD to 3.3V DC
   Connect GROUND to common ground

   I2C Address
   ===========
   The address will be different depending on whether you leave
   the ADDR pin floating (addr 0x39), or tie it to ground or vcc.
   The default addess is 0x39, which assumes the ADDR pin is floating
   (not connected to anything).  If you set the ADDR pin high
   or low, use TSL2561_ADDR_HIGH (0x49) or TSL2561_ADDR_LOW
   (0x29) respectively.

   History
   =======
   2013/JAN/31  - First version (KTOWN)
*/

Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12346);
// 2nd argument is a unique identifier for the TSL2561 attached to the device running this code.  Not getting used in cloud connected devices, duh!

const int SensorFrequency = 4000;  // I2C bus sensor polling frequency
unsigned long LastReading = 4000;
    //=========================================================================

const int PublishFrequency = 600000; // gcp upload frequency
unsigned long LastPublish = 20000;

char googleString[255]; // sensor_data_toGCP JSON string, 255 bytes max for Particle.publish data object
    //=========================================================================


// Variable Declarations
double waterTemp = 0;
double greenhouseTemp = 0;
double heattankTemp = 0;
double ambientTempC = 0;
double ambientTempF = 0;
double ambientHumidity = 0;
double lux = 0;
//int brightness = 0;
    //=========================================================================

char ROMCODE[10];
//char greenhouseTemp[10]; //should NOT be declared globally, REMOVE!
    //=========================================================================


// Adafruit I2C sensor functions
/**************************************************************************/
/*
    Displays some basic information on this sensor from the unified
    sensor API sensor_t type (see Adafruit_Sensor for more information)
*/
/**************************************************************************/
void displaySensorDetails(void)
{
  sensor_t sensor;
  tsl.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" lux");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" lux");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" lux");
  Serial.println("------------------------------------");
  Serial.println("");
  Particle.publish("Sensor: ", sensor.name);
  delay(500);
}

/**************************************************************************/
/*
    Configures the gain and integration time for the TSL2561
*/
/**************************************************************************/
void configureSensor(void)
{
  /* You can also manually set the gain or enable auto-gain support */
  tsl.setGain(TSL2561_GAIN_1X);      /* No gain ... use in bright light to avoid sensor saturation */
  // tsl.setGain(TSL2561_GAIN_16X);     /* 16x gain ... use in low light to boost sensitivity */
  // tsl.enableAutoRange(true);            /* Auto-gain ... switches automatically between 1x and 16x */

  /* Changing the integration time gives you better sensor resolution (402ms = 16-bit data) */
  // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      /* fast but low resolution */
  // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);  /* medium resolution and speed   */
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  /* 16-bit data but slowest conversions */

  /* Update these values depending on what you've set above! */
  Serial.println("------------------------------------");
  Serial.print  ("Gain:         "); Serial.println("3x gain");
  Serial.print  ("Timing:       "); Serial.println("402 ms");
  Serial.println("------------------------------------");
}


void setup()
{
  Serial.begin(9600);
  // Set up 'power' pins, comment out if not used! (Set up as I2C power pins on current Particle Photon board)
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  digitalWrite(D2, LOW);
  digitalWrite(D3, HIGH);



  Serial.println("Light Sensor Test"); Serial.println("");
  Particle.publish("Testing TSL2561"); //REMOVE FROM setup() for semi-automatic particle.connect control

  // Initialise the sensor
  if(!tsl.begin())
  {
    // There was a problem detecting the ADXL345 ... check your connections
    Serial.print("Ooops, no TSL2561 detected ... Check your wiring or I2C ADDR!");
    Particle.publish("Ooops, no TSL2561 detected ... Check your wiring or I2C ADDR!"); //REMOVE FROM setup() for semi-automatic particle.connect control
  }

  // Display some basic information on this sensor
  displaySensorDetails();

  // Setup the sensor gain and integration time
  configureSensor();


  Serial.println("HDC100x test");

  if(!hdc.begin())
  {
      Serial.print("No HDC1000 detected ...  Check your wiring or I2C ADDR!");
      Particle.publish("No HDC1000 detected ...  Check your wiring or I2C ADDR!");
  }
  delay(15);    // let the chip initialize

}

void loop()
{
  delay(5000);

  int currentMinute = Time.minute();
  int currentTens_place = currentMinute / 10;

  unsigned long CurrentMillis = millis();

  if ((CurrentMillis - LastReading) > SensorFrequency)
  {
      switch (currentTens_place) {
            case 0: ts0 = Time.now();
                    break;
            case 1: ts1 = Time.now();
                    break;
            case 2: ts2 = Time.now();
                    break;
            case 3: ts3 = Time.now();
                    break;
            case 4: ts4 = Time.now();
                    break;
            case 5: ts5 = Time.now();
                    break;
      }
      }
      
        // Get a new sensor event
      sensors_event_t event;
      tsl.getEvent(&event);

      //Particle.publish("getting sensor event");

      // Display the results (light is measured in lux)
      if (event.light)
      {
          Serial.print(event.light); Serial.print(" lux");
          lux = event.light;
          Particle.publish("lux", String(event.light));
          Serial.println("      ...publish successful.");
      }
      else
      {
          /* If event.light = 0 lux the sensor is probably saturated
              and no reliable data could be generated! */
          Serial.println("Sensor overload");
          Particle.publish("Sensor overload");
          lux = 0;
      }
      switch (currentTens_place) {
            case 0: L0 = lux;
                    break;
            case 1: L1 = lux;
                    break;
            case 2: L2 = lux;
                    break;
            case 3: L3 = lux;
                    break;
            case 4: L4 = lux;
                    break;
            case 5: L5 = lux;
                    break;
      }

      // Individual Temp/Humidity calls
      //Serial.print("Temp: "); Serial.print(hdc.readTemperature());
      //Serial.print("\t\tHum: "); Serial.println(hdc.readHumidity());

      //read Temp and Humidity from HDC1008 on I2C bus with a single read
      hdc.ReadTempHumidity();     // one conversion, one read version
      ambientTempF = hdc.GetTemperature() * 1.8 + 32.0;
      ambientHumidity = hdc.GetHumidity();
      Serial.print("\tAmbientTempC: "); Serial.print(hdc.GetTemperature());
      Serial.print("\tAmbientTempF: "); Serial.print(ambientTempF);
      Serial.print("\tAmbientHumidity: "); Serial.print(hdc.GetHumidity());
      Serial.print("\tBatteryLOW: ");
      if (hdc.batteryLOW()) Serial.println("TRUE");
      else Serial.println("FALSE");

      Particle.publish("ambientTempF & ambientHumidity", String(ambientTempF) + String(ambientHumidity));

      switch (currentTens_place) {
        case 0: aT0 = ambientTempF;
              break;
        case 1: aT1 = ambientTempF;
              break;
        case 2: aT2 = ambientTempF;
              break;
        case 3: aT3 = ambientTempF;
              break;
        case 4: aT4 = ambientTempF;
              break;
        case 5: aT5 = ambientTempF;
              break;
      }

      switch (currentTens_place) {
        case 0: aH0 = ambientHumidity;
              break;
        case 1: aH1 = ambientHumidity;
              break;
        case 2: aH2 = ambientHumidity;
              break;
        case 3: aH3 = ambientHumidity;
              break;
        case 4: aH4 = ambientHumidity;
              break;
        case 5: aH5 = ambientHumidity;
              break;
      }


      LastReading = millis();
  }

  // Read the next available 1-Wire temperature sensor
  if (sensor.read()) {
    // Do something cool with the temperature
    Serial.printf("Temperature %.2f C %.2f F ", sensor.celsius(), sensor.fahrenheit());
    Particle.publish("temperature", String(sensor.fahrenheit()), PRIVATE);

    uint8_t addr[8];
    sensor.addr(addr);
    sprintf(ROMCODE, "%02X%02X%02X%02X%02X%02X%02X%02X",
    addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]
    );

    const char greenhouseROM[] = "2874722A070000A5";
    const char waterROM[] = "285DE726070000F2";

      if (strcmp(ROMCODE,greenhouseROM) == 0) {
        greenhouseTemp = sensor.fahrenheit();

        switch(currentTens_place) {
            case 0: gT0 = greenhouseTemp;
                    break;
            case 1: gT1 = greenhouseTemp;
                    break;
            case 2: gT2 = greenhouseTemp;
                    break;
            case 3: gT3 = greenhouseTemp;
                    break;
            case 4: gT4 = greenhouseTemp;
                    break;
            case 5: gT5 = greenhouseTemp;
                    break;
        }
      //return;
      } else if (strcmp(ROMCODE,waterTemp) == 0) {
        waterTemp = sensor.fahrenheit();

        switch (currentTens_place) {
          case 0: wT0 = waterTemp;
                break;
          case 1: wT1 = waterTemp;
                break;
          case 2: wT2 = waterTemp;
                break;
          case 3: wT3 = waterTemp;
                break;
          case 4: wT4 = waterTemp;
                break;
          case 5: wT5 = waterTemp;
                break;
        }
        //return;
      } else if (strcmp(ROMCODE,heattankROM) == 0) {
        heattankTemp = sensor.fahrenheit();

        switch (currentTens_place) {
          case 0: hT0 = heattankTemp;
                break;
          case 1: hT1 = heattankTemp;
                break;
          case 2: hT2 = heattankTemp;
                break;
          case 3: hT3 = heattankTemp;
                break;
          case 4: hT4 = heattankTemp;
                break;
          case 5: hT5 = heattankTemp;
                break;
      }
      //return;
      } else {
        Particle.publish("New ROMCODE", ROMCODE);
      }
    
  
    // Additional info useful while debugging
    printDebugInfo();

  // If sensor.read() didn't return true you can try again later
  // This next block helps debug what's wrong.
  // It's not needed for the sensor to work properly
  } else {
    // Once all sensors have been read you'll get searchDone() == true
    // Next time read() is called the first sensor is read again
    if (sensor.searchDone()) {
      Serial.println("No more addresses.");
      // Avoid excessive printing when no sensors are connected
      delay(250);

    // Something went wrong
    } else {
      printDebugInfo();
    }
  }
  Serial.println();

  if ((CurrentMillis - LastPublish) > PublishFrequency) {
    //*********************************
    // format the sensor data as JSON, so it can be easily parsed
    sprintf(googleString, "{\"lux\":%f,\"waterTemp\":%.2f,\"greenhouseTemp\":%.2f,\"heattankTemp\":%.2f,\"ambientTemp\":%.2f,\"ambientHumidity\":%f,\"timestamp\":%ld}", lux, waterTemp, greenhouseTemp, heattankTemp, ambientTempF, ambientHumidity, Time.now());
    Particle.publish("sensor_data_toGCP", googleString);

    LastPublish = millis();
  }
}

void printDebugInfo() {
  // If there's an electrical error on the 1-Wire bus you'll get a CRC error
  // Just ignore the temperature measurement and try again
  if (sensor.crcError()) {
    Serial.print("CRC Error ");
  }

  // Print the sensor type
  const char *type;
  switch(sensor.type()) {
    case WIRE_DS1820: type = "DS1820"; break;
    case WIRE_DS18B20: type = "DS18B20"; break;
    case WIRE_DS1822: type = "DS1822"; break;
    case WIRE_DS2438: type = "DS2438"; break;
    default: type = "UNKNOWN"; break;
  }
  Serial.print(type);

  // Print the ROM (sensor type and unique ID)
  uint8_t addr[8];
  sensor.addr(addr);
  Serial.printf(
    " ROM=%02X%02X%02X%02X%02X%02X%02X%02X",
    addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]
  );

  //char ROMCODE[10];
  sprintf(ROMCODE, "%02X%02X%02X%02X%02X%02X%02X%02X",
    addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]
  );

  const char greenhouseROM[] = "2874722A070000A5";
  const char waterROM[] = "285DE726070000F2";
  //const char heattankROM[] = "";
//  sprintf(greenhouseROM, "%02X%02X%02X%02X%02X%02X%02X%02X",
//    0x28, 0x74, 0x72, 0x2A, 0x07, 0x00, 0x00, 0xA5
//  );
  //ROMCODE == "2874722A070000A5\0"
  if (strcmp(ROMCODE,greenhouseROM) == 0) {
      greenhouseTemp = sensor.fahrenheit();
      Particle.publish("greenhouseTemp", String(sensor.fahrenheit()));
  } else if (strcmp(ROMCODE,waterROM) == 0) {
      waterTemp = sensor.fahrenheit();
      Particle.publish("waterTemp", String(sensor.fahrenheit()));
  } else if (strcmp(ROMCODE,heattankROM) == 0) {
      heattankTemp = sensor.fahrenheit();
      Particle.publish("heattankTemp", String(sensor.fahrenheit()));
  } else {
      //Particle.publish("debug", greenhouseTemp);
      Particle.publish("new ROMCODE", ROMCODE);
  }


  char ds18String[64];
  sprintf(ds18String,
    " ROM=%02X%02X%02X%02X%02X%02X%02X%02X: %.2f",
    addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7], sensor.fahrenheit()
  );
  Particle.publish("DS18B20 Probe Temperature", ds18String);

  // Print the raw sensor data
  uint8_t data[9];
  sensor.data(data);
  Serial.printf(
    " data=%02X%02X%02X%02X%02X%02X%02X%02X%02X",
    data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8]
  );
}
