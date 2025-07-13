/******************************************************************************
u-blox_GNSS.h
u-blox GNSS Arduino
Leonardo Bispo
Mar 03, 2019
https://github.com/ldab/u-blox_GNSS

Distributed as-is; no warranty is given.
******************************************************************************/

// Enable Serial debbug on Serial UART to see registers wrote
#define GNSS_DEBUG Serial

#include "si5351.h"
#include "Wire.h"
#include "Adafruit_MCP4725.h"
#include "ublox_GNSS.h"
#include <SoftwareSerial.h>


Si5351 si5351;
Adafruit_MCP4725 dac;
int dac_value; 



SoftwareSerial Serial_GNSS(10, 11); // RX, TX
float lat, lon, acc;
uint32_t iTOW;
int32_t clkB; 
int32_t clkD; 
uint32_t tAcc; 
uint32_t fAcc;
fixType_t fix = NO_FIX;

GNSS gnss( Serial_GNSS );

void setup() {
  // put your setup code here, to run once:
  Serial.begin(57600);
  dac.begin(0x61);
  dac_value = 2047;
  dac.setVoltage(dac_value,false);

  // Set up a new SoftwareSerial object
  SoftwareSerial gpsSerial =  SoftwareSerial(10, 11);
  gpsSerial.begin(9600);


  // Initialize the Si5351 to use a 10 MHz clock on the XO input
  si5351.init(SI5351_CRYSTAL_LOAD_0PF, 0, 0);

  // Set the CLKIN reference frequency to 10 MHz
  si5351.set_ref_freq(10000000UL, SI5351_PLL_INPUT_XO);

  // Apply a correction factor to CLKIN
  si5351.set_correction(0, SI5351_PLL_INPUT_XO);

  // Set PLLA and PLLB to use the signal on CLKIN instead of the XTAL
  si5351.set_pll_input(SI5351_PLLA, SI5351_PLL_INPUT_XO);
  si5351.set_pll_input(SI5351_PLLB, SI5351_PLL_INPUT_XO);

  si5351.set_ms_source(SI5351_CLK0,SI5351_PLLA);
  si5351.set_ms_source(SI5351_CLK1,SI5351_PLLB);
  si5351.set_ms_source(SI5351_CLK2,SI5351_PLLA);

  // Set CLK0 to output 14 MHz
  //si5351.set_freq(2500000000ULL, SI5351_CLK0);
  si5351.set_freq_manual(2500000000ULL, 80000000000ULL, SI5351_CLK0);
  si5351.set_freq_manual(2600000000ULL, 78000000000ULL, SI5351_CLK1);
  si5351.set_freq_manual(4000000000ULL, 80000000000ULL, SI5351_CLK2);

  si5351.update_status();
  delay(500);
  
  Serial.println("GNSS serial connected");
  Serial_GNSS.begin( 9600 );

  if( gnss.init(  ) )
  {
    Serial.println("\nGNSS initialized.");
  }
  else
  {
    Serial.println("\nFailed to initialize GNSS module.");
  }

}

void loop()
{
  char Chr;
  int zz;
  // Get coordinates with minimum 100m accuracy;
  Serial.println("Get location");

  if ( gnss.getCoodinates(lon, lat, fix, acc, 100) == 0) 
  {
    Serial.println("Failed to get coordinates, check signal, accuracy required or wiring");
  }
  if (fix!=FIX_3D)
  {
    Serial.println("NO FIX !!");
  }
  else
  {
    Serial.println("\nHere you are, lon:" + String(lon, 7) +" lat:" + String(lat, 7));
    Serial.println("calculated error: " + String(acc) + "m");  
    Serial.println("\nOr try the following link to see on google maps:");
    Serial.println(String("https://www.google.com/maps/search/?api=1&query=") + String(lat,7) + "," + String(lon,7));

    if ( gnss.getNavClock(iTOW, clkB, clkD, tAcc, fAcc) == 0) 
    {
      Serial.println("Failed to get coordinates, check signal, accuracy required or wiring");
    }
    Serial.println("\nClock bias:" + String(clkB) +" Clock drift:" + String(clkD));
    
    Serial.println(clkD);

    if (clkD < 0 && clkD >-10)
    {
      dac_value++;
      Serial.println(dac_value); 
      dac.setVoltage(dac_value,false);
    }
    else if (clkD > 0 && clkD < 10)
    {
      dac_value--;
      Serial.println(dac_value); 
      dac.setVoltage(dac_value,false);
    } 
    else
    {
      dac_value -= clkD/5;
       Serial.println(dac_value); 
      dac.setVoltage(dac_value,false);
    }
  }
  if(Serial.available() > 0)
  {
      Chr = Serial.read();
      switch(Chr) 
    {
      case '+' :
        dac_value++;
        Serial.println(dac_value); 
        dac.setVoltage(dac_value,false);
        break;

      case '-' :
        dac_value--;
        Serial.println(dac_value); 
        dac.setVoltage(dac_value,false);
        break; 
      
      case 'v':
        zz=Serial.parseInt();
        if (zz>=0 && zz<=4097)
        {
          dac_value=zz;
          Serial.println(dac_value); 
          dac.setVoltage(dac_value,false);
        }
        break;

    }
  }
  delay(1000);

}