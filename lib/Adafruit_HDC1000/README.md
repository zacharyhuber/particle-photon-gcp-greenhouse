# Adafruit HDC1000 Library

_This is a library for the HDC1008 Humidity + Temp sensor_

Designed specifically to work with the HDC1008 breakout in the Adafruit shop

These sensors use I2C to communicate, 2 pins are required to interface
Adafruit invests time and resources providing this open source code,
please support Adafruit and open-source hardware by purchasing
products from Adafruit!

Written by Limor Fried/Ladyada for Adafruit Industries.
BSD license, all text above must be included in any redistribution

Modified for use as a Particle Photon/Electron public v2 library by RMB

## Typical usage

```
#include "Adafruit_HDC1000.h"

// Connect Vin to 3-5VDC
// Connect GND to ground
// Connect SCL to I2C clock pin (D1 on Photon/Electron)
// Connect SDA to I2C data pin  (D0 on Photon/Electron)

Adafruit_HDC1000 hdc = Adafruit_HDC1000();   // create object instance

void setup() {
  // prints to USB port on Photon/Electron   RMB
  Serial.begin(9600);
  Serial.println("HDC100x test");

  hdc.begin();
  delay(15);    // let the chip initialize  
}

void loop() {
  Serial.print("Temp: "); Serial.print(hdc.readTemperature());
  Serial.print("\t\tHum: "); Serial.println(hdc.readHumidity());  
  Serial.print("\tBatteryLOW: ");
  if (hdc.batteryLOW()) Serial.println("TRUE");
  else Serial.println("FALSE");
  delay(500);
}
```
## Examples

- [Read humidity/temp over USB:](examples/HDC1000test/HDC1000test.ino)
  In this example the Particle device sends humidity/temp readings over USB

- [Publish humidity/temp readings:](examples/HDCMonitorExample/HDCMonitor_Example.ino) In this example the Particle device publishes humidity/temp readings in JSON format to be used in ThingSpeak, etc.

- [Web page updating:](examples/html/HDCMonitor_Example.html) In this example html/js code the published humidity/temp readings are used to update on-screen JustGages

- [ThingSpeak webhook:](examples/WebHook/ParticleWebhook_ThingSpeak_CustomJSON) Paste this text into 'custom JSON' when setting up a webhook integration in Particle Console for a ThingSpeak CHANNEL to monitor published humidity/temp readings graphically!

- [README.txt:](examples/MarkDown/README.txt) View the README in MarkDown notation before rendering on GitHub. Based on the spark/PowerShield README as a template.

## Reference

### `Adafruit_HDC1000`
`Adafruit_HDC1000 hdc = Adafruit_HDC1000();`  
Creates and initializes an object to interact with the HDC1008 sensor.

### `begin`
`hdc.begin();`  
Initializes sensor. Should normally be called once in `setup()`.

### `readTemperature`  
`float tempC = hdc.readTemperature();`  
Reads the temperature in Celsius degrees.

### `readHumidity`  
`float humidity = hdc.readHumidity();`  
Reads the humidity in percent.

### `batteryLOW`  
`boolean batteryLOW = hdc.batteryLOW();`  
Added function to test battery voltage. True if < 2.8V.   

## Resource Utilization

Connect Vin to 3-5VDC  
Connect GND to ground  
Connect SCL to I2C clock pin (D1 on Photon/Electron)  
Connect SDA to I2C data pin  (D0 on Photon/Electron)  

## References

- [Adafruit HDC1008 sensor breakout board](https://www.adafruit.com/products/2635)
- [HDC1008 datasheet](http://www.ti.com/lit/ds/symlink/hdc1008.pdf)

## License

Copyright (c) 2015, AdaFruit Industries  
 All rights reserved.  
 Released under the BSD license
