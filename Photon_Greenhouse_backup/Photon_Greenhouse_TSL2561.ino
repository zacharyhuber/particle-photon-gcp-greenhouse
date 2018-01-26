/************* The following watchdog timer needs to be incorporated *************
// SYNTAX
// declare a global watchdog instance
ApplicationWatchdog wd(timeout_milli_seconds, timeout_function_to_call, stack_size);

// default stack_size of 512 bytes is used
ApplicationWatchdog wd(timeout_milli_seconds, timeout_function_to_call);
*/
// EXAMPLE USAGE
// reset the system after 60 seconds if the application is unresponsive
//ApplicationWatchdog wd(120000, System.reset);

/*
void loop() {
  while (some_long_process_within_loop) {
    wd.checkin(); // resets the AWDT count
  }
}
// AWDT count reset automatically after loop() ends

***************END OF ADDITIONAL CODE*******************/

STARTUP(WiFi.selectAntenna(ANT_AUTO)); // This makes sure the Photon is using the best wireless signal.

// This #include statement was automatically added by the Particle IDE.
#include "Adafruit_HDC1000.h"

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
   Connect SCL to analog 5
   Connect SDA to analog 4
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

const int SensorFrequency = 2000;  // I2C bus sensor polling frequency
unsigned long LastReading = 2000;

Adafruit_HDC1000 hdc = Adafruit_HDC1000();

//voltage divider set up between 12v battery and pin A2
int twelveVoltBatteryPin = A2;

//photoresitor set up between pins A5 and A0, and a resistor from A5 to GND.  A0 providing smooth 3v3.
//int photocellPin = A0;
//int photocellPower = A1;

//int christmaslightsPin = D7;
//int christmaslightsButton = D6;

//int relay_State = LOW;
//int buttonpush;
//int previous = HIGH;

//long button_timer = 0;
//const long debounce = 200;      // the debounce time, increase if the relay gets 'ticky'

double waterTempC = 0;
double waterTempF = 0;
double ambientTempC = 0;
double ambientTempF = 0;
double ambientHumidity = 0;
int brightness = 0;
int lux = 0;
int voltage12v = 0;
int batt12vSOC = 0;

char adafruitTemp[50] = "oops";
//char adafruitBrightness[5] = "42";
char adafruitAmbient[50] = "42";
char adafruitHumidity[50] = "state change registered";
char adafruitVoltage[50] = "error";

char googleString[64];

<<<<<<< Updated upstream
int deepSleep(String command);
=======
//DEBUG //int deepSleep(String command); **********************************************
>>>>>>> Stashed changes
bool goodnight = FALSE;

//Wake Time variables initialized
int nextTEN = 42;
int wakeupHour = 42;
long wakeSeconds = 42;


// Timer declaration and callback functions for the shutdown timer and publish timer
bool Timer_Running = FALSE;
bool Ubidots_waiting = FALSE;
bool Publish_stream_waiting = TRUE;

void go_to_sleep() {
    Timer_Running = FALSE;
    Particle.publish("Going to sleep");
    Particle.process();
    System.sleep(SLEEP_MODE_DEEP,410);
}

void reset_ubidots() {
    Ubidots_waiting = FALSE;
}

void reset_stream() {
    Publish_stream_waiting = FALSE;
}

Timer sleepTimer(150000, go_to_sleep);
Timer ubidotsTimer(120000, reset_ubidots);
Timer publishStreamTimer(10000, reset_stream);


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
  Serial.print  ("Gain:         "); Serial.println("No gain");
  Serial.print  ("Timing:       "); Serial.println("402 ms");
  Serial.println("------------------------------------");
}


/************************** Dallas - Ric code - NEED TO ATTRIBUTE ********************************/
#include "Dallas.h"

float T1, T2, T3, T4, T5, T6, T7, T8, T9; // The names you want to call the sensors, created in the array(s) in Dallas.cpp

// Group the sensors by which pin they're connected to:
float* temps1[] = {&T1, &T2, &T3, &T4, &T5, &T6}; // Group1: 6 waterproof sensors
float* temps2[] = {&T7, &T8, &T9};                // Group2: 2 sensors on Photon pin A3

void setup()
{
  Serial.begin(9600);

  // register the cloud function for relay control
//  Particle.function("TreeColor", treeLights);

  // Set up 'power' pins for I2C bus, comment out if not used!
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  digitalWrite(D2, LOW);
  digitalWrite(D3, HIGH);

  pinMode(twelveVoltBatteryPin, INPUT);
//  pinMode(photocellPower, OUTPUT);
//  digitalWrite(photocellPower, HIGH);
//  pinMode(photocellPin, INPUT);
//  pinMode(christmaslightsPin, OUTPUT);
//  pinMode(christmaslightsButton, INPUT);

  Serial.println("Light Sensor Test"); Serial.println("");
  Particle.publish("Testing TSL2561");

  // Initialise the sensor
  if(!tsl.begin())
  {
    // There was a problem detecting the ADXL345 ... check your connections
    Serial.print("Ooops, no TSL2561 detected ... Check your wiring or I2C ADDR!");
    Particle.publish("Ooops, no TSL2561 detected ... Check your wiring or I2C ADDR!");
  }

  // Display some basic information on this sensor
  displaySensorDetails();

  // Setup the sensor gain and integration time
  configureSensor();

  //Particle.publish("Sensor ready to go.");

  Serial.println("HDC100x test");

  if(!hdc.begin())
  {
      Serial.print("No HDC1000 detected ...  Check your wiring or I2C ADDR!");
      Particle.publish("No HDC1000 detected ...  Check your wiring or I2C ADDR!");
  }
  delay(15);    // let the chip initialize


  // register the cloud variables
  Particle.variable("waterTemp", waterTempF);
  Particle.variable("ambientTemp", ambientTempF);
  Particle.variable("Humidity", ambientHumidity);
  Particle.variable("Brightness", brightness);
  Particle.variable("Lux", lux);
  Particle.variable("result", googleString);
  Particle.variable("voltage12v", voltage12v);
  Particle.variable("Solar12vSOC", batt12vSOC);
<<<<<<< Updated upstream

  Particle.function("sleep", deepSleep);
  Particle.subscribe("hook-response/ubidots_ambientHumidity", ubidotsRESPONDED, MY_DEVICES);
=======
/**********    DEBUG problems with timing    *************************************
  //Particle.function("sleep", deepSleep);
  ********************************************************************************/

  Particle.subscribe("hook-response/sensor_data_toGCP", gcpRESPONDED, MY_DEVICES);
>>>>>>> Stashed changes
  //delay(3000);
}


void loop()
{

  unsigned long CurrentMillis = millis();

  if ((CurrentMillis - LastReading) > SensorFrequency)
  {
        // Get a new sensor event
      sensors_event_t event;
      tsl.getEvent(&event);

      //Particle.publish("getting sensor event");

      // Display the results (light is measured in lux)
      if (event.light)
      {
          Serial.print(event.light); Serial.print(" lux");
          lux = event.light;
          //Particle.publish("lux", String(event.light));
          Serial.println("      ...publish successful.");
      }
      else
      {
          /* If event.light = 0 lux the sensor is probably saturated
              and no reliable data could be generated! */
          Serial.println("Sensor overload");
          Particle.publish("Sensor overload");
      }


      Serial.print("Temp: "); Serial.print(hdc.readTemperature());
      Serial.print("\t\tHum: "); Serial.println(hdc.readHumidity());

      //read Temp and Humidity from HDC1008 on I2C bus with a single read
      hdc.ReadTempHumidity();     // one conversion, one read version
      ambientTempF = hdc.GetTemperature() * 1.8 + 32.0;
      Serial.print("\tAmbientTempC: "); Serial.print(hdc.GetTemperature());
      Serial.print("\tAmbientTempF: "); Serial.print(ambientTempF);
      Serial.print("\tAmbientHumidity: "); Serial.print(hdc.GetHumidity());
      Serial.print("\tBatteryLOW: ");
      if (hdc.batteryLOW()) Serial.println("TRUE");
      else Serial.println("FALSE");

      ambientHumidity = hdc.GetHumidity();

      LastReading = millis();
  }

  //digitalWrite(voltageMeterPIN, HIGH);  // Turn voltage meter circuit on
  //delay(15);
  voltage12v = analogRead(twelveVoltBatteryPin);
  Serial.print("\t12v Battery Voltage: "); Serial.print(voltage12v);
  batt12vSOC = voltage12v * 100;
  batt12vSOC = batt12vSOC - 300000;
  batt12vSOC = batt12vSOC / 1095;
  if (voltage12v < 3000) {
      Serial.println("BATTERY VOLTAGE TOO LOW");
  }
  Serial.print("    State of Charge: "); Serial.print(batt12vSOC); Serial.println("%");
//  brightness = analogRead(photocellPin);
//  Serial.print("\tBrightness: "); Serial.println(brightness);

  //convert variables to char arrays
//  sprintf(adafruitBrightness, "%f", brightness);
//  Serial.print("\tChar* array for io.adafruit.com brightess: "); Serial.println(adafruitBrightness);

  //convert variables to char arrays
  sprintf(adafruitAmbient, "%f", ambientTempF);
  Serial.print("\tChar* array for io.adafruit.com Ambient Temperature: "); Serial.println(adafruitAmbient);

  //convert variables to char arrays
  sprintf(adafruitHumidity, "%f", ambientHumidity);
  Serial.print("\tChar* array for io.adafruit.com Ambient Humidity: "); Serial.println(adafruitHumidity);


  // -----Here is some temporary code that can be used to control relays-----
  /*
  buttonpush = digitalRead(christmaslightsButton);

  // if the input just went from HIGH to LOW and we've waited long enough
  // to ignore any noise on the circuit, toggle the output pin
  // and remember the time
  if (buttonpush == LOW && previous == HIGH && millis() - button_timer > debounce)  {
    if (relay_State == HIGH) {
        relay_State = LOW;
    }
    else {
        relay_State = HIGH;
    }

    button_timer = millis();

    Serial.print("Relay switch is: ");
    Serial.println(relay_State);
  }

  digitalWrite(christmaslightsPin, relay_State);

  previous = buttonpush;
  */

  /************************** Dallas - Ric code - NEED TO ATTRIBUTE ********************************/

  // Read sensor values! Argument 1: ??? (N° of sensors?) , Argument 2: Particle I/O pin , Argument 3: Select array of addresses (addrs0, addrs1 ...) in Dallas.cpp
  //getTemperatures(*temps1, 6, D6, 0);
  getTemperatures(*temps2, 3, A3, 1);

  Particle.publish("DS18B20 check: T7,T8,T9,T10", String(T7) + String(T8) + String(T9) + String(T10));

  // From now you can use the chosen sensor names as if it were ordinary floating type variables...

  // Here's an example: Using one of the temperatures in a condition: Warm it up >25°C, then prints all temperatures to serial monitor by bus...

  if (T4<200) //print all temperatures to serial monitor by bus...
  {
   Serial.printf("A3 BUS: T7 = %.1f T8 = %.1f", T7, T8);   Serial.printf(" - D6 BUS: T1 = %.1f T2 = %.1f T3 = %.1f T4 = %.1f T5 = %.1f T6 = %.1f", T1, T2, T3, T4, T5, T6);    Serial.println();
  }
  else // Alert!
  {
   Serial.print("Wait a while: T4 > 30C! : "); Serial.println(T4);
  }
  delay(2000); // Minimum 1000 !!!


  //Serial.print("  Temperature = ");
  //Serial.print(celsius);
  //Serial.print(" Celsius, ");
  //Serial.print(fahrenheit);
  //Serial.println(" Fahrenheit");
  //Serial.print("\t\tHum: "); Serial.println(hdc.readHumidity());

  //waterTempF = fahrenheit;

  //convert variables to char arrays
  sprintf(adafruitTemp, "%f", T7);
  Serial.print("\tChar* array for io.adafruit.com: "); Serial.println(adafruitTemp);

  if (Publish_stream_waiting == FALSE && Particle.connected()) {
    Particle.publish("waterTemp", adafruitTemp);
    Particle.publish("greenhouseTemp", String(T8));
    Particle.publish("ambientTemp", adafruitAmbient);
    Particle.publish("Humidity", adafruitHumidity);
    Particle.publish("lux", String(lux));
    Particle.publish("Battery Voltage (4095=3.3v)", String(voltage12v));
    Particle.publish("test of Solar Battery SOC", String(batt12vSOC));  //this is debugging code for the batt12vSOC variable
    publishStreamTimer.start();
    Publish_stream_waiting = TRUE;
  }



//DEBUG - remove when these variables are fixed
  Particle.publish("ambientTempF, ambientHumidity", String(ambientTempF) + ", " + String(ambientHumidity));
  delay(2000);
  // format the sensor data as JSON, so it can be easily parsed
<<<<<<< Updated upstream
  //sprintf(googleString, "{\"waterTemp\":%f,\"ambientTemp\":%f,\"Humidity\":%f}", waterTempF, ambientTempF, ambientHumidity);
=======
  sprintf(googleString, "{\"lux\":%d,\"greenhouseTemp\":%.2f,\"waterTemp\":%.2f,\"ambientTemp\":%.2f,\"ambientHumidity\":%f,\"timestamp\":%d}", lux, T7, T8, ambientTempF, ambientHumidity, Time.now());
//DEBUG - remove if this is sending every time
  Particle.publish("Checking if Particle.connected");
  delay(2000);
  if (Particle.connected()) {
    //DEBUG - remove if this is sending every time
    Particle.publish("Checking if GCP Publish is Ready");
    delay(2000);
    //if (gcp_waiting_to_publish == false) {
      Particle.publish("sensor_data_toGCP", googleString);

      // *********** No longer using Ubidots for data collection and visualization ******
      //Particle.publish("ubidots_solarVoltage", "{ \"ubidots_solarVoltage\": \"" + String(batt12vSOC) + "\" }");
      //Particle.process();
      //delay(1000);
      //Particle.publish("ubidots_lux", "{ \"ubidots_lux\": \"" + String(lux) + "\" }");
      //Particle.process();
      //delay(1000);
      //Particle.publish("ubidots_greenhouseTemp", "{ \"ubidots_greenhouseTemp\": \"" + String(T8) + "\" }");
      //Particle.process();
      //delay(1000);
      //Particle.publish("ubidots_waterTemp", "{ \"ubidots_waterTemp\": \"" + String(T7) + "\" }");
      //Particle.process();
      //delay(1000);
      //Particle.publish("ubidots_ambientTemp", "{ \"ubidots_ambientTemp\": \"" + String(ambientTempF) + "\" }");
      //Particle.process();
      //delay(1000);
      //Particle.publish("ubidots_ambientHumidity", "{ \"ubidots_ambientHumidity\": \"" + String(ambientHumidity) + "\" }");
>>>>>>> Stashed changes

  if (Particle.connected() && Ubidots_waiting == FALSE) {
      Particle.publish("ubidots_solarVoltage", "{ \"ubidots_solarVoltage\": \"" + String(batt12vSOC) + "\" }");
      Particle.process();
<<<<<<< Updated upstream
      delay(1000);
      Particle.publish("ubidots_lux", "{ \"ubidots_lux\": \"" + String(lux) + "\" }");
      Particle.process();
      delay(1000);
      Particle.publish("ubidots_greenhouseTemp", "{ \"ubidots_greenhouseTemp\": \"" + String(T8) + "\" }");
      Particle.process();
      delay(1000);
      Particle.publish("ubidots_waterTemp", "{ \"ubidots_waterTemp\": \"" + String(T7) + "\" }");
      Particle.process();
      delay(1000);
      Particle.publish("ubidots_ambientTemp", "{ \"ubidots_ambientTemp\": \"" + String(ambientTempF) + "\" }");
      Particle.process();
      delay(1000);
      Particle.publish("ubidots_ambientHumidity", "{ \"ubidots_ambientHumidity\": \"" + String(ambientHumidity) + "\" }");
      Particle.process();
      ubidotsTimer.start();
      Ubidots_waiting = TRUE;
=======
      delay(3000); //should be unnecessary
      gcpTimer.start();
      gcp_waiting_to_publish = true;
    //}
    //else {
      Serial.println("Waiting on timer for Google Cloud Platfrom upload...");
    //DEBUG - remove if this is sending every time
      Particle.publish("Waiting on timer for GCP upload...", String(gcp_waiting_to_publish));
      delay(1000); //easy way to stop publish spam
    //}
>>>>>>> Stashed changes
  }


  // Check if the device is connected to the cloud (to make sure it has updated the Particle.variables), then go to sleep
  if (Particle.connected()) {
      if (Timer_Running == FALSE) {
          Particle.publish("Sleep Timer started");
          sleepTimer.start();
          Timer_Running = TRUE;
      }
      else if (Timer_Running == TRUE) {
          //Particle.publish("Sleep Timer already running");
      }
      else {
          Particle.publish("Sleep Timer error");
      }
  }

<<<<<<< Updated upstream
  if (goodnight == TRUE) {
      Particle.publish("Device has been told to sleep: ");
=======
/************** DEBUG timing of sleep functions ********************
  if (goodnight == true) {
      Particle.publish("Device has been told to sleep: ", String(goodnight));
>>>>>>> Stashed changes
      Particle.process();
      go_to_sleep();
  }
  else {
    //DEBUG - remove this whole else statement once timer is fixed ***
    Particle.publish("goodnight boolean is:", String(goodnight));
    Particle.process();
    delay(2000);
  }
  *******************************************************************/

  if (voltage12v > 0) {
    if (voltage12v < 3000) {
<<<<<<< Updated upstream
        Particle.publish("Battery Critically Low - Entering Power Save Mode");
=======
        Particle.publish("Battery Critically Low - Entering Power Save Mode", String(voltage12v));
>>>>>>> Stashed changes
        Particle.process();
        delay(10000);
        System.sleep(SLEEP_MODE_DEEP, 3600);
    }
  }
  else {
<<<<<<< Updated upstream
    Particle.publish("Battery Monitor Circuit Error");
=======
    Particle.publish("Battery Monitor Circuit Error", String(voltage12v));
    delay(2000); // This delay is just an easy way to make sure there aren't too many publish calls
>>>>>>> Stashed changes
  }

}


void set_wake_time()  {
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
    Particle.publish("Wake-up time is: ", String(wakeupHour) + ":" + String(wakeupMinute));

    int wakeTime = wakeupMinute - currentMinute;
    if (wakeupMinute == 0) { wakeTime = 60 - currentMinute; }

    Serial.print("Minutes to wake-up: ");
    Serial.println(wakeTime);
    Particle.publish("Minutes to wake-up: ", String(wakeTime));
    Serial.println("Particle.publish sent");
    wakeSeconds = wakeTime * 60;
}

void ubidotsRESPONDED(const char *name, const char *data) {
    // This is where a handler could process the timestamp of last post to ubidots
    // but this would need to be stored in backup SRAM using a backup battery on VBAT

    set_wake_time();
    /*
    int currentHour = Time.hour();
    int currentMinute = Time.minute();
    int currentTEN = currentMinute / 10;
    if (currentTEN == 5) {
        int nextTEN = 0;
        if (currentHour == 23) {
            currentHour = 0;
        }
        else { currentHour = currentHour + 1; }
    }
    else {
        int nextTEN = currentTEN + 1;
    }
    int wakeupMinute = nextTEN * 10;
    // ***insert code to sleepUntil (currentHour:wakeupMinute)
    // ***also, the above code could be moved to setup(); thus setting the wakeup time during wakeup
    void sleepUntil(time_t wakeupTime) {
     wakeupTime -= Time.local();
     System.sleep(SLEEP_MODE_DEEP, wakeupTime);
    }
    */


    //delay(3000);  //This shouldn't be necessary
<<<<<<< Updated upstream
    Ubidots_waiting = FALSE;  // Comment out if non-publish functions are used.
=======
    gcp_waiting_to_publish = false;
    // ******* DEBUG CODE **********
    Particle.publish("gcpRESPONDED told this device to sleep");
    delay(5000);
    // *****************************
>>>>>>> Stashed changes
    System.sleep(A4,RISING,wakeSeconds);
}

int deepSleep(String command)
{
    goodnight = TRUE;
    //Particle.publish("Device has been told to sleep: ");
    //Particle.publish(command);
    //Particle.process();
    return 1;
}

/* This code didn't work correctly, but a simple toggle function in its own app did work.
int treeLights(String cmd) {
    cmd.trim();
    cmd.toLowerCase();
    Serial.println("Particle Function received");

    if (cmd.startsWith("white")) {
        digitalWrite(christmaslightsPin, HIGH);
        Serial.println("*"); Serial.println("*"); Serial.println("*");
        Serial.println("Relay activated");
        Serial.println("*"); Serial.println("*"); Serial.println("*");
        //int pin = cmd.substring(6).toInt();
        //digitalWrite(pin, HIGH);
    }
    if (cmd.startsWith("color")) {
        digitalWrite(christmaslightsPin, LOW);
    }
}
*/
