/* Ric_Dallas.ino = Sketch to read hardcoded DS18B20 sensors by their HEX sensor code and call them from the loop() by their names...
Created by @Ric here: https://community.particle.io/t/multiple-ds18b20-with-photon/13332/68?u=fidel

These are the ROM codes (+ HEX format) of my 8 test-sensors: (Starting with 10=DS18S20, with 28=DS18B20)
Sensor# 1	= 28ABE11C070000FD = 0x28,0xAB,0xE1,0x1C,0x07,0x00,0x00,0xFD (DS18B20 Waterproof wired sensor #1)
Sensor# 2	= 28233E2A070000C6 = 0x28,0x23,0x3E,0x2A,0x07,0x00,0x00,0xC6 (DS18B20 Waterproof wired sensor #2)
Sensor# 3	= 2854EDF6004 = 0x28,0x54,0xED,0xF,0x06,0x00,0x00,0x04 (DS18B20 Waterproof wired sensor #3)
Sensor# 4	= 28FF219F611503F9 = 0x28,0x4F,0x72,0x2A,0x07,0x00,0x00,0xB0 (DS18B20 Waterproof wired sensor #4)
//Sensor# 5	= 28FF166B00160308 = 0x28,0xFF,0x16,0x6B,0x00,0x16,0x03,0x08 (DS18B20 Waterproof wired sensor #5)
//Sensor# 6	= 28FF90A200160476 = 0x28,0xFF,0x90,0xA2,0x00,0x16,0x04,0x76 (DS18B20 Waterproof wired sensor #6)
Sensor# 7	= 28EFE86F03000019 = 0x28,0xEF,0xE8,0x6F,0x03,0x00,0x00,0x19 (DS18B20 Waterproof wired sensor #7)
Sensor# 8	= 285DE726070000F2 = 0x28,0x5D,0xE7,0x26,0x07,0x00,0x00,0xF2 (DS18B20 Waterproof wired sensor #8)
*/

#include "Dallas.h"
// Choose one of 2 possibilities:
// 1) For use in the IDE:
//#include "OneWire/OneWire.h"

// 2) For use with Particle Dev:
#include "OneWire.h"

double celsius;
double fahrenheit;

// Group sensors in their bus (on one I/O pin) ; [6][8] means: 6 sensor codes with 8 bytes.
// Attention: If the 2 types of sensors are mixed, you must group DS18S20 sensors in one bus and DS18B20 in another bus. You must adapt below code (if(select == 0 or 1)) to the sensor type...
byte addrs0[6][8] = {{0x28,0xAB,0xE1,0x1C,0x07,0x00,0x00,0xFD}, {0x28,0x23,0x3E,0x2A,0x07,0x00,0x00,0xC6}, {0x28,0x54,0xED,0xF,0x06,0x00,0x00,0x04}, {0x28,0x4F,0x72,0x2A,0x07,0x00,0x00,0xB0}, {0x28,0xFF,0x16,0x6B,0x00,0x16,0x03,0x08}, {0x28,0xFF,0x90,0xA2,0x00,0x16,0x04,0x76}};// 6x DS18B20 Waterproof wired sensors
byte addrs1[3][8] = {{0x28,0xEF,0xE8,0x6F,0x03,0x00,0x00,0x19}, {0x28,0x5D,0xE7,0x26,0x07,0x00,0x00,0xF2}, {0x28, 0xD, 0xD3, 0xE2, 0x3, 0x0, 0x0, 0xEE}};// 2x DS18B20 waterproof greenhouse sensors


void getTemperatures(float temps[], int tempsCount, int pin, int select)
{
    OneWire ds = OneWire(pin);
    ds.reset();
    ds.skip();          // Make all devices start the temperature conversion
    ds.write(0x44, 0);  // Tell it to start a conversion (second argument: 1 = parasite power, 0 = powered mode)

    delay(1000);       //  Wait 1 sec for conversion
    ds.reset();

    for (int i=0; i<tempsCount; i++)
    {
        switch (select)
        {
            case 0:
                ds.select(addrs0[i]);
                break;
            case 1:
                ds.select(addrs1[i]);
                break;
        }

        ds.write(0xBE,0);

        byte data0 = ds.read();
        byte data1 = ds.read();
        ds.reset();

        /*
        // If you mix both types of sensors, you need this branching:
        if (select == 0)
        {
            int16_t raw = (data1 << 8) | data0;
            celsius = (float)raw * 0.0625; // DS18B20 sensor = 12 bit resolution device
        }
        else if (select == 1)
        {
            int16_t raw = data0;
            celsius = (float)raw * 0.5; // DS18S20 sensor = 9 bit resolution device
        }
        */

        // If you use only DS18B20 sensors, you can keep it simpler:
        int16_t raw = (data1 << 8) | data0;
        celsius = (float)raw * 0.0625; // For DS18B20 sensors

        fahrenheit = celsius * 1.8 + 32.0;
        temps[i] = fahrenheit;

    }
}
