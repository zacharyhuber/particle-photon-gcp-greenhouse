/*
 * Project particle-gcp-greenhouse
 * Description: Greenhouse environment monitor based on Particle devices and Google Cloud Platform for data storage and analysis
 * Author: Zack Huber
 * Date: Feb 21, 2019
 */
// Product ID and Version for Particle Product firmware deployment
//PRODUCT_ID(????); // Argon version using DeviceOS v0.9.0
//PRODUCT_VERSION(1);

// Semi-Automatic Mode allows collection of data without a network connection.
// Particle.connect() will block the rest of the application code until a connection to Particle Cloud is established.
//SYSTEM_MODE(SEMI_AUTOMATIC);

// ************************** STARTUP CALLS ***********************************
    // ALL STARTUP() CODE SHOULD BE WITHIN ONE STARTUP(FUNCTION).
    //=========================================================================
// set relay I/O pins to LOW as soon as possible on reset to avoid dangerous condition where all relays are ON
STARTUP( initialize_solar_heater_relays() );

    //=========================================================================
// This makes sure the Photon is using the best wireless signal.
//~photon code~STARTUP(WiFi.selectAntenna(ANT_AUTO));
    //=========================================================================
// reset the system after 60 seconds if the application is unresponsive
//ApplicationWatchdog wd(120000, System.reset);
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
    /********* Retained variables are not available on the Argon *****************
     * ***** HOWEVER, the simulated EEPROM should work just fine: ****************
     * a double variable is 8 bytes
     * a float variable is 4 bytes
     * a bool variable is 1 byte
     * uint32_t variable is 4 bytes
     * // EXAMPLE USAGE
        // Write a value (2 bytes in this case) to the EEPROM address
        int addr = 10;
        uint16_t value = 12345;
        EEPROM.put(addr, value);

        // Write an object to the EEPROM address
        addr = 20;
        struct MyObject {
          uint8_t version;
          float field1;
          uint16_t field2;
          char name[10];
        };
        MyObject myObj = { 0, 12.34f, 25, "Test!" };
        EEPROM.put(addr, myObj);

// This enables retained variables in the Backup RAM (SRAM) 
STARTUP(System.enableFeature(FEATURE_RETAINED_MEMORY));


//       When adding new retained variables to an existing set of retained variables,
//       it's a good idea to add them after the existing variables.
//       This ensures the existing retained data is still valid even with the new code.

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

retained double gT0;
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

retained float bV3_0;
retained float bV3_1;
retained float bV3_2;
retained float bV3_3;
retained float bV3_4;
retained float bV3_5;

retained uint16_t bV12_0;
retained uint16_t bV12_1;
retained uint16_t bV12_2;
retained uint16_t bV12_3;
retained uint16_t bV12_4;
retained uint16_t bV12_5;
*/

double L0;
double L1;
double L2;
double L3;
double L4;
double L5;

double wT0;
double wT1;
double wT2;
double wT3;
double wT4;
double wT5;

double gT0;
double gT1;
double gT2;
double gT3;
double gT4;
double gT5;

double hT0;
double hT1;
double hT2;
double hT3;
double hT4;
double hT5;

double aT0;
double aT1;
double aT2;
double aT3;
double aT4;
double aT5;

double aH0;
double aH1;
double aH2;
double aH3;
double aH4;
double aH5;

uint32_t ts0;
uint32_t ts1;
uint32_t ts2;
uint32_t ts3;
uint32_t ts4;
uint32_t ts5;

float bV3_0;
float bV3_1;
float bV3_2;
float bV3_3;
float bV3_4;
float bV3_5;

uint16_t bV12_0;
uint16_t bV12_1;
uint16_t bV12_2;
uint16_t bV12_3;
uint16_t bV12_4;
uint16_t bV12_5;

    //=========================================================================

/*
I/O setup: 
These made it easy to just 'plug in' my 18B20

D3 - 1-wire ground, or just use regular pin and comment out below.
D4 - 1-wire signal, 2K-10K resistor to D5 (3v3)
D5 - 1-wire power, ditto ground comment.
*/
#include "DS18.h"

DS18 sensor(D6);
    //=========================================================================


/*
I/O setup:
These made it easy to just 'plug in' my I2C sensors

D2 - I2C ground, or just use regular pin and comment out below.
D3 - I2C power, ditto ground comment.
*/
#include "Adafruit_HDC1000.h"

Adafruit_HDC1000 hdc = Adafruit_HDC1000();
    //=========================================================================

    //=========================================================================

#include "adafruit-ina219.h"

Adafruit_INA219 ina219;

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
   Connect SCL to D1 //different pins on Particle Photon
   Connect SDA to D0 //different pins on Particle Photon
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

bool I2C_sensors_finished = false;
bool ONEWIRE_sensors_finished = false;

    //=========================================================================

//const int PublishFrequency = 1200000; // gcp upload frequency
//unsigned long LastPublish = 20000;

char googleString[255]; // sensor_data_toGCP JSON string, 255 bytes max for Particle.publish data object (622 bytes after DeviceOS v0.8.0)
    //=========================================================================

// Variable Declarations
double waterTemp = 0;
double greenhouseTemp = 0;
double heattankTemp = 0;
double ambientTempC = 0;
double ambientTempF = 0;
double ambientHumidity = 0;
float lux = 0; // As specified for event.light by Unified Sensor Library
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
  //~photon code~Particle.publish("Sensor: ", sensor.name, 60, PRIVATE);
  //~photon code~delay(500);
}

/**************************************************************************/
/*
    Configures the gain and integration time for the TSL2561
*/
/**************************************************************************/
void configureSensor(void)
{
  /* You can also manually set the gain or enable auto-gain support */
  //tsl.setGain(TSL2561_GAIN_1X);      /* No gain ... use in bright light to avoid sensor saturation */
  // tsl.setGain(TSL2561_GAIN_16X);     /* 16x gain ... use in low light to boost sensitivity */
  tsl.enableAutoRange(true);            /* Auto-gain ... switches automatically between 1x and 16x */

  /* Changing the integration time gives you better sensor resolution (402ms = 16-bit data) */
  // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);      /* fast but low resolution */
  // tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_101MS);  /* medium resolution and speed   */
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);  /* 16-bit data but slowest conversions */

  /* Update these values depending on what you've set above! */
  Serial.println("------------------------------------");
  Serial.print  ("Gain:         "); Serial.println("1x gain");
  Serial.print  ("Timing:       "); Serial.println("402 ms");
  Serial.println("------------------------------------");
}


/**************************************************************************/
/*
    Set up System.sleep timing to wake up at the next 10 minute mark: x:00, x:10, x:20, x:30, x:40, x:50.
*/
/**************************************************************************/

int nextTEN = 42;
int wakeupHour = 42;
long wakeSeconds = 42;
//bool goodnight = FALSE;  // Use this only if the Particle.publish calls are being cut off by sleep.


void set_wake_time(void)  
{
    int currentHour = Time.hour();
    Serial.print("currentHour is ");
    Serial.println(currentHour);
    int currentMinute = Time.minute();
    int currentTEN = currentMinute / 10;
    // this should be set up as a series of else if statements, probably
    if (currentTEN == 5) {
        nextTEN = 0;
        if (currentHour == 23) {
            wakeupHour = 0;
        }
        else { wakeupHour = currentHour + 1; }
    }
    else {
        nextTEN = currentTEN + 1;
        wakeupHour = currentHour;
    }
    int wakeupMinute = nextTEN * 10;

    Serial.print("Wake-up time is: ");
    Serial.print(wakeupHour);
    Serial.print(":");
    Serial.println(wakeupMinute);
    // ********* Removed for cellular data efficiency**************/ Particle.publish("Wake-up time is: ", String(wakeupHour) + ":" + String(wakeupMinute), PRIVATE);

    int wakeTime = wakeupMinute - currentMinute;
    if (wakeupMinute == 0) { wakeTime = 60 - currentMinute; }

    Serial.print("Minutes to wake-up: ");
    Serial.println(wakeTime);
    // ********* Removed for cellular data efficiency**************/ Particle.publish("Minutes to wake-up: ", String(wakeTime), PRIVATE);
    //Serial.println("Particle.publish sent");
    wakeSeconds = wakeTime * 60;
}


/**************************************************************************/
/*
    Turn on (then off) voltage divider to read 12v Battery Voltage
*/
/**************************************************************************/
#define vDividerONpin D13
#define vDividerOFFpin D8
#define vDividerREADpin A5

int read12vBatteryVoltage(void)
{
    //pinMode(vDividerONpin, OUTPUT); // moved to STARTUP()
    //pinMode(vDividerOFFpin, OUTPUT);
    //pinMode(vDividerREADpin, INPUT);
    digitalWrite(vDividerONpin, HIGH);
    delay(200);
    digitalWrite(vDividerONpin, LOW);
    delay(500);
    float batteryReading12v = analogRead(vDividerREADpin);
    batteryReading12v = (batteryReading12v + analogRead(vDividerREADpin)) / 2;
    batteryReading12v = (batteryReading12v + analogRead(vDividerREADpin)) / 2;
    batteryReading12v = (batteryReading12v + analogRead(vDividerREADpin)) / 2;
    batteryReading12v = (batteryReading12v + analogRead(vDividerREADpin)) / 2;
    digitalWrite(vDividerOFFpin, HIGH);
    delay(200);
    digitalWrite(vDividerOFFpin, LOW);
    return batteryReading12v;
}


/**************************************************************************/
/*
    Monitor 12v battery for high voltage and turn on solar dump load to use excess photovoltaic power
        and turn off solar dump load when battery voltage drops.
*/
/**************************************************************************/
#define relay0pin D2
#define relay1pin D3
#define relay2pin D4
#define relay3pin D5

//Called in STARTUP() function
void initialize_solar_heater_relays() {
    pinMode(relay0pin, OUTPUT);
    pinMode(relay1pin, OUTPUT);
    pinMode(relay2pin, OUTPUT);
    pinMode(relay3pin, OUTPUT);

    digitalWrite(relay0pin, LOW);
    digitalWrite(relay1pin, LOW);
    digitalWrite(relay2pin, LOW);
    digitalWrite(relay3pin, LOW);

    pinMode(vDividerONpin, OUTPUT);
    pinMode(vDividerOFFpin, OUTPUT);

    digitalWrite(vDividerONpin, LOW);
    digitalWrite(vDividerOFFpin, LOW);
}

#define Time_for_SolarHeater_ON 16 //10:00 AM CST (4:00 PM UTC)
#define Time_for_SolarHeater_OFF 22 //3:00 PM CST (9:00 PM UTC) //DEBUG CHANGE BACK TO "21" IMMEDIATELY
#define MAX_SolarHeater_ON_Time 240000 // in millis
#define Supercap_Charging_Period 30000 // in millis THIS SHOULD BE REPLACED WITH A CURRENT MONITOR ON THE SUPERCAPACITOR
#define Battery12v_Recovery_Period 120000 // in millis THIS SHOULD BE REPLACED WITH A CAREFUL VOLTAGE_BASED ACCOUNTING OF BATTERY HEALTH

bool testingSolarCharger = false;
bool testingLowBattery = false;
bool testingLowLight = false;
bool solarHeaterON = false;

void test_of_Solar_Charger() {
    //if (read12vBatteryVoltage() > 3475) { // ~13.0v This could have better tests, including a "float" LED signal from the solar charge controller.
    if (analogRead(vDividerREADpin) > 2700) { // ~13.0v with diode-skewed GND // DEBUG switched vDividerON earlier in flow

        testingSolarCharger = false;
        solarHeaterON = true;
        
        //digitalWrite(vDividerONpin, HIGH); // DEBUG moved to earlier in flow
        //delay(200); // DEBUG moved to earlier in flow
        //digitalWrite(vDividerONpin, LOW); // DEBUG moved to earlier in flow

        // Serial.println("Solar Heater ON"); // ***DEBUG Serial and Particle.publish calls will crash the application code in Timer callback functions*** 
        
    } else {
        Particle.publish("debug test_of_Solar_Charger failed", PRIVATE);
        //Particle.process();  // just to make sure. REMOVE IF PUBLISH WORKS
        //delay(4000); // DEBUG
        digitalWrite(vDividerOFFpin, HIGH);
        delay(200);
        digitalWrite(vDividerOFFpin, LOW);
        delay(500);
    }
}
Timer testingSolarTimer(10000, test_of_Solar_Charger, true); // Should maybe have two or three timers with success flags for each.


void test_of_Low_Battery_Voltage() {
    //if (read12vBatteryVoltage() < 3200) { // ~12.0v
    if (read12vBatteryVoltage() < 2400) { // ???~12.0v??? <12.9v with diode-skewed GND
        testingLowBattery = false;
        solarHeaterON = false;
        digitalWrite(vDividerOFFpin, HIGH);
        delay(200);
        digitalWrite(vDividerOFFpin, LOW);
        Particle.publish("Solar Heater OFF due to low battery", PRIVATE, NO_ACK);
        Serial.println("Solar Heater OFF due to low battery");
    }
}
Timer testingLowBatteryTimer(10000, test_of_Low_Battery_Voltage, true);


void test_of_Low_Light_Level() {
    if (lux < 800) {
        testingLowLight = false;
        solarHeaterON = false;
        digitalWrite(vDividerOFFpin, HIGH);
        delay(200);
        digitalWrite(vDividerOFFpin, LOW);
        Particle.publish("Solar Heater OFF due to low light levels", PRIVATE, NO_ACK);
        Serial.println("Solar Heater OFF due to low light levels");
    }
}
Timer testingLowLightTimer(300000, test_of_Low_Light_Level, true);


bool reset_BatteryRecoveryTimer = false;

bool debug_battery_recovery_Timer_is_running = false; // DEBUG BatteryRecoveryTimer.isActive() keeps returning false while Timer is running
void transition_from_Battery_Recovery_out() {
    debug_battery_recovery_Timer_is_running = false;
    int highestReading12vBattery = analogRead(vDividerREADpin);
    //if (highestReading12vBattery < 3400) {
    if (highestReading12vBattery < 2650) { // <13.0v with diode-skewed GND
        Particle.publish("12v Battery did not recover fully during Recovery Period", String(highestReading12vBattery), PRIVATE);
        reset_BatteryRecoveryTimer = true;
    }
}
Timer BatteryRecoveryTimer(Battery12v_Recovery_Period, transition_from_Battery_Recovery_out, true);


bool debug_solar_heater_Timer_is_running = false; // DEBUG SolarHeaterTimer.isActive() keeps returning false while Timer is running
void transition_from_Solar_Heater_to_Battery_Recovery() {
    debug_solar_heater_Timer_is_running = false;
    digitalWrite(relay1pin, LOW);
    delay(100);
    int lowestReading12vBattery = analogRead(vDividerREADpin);
    //if (lowestReading12vBattery < 3250) {
    if (lowestReading12vBattery < 2350) { // ??? 11.5v ??? with diode-skewed GND
        Particle.publish("12v battery voltage very low under load from Solar Heater", String(lowestReading12vBattery), PRIVATE);
    }
    BatteryRecoveryTimer.start();
    debug_battery_recovery_Timer_is_running = true;
}
Timer SolarHeaterTimer(MAX_SolarHeater_ON_Time, transition_from_Solar_Heater_to_Battery_Recovery, true);


bool debug_supercap_charger_Timer_is_running = false; // DEBUG the SupercapChargerTimer.isActive() keeps returning false while Timer is running
void transition_from_Supercap_Charger_to_Solar_Heater() {
    debug_supercap_charger_Timer_is_running = false;
    delay(1000); // if Battery Recovery was just called, the Solar Heater relay may not be done switching
    digitalWrite(relay2pin, LOW);
    delay(1000);
    digitalWrite(relay3pin, LOW);
    delay(200); // let relay connections break before connecting high current load
    SolarHeaterTimer.start();
    debug_solar_heater_Timer_is_running = true;
    digitalWrite(relay1pin, HIGH);
    // turn on high power heater, connected to D7
    int priorReading12vBattery = analogRead(vDividerREADpin);
    Particle.publish("Supercapacitor Charging timed out.", String::format("{\"Supercapacitor_Voltage\":%.2f,\"12vBattery_Voltage\":%d}", ina219.getBusVoltage_V(), priorReading12vBattery), PRIVATE);
}
Timer SupercapChargerTimer(Supercap_Charging_Period, transition_from_Supercap_Charger_to_Solar_Heater, true);


bool solarHeaterPAUSE = false;
// call this Timer with pause_for_Sensors_Timer.changePeriod((wakeSeconds - 60) * 1000)
void pauseSolarHeater() {
    digitalWrite(relay1pin, LOW);
    digitalWrite(relay2pin, LOW);
    digitalWrite(relay3pin, LOW);
    SupercapChargerTimer.dispose();
    debug_supercap_charger_Timer_is_running = false;
    SolarHeaterTimer.dispose();
    debug_solar_heater_Timer_is_running = false;
    BatteryRecoveryTimer.dispose();
    debug_battery_recovery_Timer_is_running = false;
    digitalWrite(vDividerOFFpin, HIGH);
    delay(200);
    digitalWrite(vDividerOFFpin, LOW);
    solarHeaterPAUSE = true; // This should immediately break the while loop for the solarHeater
    //set_wake_time();
    //System.sleep(SLEEP_MODE_DEEP, wakeSeconds, SLEEP_NETWORK_STANDBY);
    //System.sleep(D8, FALLING, wakeSeconds); // v0.9.0 of DeviceOS does not have a self-terminating SLEEP_MODE_DEEP, so use STOP mode
    //System.reset();
}
// Call this timer with the following function to replace the System.sleep period with a SolarHeater period
//pause_for_Sensors_Timer.changePeriod((wakeSeconds - 60) * 1000);
Timer pause_for_Sensors_Timer(540000, pauseSolarHeater, true);


void turnONsolarHeater(void) 
{
    if (solarHeaterON == false && Time.hour() >= Time_for_SolarHeater_ON && Time.hour() <= Time_for_SolarHeater_OFF && lux > 1000) {
        
        //if (testingSolarCharger == false && read12vBatteryVoltage() > 3475) { // ~13.0v
        if (testingSolarCharger == false && read12vBatteryVoltage() > 2650) { // ~12.9v with diode-skewed GND
            testingSolarCharger = true;
            testingSolarTimer.start();
            delay(500); // let vDivider relay demagnetize from read12vBatteryVoltage call
            digitalWrite(vDividerONpin, HIGH);
            delay(200);
            digitalWrite(vDividerONpin, LOW);
            delay(500);
            Particle.publish("Solar Heater waiting...", PRIVATE, NO_ACK);
            Serial.println("Solar Heater waiting...");
            
            // Other stuff to do when preparing to start the solar heater

        }
        if (testingSolarTimer.isActive()) {
            // Anything that needs to be done while testing Solar Charger voltage.
            return;
        }

    } else {
        Particle.publish("turnOnsolarHeater conditions not met", PRIVATE); //debug
        // do anything that should be done while solarHeater is ON. (like provide a reason for no activation)
    }

}

void turnOFFsolarHeater(void)
{
    if (solarHeaterON == true && Time.hour() > Time_for_SolarHeater_OFF) {
        solarHeaterON = false;
        digitalWrite(vDividerOFFpin, HIGH);
        delay(200);
        digitalWrite(vDividerOFFpin, LOW);
        Particle.publish("Solar Heater OFF to charge battery for the night", PRIVATE, WITH_ACK);
        Serial.println("Solar Heater OFF to charge battery for the night");
    }
    if (!testingLowLightTimer.isActive()) {
        if (solarHeaterON == true && lux < 800) {
            testingLowLight = true;
            testingLowLightTimer.start();
        }
    }
    if (!testingLowBatteryTimer.isActive()) {
        if (solarHeaterON == true && read12vBatteryVoltage() < 3300) { // ~12.3v
            testingLowBattery = true;
            testingLowBatteryTimer.start();
        }
    }

}


/**************************************************************************/
/*
    Retries the Particle.publish(sensor_data_toGCP) twice more while in a "pause" sleep before a SLEEP_MODE_DEEP/reset.
*/
/**************************************************************************/
void retry_publish(void) 
{
        set_wake_time();
        //~electron code~System.sleep(A7, FALLING, (wakeSeconds / 3), SLEEP_NETWORK_STANDBY);
        System.sleep(D8, FALLING, (wakeSeconds / 3));
        
        Serial.print("attempting to resend sensor_data_toGCP");
        if (Particle.publish("sensor_data_toGCP", googleString, 60, PRIVATE, WITH_ACK)) {
                Serial.println(" Success!");
                return;
        }
        Serial.print("...retry #1 failed");

        //~electron code~System.sleep(A7, FALLING, (wakeSeconds / 3), SLEEP_NETWORK_STANDBY);
        System.sleep(D8, FALLING, (wakeSeconds / 3));

        Serial.print("...last attempt to resend sensor_data_toGCP");
        
        if (Particle.publish("sensor_data_toGCP", googleString, 60, PRIVATE, WITH_ACK)) {
                Serial.println(" Success!");
                return;
        }
        Serial.print(" FAILED TO SEND sensor_data_toGCP");

        // Final failure code goes here ///
}

void setup()
{
  Serial.begin(9600);
  // Set up 'power' pins, comment out if not used! (Set up as I2C power pins on current Particle Photon board)
  //pinMode(D2, OUTPUT);
  //pinMode(D3, OUTPUT);
  //digitalWrite(D2, LOW);
  //digitalWrite(D3, HIGH);

  //pinMode(relay0pin, OUTPUT);
  //pinMode(relay1pin, OUTPUT);
  //pinMode(relay2pin, OUTPUT);
  //pinMode(relay3pin, OUTPUT); //MOVED TO STARTUP() FUNCTION

  //pinMode(vDividerONpin, OUTPUT);
  //pinMode(vDividerOFFpin, OUTPUT); // MOVED TO STARTUP() FUNCTION
  pinMode(vDividerREADpin, INPUT);

  // ****** PLACED INA219 ~.publish HERE TO CHECK WHAT IS GOING ON WITH RELAYS TRIGGERING PRIOR TO setup() ****
  // Initialize the INA219.
  // By default the initialization will use the largest range (32V, 2A).  However
  // you can call a setCalibration function to change this range (see comments in example .ino).
  ina219.begin();
  //if(ina219.getBusVoltage_V() == 0) // This isn't how this should work...
  //{
  //    Serial.print("No INA219 detected ... Check your wiring or I2C ADDR!");
  //    Particle.publish("Ooops, no INA219 detected ... Check your wiring or I2C ADDR!", PRIVATE); //REMOVE FROM setup() for SEMI-AUTOMATIC particle.connect control
  //}
  //Particle.publish("INA219 connected. Reading supercapacitor: ", String::format("current(mA): %.2f / voltage: %.2f", ina219.getCurrent_mA(), ina219.getBusVoltage_V()));
  // ************************** MOVE TO BELOW pin SETUP *******************************************************

  //digitalWrite(relay0pin, LOW);
  //digitalWrite(relay1pin, LOW);
  //digitalWrite(relay2pin, LOW);
  //digitalWrite(relay3pin, LOW); //MOVED TO STARTUP() FUNCTION
  digitalWrite(vDividerONpin, LOW);
  digitalWrite(vDividerOFFpin, LOW);



  Serial.println("Light Sensor Test"); Serial.println("");
  //~photon code~Particle.publish("Testing TSL2561"); //REMOVE FROM setup() for semi-automatic particle.connect control

  // Initialise the sensor
  if(!tsl.begin())
  {
    // There was a problem detecting the ADXL345 ... check your connections
    Serial.print("Ooops, no TSL2561 detected ... Check your wiring or I2C ADDR!");
    Particle.publish("Ooops, no TSL2561 detected ... Check your wiring or I2C ADDR!", PRIVATE); //REMOVE FROM setup() for SEMI-AUTOMATIC particle.connect control
  }

  // Display some basic information on this sensor
  displaySensorDetails();

  // Setup the sensor gain and integration time
  configureSensor();



  Serial.println("HDC100x test");

  if(!hdc.begin())
  {
      Serial.print("No HDC1000 detected ...  Check your wiring or I2C ADDR!");
      Particle.publish("No HDC1000 detected ...  Check your wiring or I2C ADDR!", PRIVATE);
  }
  delay(15);    // let the chip initialize

}

void loop()
{
  //delay(5000); // 5 second pause provides window to manually begin a OTA flash remotely.  Should use Particle Product instead.
  int currentTens_place = (Time.minute() / 10);

  //if (currentTens_place == 5) {
  if (Time.hour() == 9 /*&& currentTens_place == 5*/) {
      Particle.connect();
      waitUntil(Particle.connected);
      Particle.process();
      delay(420000);
      //Particle.publish("No OTA update", PRIVATE);
  }

  //unsigned long CurrentMillis = millis();
  //if ((CurrentMillis - LastReading) > SensorFrequency)
  if (I2C_sensors_finished == false)
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
      //}
      
        // Get a new sensor event
      sensors_event_t event;
      tsl.getEvent(&event);

      //Particle.publish("getting sensor event");

      // Display the results (light is measured in lux)
      if (event.light)
      {
          Serial.print(event.light); Serial.print(" lux");
          lux = event.light;
          //~photon code~Particle.publish("lux", String(event.light), 60, PRIVATE);
          Serial.println("      ...publish successful.");
      }
      else
      {
          /* If event.light = 0 lux the sensor is probably saturated
              and no reliable data could be generated! */
          Serial.println("Sensor overload");
          //~photon code~Particle.publish("Sensor overload", PRIVATE);
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

      //~photon code~Particle.publish("ambientTempF & ambientHumidity", String(ambientTempF) + String(ambientHumidity), 60, PRIVATE);

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


      I2C_sensors_finished = true;
      //LastReading = millis();
  }

  // Read the next available 1-Wire temperature sensor
  if (sensor.read()) {
    // Do something cool with the temperature
    Serial.printf("Temperature %.2f C %.2f F ", sensor.celsius(), sensor.fahrenheit());
    //Particle.publish("temperature", String(sensor.fahrenheit()), PRIVATE);

    uint8_t addr[8];
    sensor.addr(addr);
    sprintf(ROMCODE, "%02X%02X%02X%02X%02X%02X%02X%02X",
    addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]
    );

    const char greenhouseROM[] = "2874722A070000A5";
    const char waterROM[] = "285DE726070000F2";
    const char heattankROM[] = "28F1FA29070000D9";

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
      } else if (strcmp(ROMCODE,waterROM) == 0) {
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
        //Particle.connect();
        Particle.publish("New ROMCODE", ROMCODE, 60, PRIVATE, WITH_ACK);
      }
    
  
    // Additional info useful while debugging
    //printDebugInfo();

  // If sensor.read() didn't return true you can try again later
  // This next block helps debug what's wrong.
  // It's not needed for the sensor to work properly
  } else {
    // Once all sensors have been read you'll get searchDone() == true
    // Next time read() is called the first sensor is read again
    if (sensor.searchDone()) {
      Serial.println("No more addresses.");
      ONEWIRE_sensors_finished = true;
      // Avoid excessive printing when no sensors are connected
      delay(250);

    // Something went wrong
    } else {
      printDebugInfo();
    }
  }
  Serial.println();

  /*
  if (I2C_sensors_finished == true && ONEWIRE_sensors_finished == true) {
      //FuelGauge fuel; //Electron code
      //float batteryVoltage3v = fuel.getVCell(); //Electron code
      float batteryVoltage3v = analogRead(BATT) * 0.0011224;

      switch (currentTens_place) {
          case 0: bV3_0 = batteryVoltage3v;
                break;
          case 1: bV3_1 = batteryVoltage3v;
                break;
          case 2: bV3_2 = batteryVoltage3v;
                break;
          case 3: bV3_3 = batteryVoltage3v;
                break;
          case 4: bV3_4 = batteryVoltage3v;
                break;
          case 5: bV3_5 = batteryVoltage3v;
                break;
      }
  }
  */

  // Conditional statement for Publishing to Google Cloud Platform
  if (ONEWIRE_sensors_finished == true && I2C_sensors_finished == true && ((currentTens_place == 1) || (currentTens_place == 3) || (currentTens_place == 5))) {

      //FuelGauge fuel; //Electron code
      //float batteryVoltage3v = fuel.getVCell(); //Electron code
      float batteryVoltage3v = analogRead(BATT) * 0.0011224; //Argon code
      
      switch (currentTens_place) {
          case 0: bV3_0 = batteryVoltage3v;
                break;
          case 1: bV3_1 = batteryVoltage3v;
                break;
          case 2: bV3_2 = batteryVoltage3v;
                break;
          case 3: bV3_3 = batteryVoltage3v;
                break;
          case 4: bV3_4 = batteryVoltage3v;
                break;
          case 5: bV3_5 = batteryVoltage3v;
                break;
      }

      //float batteryVoltage12v = ((read12vBatteryVoltage() / 4095) *  //14.2); // Would need to determine exact voltages of battery for the given divider and analog input.
      int batteryReading12v = read12vBatteryVoltage();

      switch (currentTens_place) {
          case 0: bV12_0 = batteryReading12v;
                break;
          case 1: bV12_1 = batteryReading12v;
                break;
          case 2: bV12_2 = batteryReading12v;
                break;
          case 3: bV12_3 = batteryReading12v;
                break;
          case 4: bV12_4 = batteryReading12v;
                break;
          case 5: bV12_5 = batteryReading12v;
                break;
      }

  //if ((CurrentMillis - LastPublish) > PublishFrequency) {
    //*********************************
    // format the sensor data as JSON, so it can be easily parsed
    //sprintf(googleString, "{\"lux\":%f,\"waterTemp\":%.2f,\"greenhouseTemp\":%.2f,\"heattankTemp\":%.2f,\"ambientTemp\":%.2f,\"ambientHumidity\":%f,\"3vBattery\":%f,\"timestamp\":%ld}", lux, waterTemp, greenhouseTemp, heattankTemp, ambientTempF, ambientHumidity, batteryVoltage3v, Time.now());
    switch (currentTens_place) {
      case 0: sprintf(googleString, "{\"lux\":%f,\"waterTemp\":%.2f,\"greenhouseTemp\":%.2f,\"heattankTemp\":%.2f,\"ambientTemp\":%.2f,\"ambientHumidity\":%f,\"3vBattery\":%f,\"12vBattery\":%d,\"timestamp\":%ld}", lux, waterTemp, greenhouseTemp, heattankTemp, ambientTempF, ambientHumidity, batteryVoltage3v, batteryReading12v, Time.now());
              break;
      case 1: sprintf(googleString, "{\"ts0\":%ld,\"L0\":%.0f,\"wT0\":%.2f,\"gT0\":%.2f,\"hT0\":%.2f,\"aT0\":%.2f,\"aH0\":%.0f,\"bV3_0\":%f,\"bV12_0\":%d,\"ts1\":%ld,\"L1\":%.0f,\"wT1\":%.2f,\"gT1\":%.2f,\"hT1\":%.2f,\"aT1\":%.2f,\"aH1\":%.0f,\"bV3_1\":%f,\"bV12_1\":%d}", ts0, L0, wT0, gT0, hT0, aT0, aH0, bV3_0, bV12_0, ts1, L1, wT1, gT1, hT1, aT1, aH1, bV3_1, bV12_1);
              break;
      case 2: sprintf(googleString, "{\"lux\":%f,\"waterTemp\":%.2f,\"greenhouseTemp\":%.2f,\"heattankTemp\":%.2f,\"ambientTemp\":%.2f,\"ambientHumidity\":%f,\"3vBattery\":%f,\"12vBattery\":%d,\"timestamp\":%ld}", lux, waterTemp, greenhouseTemp, heattankTemp, ambientTempF, ambientHumidity, batteryVoltage3v, batteryReading12v, Time.now());
              break;
      case 3: sprintf(googleString, "{\"ts2\":%ld,\"L2\":%.0f,\"wT2\":%.2f,\"gT2\":%.2f,\"hT2\":%.2f,\"aT2\":%.2f,\"aH2\":%.0f,\"bV3_2\":%f,\"bV12_2\":%d,\"ts3\":%ld,\"L3\":%.0f,\"wT3\":%.2f,\"gT3\":%.2f,\"hT3\":%.2f,\"aT3\":%.2f,\"aH3\":%.0f,\"bV3_3\":%f,\"bV12_3\":%d}", ts2, L2, wT2, gT2, hT2, aT2, aH2, bV3_2, bV12_2, ts3, L3, wT3, gT3, hT3, aT3, aH3, bV3_3, bV12_3);
              break;
      case 4: sprintf(googleString, "{\"lux\":%f,\"waterTemp\":%.2f,\"greenhouseTemp\":%.2f,\"heattankTemp\":%.2f,\"ambientTemp\":%.2f,\"ambientHumidity\":%f,\"3vBattery\":%f,\"12vBattery\":%d,\"timestamp\":%ld}", lux, waterTemp, greenhouseTemp, heattankTemp, ambientTempF, ambientHumidity, batteryVoltage3v, batteryReading12v, Time.now());
              break;
      case 5: sprintf(googleString, "{\"ts4\":%ld,\"L4\":%.0f,\"wT4\":%.2f,\"gT4\":%.2f,\"hT4\":%.2f,\"aT4\":%.2f,\"aH4\":%.0f,\"bV3_4\":%f,\"bV12_4\":%d,\"ts5\":%ld,\"L5\":%.0f,\"wT5\":%.2f,\"gT5\":%.2f,\"hT5\":%.2f,\"aT5\":%.2f,\"aH5\":%.0f,\"bV3_5\":%f,\"bV12_5\":%d}", ts4, L4, wT4, gT4, hT4, aT4, aH4, bV3_4, bV12_4, ts5, L5, wT5, gT5, hT5, aT5, aH5, bV3_5, bV12_5);
              break;
      default: //Particle.connect();
                Particle.publish("Batch timing error", NULL, PRIVATE);
              break;
    }

    I2C_sensors_finished = false;
    ONEWIRE_sensors_finished = false; 
    // only two of these strings can fit in the 255 bytes that Particle.publish is limited to.
    // Figure out how to fit more data in a single publish!  --oh, oh, I know! DeviceOS v0.8.0 supports up to 622 bytes of data!
    //sprintf(googleString, "{\"ts0\":%ld,\"L0\":%.0f,\"wT0\":%.2f,\"gT0\":%.2f,\"hT0\":%.2f,\"aT0\":%.2f,\"aH0\":%.0f,\"ts1\":%ld,\"L1\":%.0f,\"wT1\":%.2f,\"gT1\":%.2f,\"hT1\":%.2f,\"aT1\":%.2f,\"aH1\":%.0f}", ts0, L0, wT0, gT0, hT0, aT0, aH0, ts1, L1, wT1, gT1, hT1, aT1, aH1);
    
    //Particle.connect();
    if (Particle.publish("sensor_data_toGCP", googleString, 60, PRIVATE, WITH_ACK)) {
            set_wake_time();
            Serial.print("sensor_data_toGCP published successfully");
            Serial.println("  ...Going to Sleep");
            /*
            if ((Time.hour() == 10) && (currentTens_place == 0)) {
                // Give DeviceOS time to check for and download firmware updates (at 10:00 --maybe should add a check for solar power and battery voltage)
                delay(60000); 
            }
            */
           if (batteryVoltage3v < 3.56) {
               Particle.publish("Low Battery... sleeping for 1 hour", String(batteryVoltage3v), PRIVATE);
               Particle.process();
               delay(3000);
               //System.sleep(SLEEP_MODE_DEEP, 3600000);
               System.sleep(D8, FALLING, 3600000); // v0.9.0 of DeviceOS does not have a self-terminating SLEEP_MODE_DEEP, so use STOP mode
               System.reset(); // Added to make sure reset occurs if System.sleep(SLEEP_MODE_DEEP) is not implemented in system firmware.
           }
            //System.sleep(SLEEP_MODE_DEEP, wakeSeconds, SLEEP_NETWORK_STANDBY);
            System.sleep(D8, FALLING, wakeSeconds); // v0.9.0 of DeviceOS does not have a self-terminating SLEEP_MODE_DEEP, so use STOP mode
            waitUntil(Particle.connected);
            Particle.publish("waking up from sleep... system resetting", PRIVATE, WITH_ACK);
            delay(4000); //debug This .publish isn't working
            System.reset(); // Added to make sure reset occurs if System.sleep(SLEEP_MODE_DEEP) is not implemented in system firmware.
    } else {
            Particle.publish("Error sending sensor_data to GCP", NULL, 120, PRIVATE, NO_ACK);
            retry_publish();
            set_wake_time();
            //System.sleep(SLEEP_MODE_DEEP, wakeSeconds, SLEEP_NETWORK_STANDBY);
            System.sleep(D8, FALLING, wakeSeconds); // v0.9.0 of DeviceOS does not have a self-terminating SLEEP_MODE_DEEP, so use STOP mode
            System.reset(); // Added to make sure reset occurs if System.sleep(SLEEP_MODE_DEEP) is not implemented in system firmware.
    }
    
     // Particle.connect();
     // Particle.publish("sensor_data_toGCP", googleString, 60, PRIVATE, WITH_ACK);

    //LastPublish = millis();
  }

  if (I2C_sensors_finished == true && ONEWIRE_sensors_finished == true) {

          //FuelGauge fuel; //Electron code
          //float batteryVoltage3v = fuel.getVCell(); //Electron code
          float batteryVoltage3v = analogRead(BATT) * 0.0011224; //Argon code
      
          switch (currentTens_place) {
               case 0: bV3_0 = batteryVoltage3v;
                  break;
               case 1: bV3_1 = batteryVoltage3v;
                  break;
               case 2: bV3_2 = batteryVoltage3v;
                  break;
               case 3: bV3_3 = batteryVoltage3v;
                  break;
               case 4: bV3_4 = batteryVoltage3v;
                  break;
               case 5: bV3_5 = batteryVoltage3v;
                  break;
          }

          //float batteryVoltage12v = ((read12vBatteryVoltage() / 4095) *  //14.2); // Would need to determine exact voltages of battery for the given divider and analog input.
          float batteryReading12v = read12vBatteryVoltage();

           switch (currentTens_place) {
                case 0: bV12_0 = batteryReading12v;
                    break;
                case 1: bV12_1 = batteryReading12v;
                    break;
                case 2: bV12_2 = batteryReading12v;
                    break;
                case 3: bV12_3 = batteryReading12v;
                    break;
                case 4: bV12_4 = batteryReading12v;
                    break;
                case 5: bV12_5 = batteryReading12v;
                    break;
          }

          I2C_sensors_finished = false;
          ONEWIRE_sensors_finished = false;
          
          //if (batteryReading12v > 3475 && lux > 1000) { // ~13.0v
          if (batteryReading12v > 2700 && lux > 1000) { // ~13.0v with diode-skewed GND
              delay(1000); // DEBUG rest period between calls to read12vBatteryVoltage() to allow relay coils to deenergize
              turnONsolarHeater();
              Particle.publish("debug turnONsolarHeater", PRIVATE);
              delay(4000); // DEBUG rest period to make sure any .publish calls from turn ON process go through
          } else if (lux == 0.00) {
              Particle.publish("Possible TSL2561 Oversaturation", String(lux), PRIVATE);
          }
          //if (batteryReading12v < 3200 || lux < 800 || Time.hour() > Time_for_SolarHeater_OFF) { // ~12.0v
          if (batteryReading12v < 2400 || lux < 800 || Time.hour() > Time_for_SolarHeater_OFF) { // ??? 12.0v ??? with diode-scewed GND
              turnOFFsolarHeater();
          }

          if (testingSolarCharger == true || testingLowLight == true || testingLowBattery == true) {
              // THIS MIGHT BE BETTER AS A SERIES OF FOR LOOPS TO LIMIT THE CHANCE FOR BLOCKING
              while (testingSolarTimer.isActive()) {
                  // ******** DEBUG CODE **********
                  delay(1000);
                  Particle.publish("testingSolarTimer.isActive", PRIVATE);
                  Particle.process();
                  delay(1000);
                  // ****** END DEBUG CODE ********
                  Particle.process();
              }
              while (testingLowLightTimer.isActive()) {
                  Particle.process();
              }
              while (testingLowBatteryTimer.isActive()) {
                  Particle.process();
              }
          }

          while (solarHeaterON == true && solarHeaterPAUSE == false) {
              //Particle.process();
              /******** DEBUG CODE **********
              Particle.publish("debug YOU HAVE REACHED THE WHILE LOOP", PRIVATE);
              Particle.process();
              delay(5000);
              // ****** END DEBUG CODE ********/
              if (!pause_for_Sensors_Timer.isActive()) {
                  set_wake_time();
                  if (wakeSeconds < 60) {
                      Particle.publish("Not enough time to start Solar Heating Period, going to sleep", PRIVATE);
                      // ******** DEBUG CODE **********
                      Particle.process();
                      delay(1000);
                      // ****** END DEBUG CODE ********
                      break;
                  }
                  pause_for_Sensors_Timer.changePeriod((wakeSeconds - 60) * 1000);
                  Particle.publish("Solar Heating Period starting. Time remaining:", String(wakeSeconds - 60), PRIVATE);
                  // ******** DEBUG CODE **********
                  Particle.process();
                  delay(1000);
                  // ****** END DEBUG CODE ********
              }

              int currentReading12vBattery = analogRead(vDividerREADpin);
              //if (currentReading12vBattery < 3200) { // ~12.0v
              if (currentReading12vBattery < 2200) { // ??? 11.5v ??? with diode-skewed GND // DEBUG this shouldn't have triggered...
                  digitalWrite(relay1pin, LOW);
                  delay(500);
                  digitalWrite(relay2pin, LOW);
                  delay(500);
                  digitalWrite(relay3pin, LOW);
                  pause_for_Sensors_Timer.dispose();
                  SupercapChargerTimer.dispose();
                  debug_supercap_charger_Timer_is_running = false;
                  SolarHeaterTimer.dispose();
                  debug_solar_heater_Timer_is_running = false;
                  BatteryRecoveryTimer.dispose();
                  debug_battery_recovery_Timer_is_running = false;
                  //debugging publish, remove if this works:
                  Particle.publish("Very low battery under load from Solar Heater", PRIVATE);
                  // ******** DEBUG CODE **********
                  Particle.process();
                  delay(1000);
                  // ****** END DEBUG CODE ********

                  break;
                  // this break uses the system.sleep below this while loop
              } // else if (currentReading12vBattery > 3400) {}

              if (reset_BatteryRecoveryTimer == true) {
                  reset_BatteryRecoveryTimer = false;
                  BatteryRecoveryTimer.reset();
                  debug_battery_recovery_Timer_is_running = true;
              }

              if (debug_solar_heater_Timer_is_running == false && debug_battery_recovery_Timer_is_running == false) { // DEBUG !SolarHeaterTimer.isActive() and !BatteryRecoveryTimer.isActive() were not evaluating correctly
                  if (debug_supercap_charger_Timer_is_running == false) { // DEBUG !SupercapChargerTimer.isActive() was not evaluating correctly
                      // ******** DEBUG CODE **********
                      Particle.publish("debug ATTEMPTING TO START SUPERCAPACITOR CHARGER", PRIVATE, NO_ACK);
                      Particle.process();
                      delay(4000);
                      // ****** END DEBUG CODE ********
                      digitalWrite(relay3pin, HIGH);
                      delay(1000);
                      digitalWrite(relay2pin, HIGH);
                      SupercapChargerTimer.start();
                      debug_supercap_charger_Timer_is_running = true;
                      delay(200); // avoid reading the peak current into supercapacitor with INA219
                      Particle.publish("Supercapacitor Charger STARTED. Current(mA):", String(ina219.getCurrent_mA()), PRIVATE, NO_ACK);
                      // ******** DEBUG CODE **********
                      Particle.process();
                      delay(1000);
                      // ****** END DEBUG CODE ********
                      continue;
                  } else if (ina219.getCurrent_mA() < 300 && ina219.getCurrent_mA() > -300) {
                      digitalWrite(relay2pin, LOW);
                      delay(1000);
                      digitalWrite(relay3pin, LOW);
                      SupercapChargerTimer.dispose();
                      debug_supercap_charger_Timer_is_running = false;
                      delay(1000); // allow relay connection to break before connecting large load
                      SolarHeaterTimer.start();
                      debug_solar_heater_Timer_is_running = true;
                      digitalWrite(relay1pin, HIGH);
                      // turn on high power heater, connected to D7
                      //int priorReading12vBattery = analogRead(vDividerREADpin); // unused variable was throwing a makeError in compiler
                      Particle.publish("Solar Heating Element STARTED. Supercapacitor current(mA)", String(ina219.getCurrent_mA()), PRIVATE, NO_ACK);
                      // ******** DEBUG CODE **********
                      Particle.process();
                      delay(3000);
                      // ****** END DEBUG CODE ********
                      continue;
                      /********************************************* // Moved into else/if statement
                      if (ina219.getCurrent_mA() < 300 && ina219.getCurrent_mA() > -300) { // DEBUG need to figure out wiring of relays
                          digitalWrite(relay2pin, LOW);
                          delay(1000);
                          digitalWrite(relay3pin, LOW);
                          SupercapChargerTimer.dispose();
                          delay(1000); // allow relay connection to break before connecting large load
                          SolarHeaterTimer.start();
                          digitalWrite(relay1pin, HIGH);
                          // turn on high power heater, connected to D7
                          int priorReading12vBattery = analogRead(vDividerREADpin);
                          Particle.publish("Solar Heating Element STARTED. Supercapacitor current(mA)", String(ina219.getCurrent_mA()), PRIVATE, NO_ACK);
                          // ******** DEBUG CODE **********
                          Particle.process();
                          delay(1000);
                          // ****** END DEBUG CODE ********
                      }
                      **********************************************/
                  } else if (debug_supercap_charger_Timer_is_running == true) { //DEBUG SupercapCharger.isActive() not evaluating correctly
                      // ******** DEBUG CODE **********
                      Particle.publish("debug SupercapCharger.isActive. Current(mA):", String(ina219.getCurrent_mA()), PRIVATE, NO_ACK);
                      Particle.process();
                      delay(1000);
                      // ****** END DEBUG CODE ********
                      continue;
                  } else {
                      // ******** DEBUG CODE **********
                      Particle.publish("debug ERROR in Supercapacitor Charger Timer loop", PRIVATE, WITH_ACK);
                      Particle.process();
                      delay(1000);
                      // ****** END DEBUG CODE ********
                      continue;
                  }
                  /******** DEBUG REMOVE AND PLACE IN ! (NOT) conditional *********
                  if (SupercapChargerTimer.isActive()) {
                      if (ina219.getCurrent_mA() > -300) {  // DEBUG this may need to be reversed " < 300"
                          digitalWrite(relay2pin, LOW);
                          delay(1000);
                          digitalWrite(relay3pin, LOW);
                          SupercapChargerTimer.dispose();
                          delay(1000); // allow relay connection to break before connecting large load
                          SolarHeaterTimer.start();
                          digitalWrite(relay1pin, HIGH);
                          // turn on high power heater, connected to D7
                          int priorReading12vBattery = analogRead(vDividerREADpin);
                          Particle.publish("Solar Heating Element STARTED. Supercapacitor current(mA)", String(ina219.getCurrent_mA()), PRIVATE, NO_ACK);
                          // ******** DEBUG CODE **********
                          Particle.process();
                          delay(1000);
                          // ****** END DEBUG CODE ********
                      }
                  } else {
                      // ******** DEBUG CODE **********
                      Particle.publish("debug ATTEMPTING TO START SUPERCAPACITOR CHARGER", PRIVATE, NO_ACK);
                      Particle.process();
                      delay(5000);
                      // ****** END DEBUG CODE ********
                      digitalWrite(relay3pin, HIGH);
                      delay(1000);
                      digitalWrite(relay2pin, HIGH);
                      SupercapChargerTimer.start();
                      delay(500); // avoid reading the peak current into supercapacitor with INA219
                      Particle.publish("Supercapacitor Charger STARTED. Current(mA):", String(ina219.getCurrent_mA()), PRIVATE, NO_ACK);
                      // ******** DEBUG CODE **********
                      Particle.process();
                      delay(1000);
                      // ****** END DEBUG CODE ********
                  }
                  ********END DEBUG REMOVAL**********/
              }

              if (debug_battery_recovery_Timer_is_running == true) {
                  //if (currentReading12vBattery > 3775) { // >13.5v
                  if (currentReading12vBattery > 3050) { // ???>13.4v??? with diode-skewed GND
                      delay(1000); // if Battery Recovery was just activated, the Solar Heater relay may not be done switching
                      BatteryRecoveryTimer.dispose();
                      debug_battery_recovery_Timer_is_running = false;
                      digitalWrite(relay3pin, HIGH);
                      delay(1000);
                      digitalWrite(relay2pin, HIGH);
                      SupercapChargerTimer.start();
                      debug_supercap_charger_Timer_is_running = true;
                      delay(200); // avoid reading the peak current into supercapacitor with INA219
                      Particle.publish("Battery Recovery complete. Supercapacitor Charger ON. Current(mA):", String(ina219.getCurrent_mA()), PRIVATE, NO_ACK);
                      // ******** DEBUG CODE **********
                      Particle.process();
                      delay(1000);
                      // ****** END DEBUG CODE ********
                  }
              }

              if (debug_solar_heater_Timer_is_running == true) { // DEBUG !SolarHeaterTimer.isActive() was not evaluating correctly
                  //if (currentReading12vBattery < 3350) { // ~12.5v
                  if (currentReading12vBattery < 2450) { // ???~12.5v??? with diode-skewed GND
                      digitalWrite(relay1pin, LOW);
                      SolarHeaterTimer.dispose();
                      debug_solar_heater_Timer_is_running = false;
                      delay(100);
                      int lowestReading12vBattery = analogRead(vDividerREADpin);
                      //if (lowestReading12vBattery < 3250) { // ~12.2v
                      if (lowestReading12vBattery < 2350) { // ???~12.2v??? with diode-skewed GND
                          //debugging publish, remove if this works:
                          Particle.publish("Low battery under load from Solar Heater", String(lowestReading12vBattery), PRIVATE);
                          // ******** DEBUG CODE **********
                          Particle.process();
                          delay(1000);
                          // ****** END DEBUG CODE ********

                          break;
                          // this break uses the system.sleep below this while loop
                      }
                      BatteryRecoveryTimer.start();
                      debug_battery_recovery_Timer_is_running = true;
                      Particle.publish("Battery Recovery STARTED. Supercapacitor Voltage:", String(ina219.getBusVoltage_V()), PRIVATE, NO_ACK);
                      // ******** DEBUG CODE **********
                      Particle.process();
                      delay(1000);
                      // ****** END DEBUG CODE ********
                  } else if (ina219.getCurrent_mA() < 200 && ina219.getCurrent_mA() > -200) {
                      // ******** DEBUG CODE **********
                      Particle.publish("debug Turning Off Solar Heater to Recharge Supercapacitor", String(ina219.getCurrent_mA()), PRIVATE, NO_ACK);
                      Particle.process();
                      delay(1000);
                      // ****** END DEBUG CODE ********
                      //delay(1000); // if relays just switched they may not have demagnetized yet
                      digitalWrite(relay1pin, LOW);
                      SolarHeaterTimer.dispose();
                      debug_solar_heater_Timer_is_running = false;
                      delay(1000);
                      digitalWrite(relay3pin, HIGH);
                      delay(1000);
                      digitalWrite(relay2pin, HIGH);
                      SupercapChargerTimer.start();
                      debug_supercap_charger_Timer_is_running = true;
                      delay(200); // avoid reading the peak current into supercapacitor with INA219
                      Particle.publish("Supercapacitor Charger STARTED. Current(mA):", String(ina219.getCurrent_mA()), PRIVATE, NO_ACK);
                      // ******** DEBUG CODE **********
                      Particle.process();
                      delay(1000);
                      // ****** END DEBUG CODE ********

                  } else {
                      // ******** DEBUG CODE **********
                      Particle.publish("debug SolarHeater.isActive. Current(mA):", String(ina219.getCurrent_mA()), PRIVATE, NO_ACK);
                      Particle.process();
                      delay(1000);
                      // ****** END DEBUG CODE ********
                  }
              }
              

              //long currentMillis = millis();
              /*if (currentMillis - start_of_supercap_chargeMillis > Supercap_Charging_Period) {
                  digitalWrite(relay2pin, LOW);
                  digitalWrite(relay3pin, LOW);
                  delay(100);
                  digitalWrite(relay1pin, HIGH);
                  // turn on high power heater, connected to D7
                  int priorReading12vBattery = analogRead(vDividerREADpin);
                  long start_of_solarHeaterMillis = millis();
              }*/
              /*if (currentMillis - start_of_solarHeaterMillis > MAX_SolarHeater_ON_Time) {
                  digitalWrite(relay1pin, LOW);
                  delay(100);
                  int lowestReading12vBattery = analogRead(vDividerREADpin);
                  long start_of_RecoveryPeriodMillis = millis();
                  if (lowestReading12vBattery < 3250) {
                      break;
                  }
                  //return;
              }*/
              /*if (currentMillis - start_of_RecoveryPeriodMillis > Battery12v_Recovery_Period) {
                  int highestReading12vBattery = analogRead(vDividerREADpin);
                  if (highestReading12vBattery < 3475) {
                      Particle.process();
                      delay(5000);
                  }
                  if (highestReading12vBattery > 3475) {
                      digitalWrite(relay3pin, HIGH);
                      digitalWrite(relay2pin, HIGH);
                      long start_of_supercap_chargeMillis = millis();
                  } else {
                      break;
                  }
                  //digitalWrite(relay3pin, HIGH);
                  //digitalWrite(relay2pin, HIGH);
                  //start_of_supercap_chargeMillis = millis();
              }*/
              /*if (lowestReading12vBattery < 3375 || analogRead(vDividerREADpin) < 3375) {
                  digitalWrite(relay1pin, LOW);
                  delay(100);
                  digitalWrite(relay3pin, HIGH);
                  digitalWrite(relay2pin, HIGH);
                  long start_of_supercap_chargeMillis = millis();
              }*/
              /*if (ina219.getCurrent_mA() > 2500 || ina219.getCurrent_mA() < -2500) {
                  continue;
              }*/
              /*if (currentMillis - start_of_supercap_chargeMillis < Supercap_Charging_Period && ina219.getCurrent_mA() < 300 && ina219.getCurrent_mA() > -300) {
                  //float busvoltage = 0;
                  //float current_mA = 0;
                  //busvoltage = ina219.getBusVoltage_V();
                  //current_mA = ina219.getCurrent_mA();
                  digitalWrite(relay2pin, LOW);
                  digitalWrite(relay3pin, LOW);
                  delay(100);
                  digitalWrite(relay1pin, HIGH);
                  // turn on high power heater, connected to D7
                  int priorReading12vBattery = analogRead(vDividerREADpin);
                  long start_of_solarHeaterMillis = millis();
              }*/
              Particle.process();
              //digitalWrite(relay3pin, HIGH);
              //digitalWrite(relay2pin), HIGH);
          }
          solarHeaterPAUSE = false;

          // all retained variables should have been updated so go to SLEEP_MODE_DEEP until next measurement
          // BUT NOT ON THE ARGON (unless EEPROM storage of variables is implemented)
          set_wake_time();

          
          Serial.println("sensors updated ...going to sleep");
          // 0.9.0 version of Argon DeviceOS does not have backup registers implemented, but:
          //    EEPROM could be used to store variables during STANDBY mode (deep sleep mode) but there is no way to wake up the device at a fixed time.
          // Replacing SLEEP_MODE_DEEP with STOP mode System.sleep().
          //~electron code~System.sleep(SLEEP_MODE_DEEP, wakeSeconds, SLEEP_NETWORK_STANDBY);
          System.sleep(D8, FALLING, wakeSeconds); // v0.9.0 of DeviceOS does not have a self-terminating SLEEP_MODE_DEEP, so use STOP mode

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
  const char heattankROM[] = "28F1FA29070000D9";
//  sprintf(greenhouseROM, "%02X%02X%02X%02X%02X%02X%02X%02X",
//    0x28, 0x74, 0x72, 0x2A, 0x07, 0x00, 0x00, 0xA5
//  );
  //ROMCODE == "2874722A070000A5\0"
  if (strcmp(ROMCODE,greenhouseROM) == 0) {
      greenhouseTemp = sensor.fahrenheit();
      Particle.publish("greenhouseTemp", String(sensor.fahrenheit()), 60, PRIVATE);
  } else if (strcmp(ROMCODE,waterROM) == 0) {
      waterTemp = sensor.fahrenheit();
      Particle.publish("waterTemp", String(sensor.fahrenheit()), 60, PRIVATE);
  } else if (strcmp(ROMCODE,heattankROM) == 0) {
      heattankTemp = sensor.fahrenheit();
      Particle.publish("heattankTemp", String(sensor.fahrenheit()), 60, PRIVATE);
  } else {
      //Particle.publish("debug", greenhouseTemp, PRIVATE);
      Particle.publish("new ROMCODE", ROMCODE, 60, PRIVATE);
  }


  char ds18String[64];
  sprintf(ds18String,
    " ROM=%02X%02X%02X%02X%02X%02X%02X%02X: %.2f",
    addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7], sensor.fahrenheit()
  );
  //~photon code~Particle.publish("DS18B20 Probe Temperature", ds18String, 60, PRIVATE);

  // Print the raw sensor data
  uint8_t data[9];
  sensor.data(data);
  Serial.printf(
    " data=%02X%02X%02X%02X%02X%02X%02X%02X%02X",
    data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8]
  );
}
