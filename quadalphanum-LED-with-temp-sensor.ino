/*
 * Basic program for a 4-alphanumeric character 14-segment display
 * using an HT16K33 I2C driver chip.
 * Temp sensor is a Maxim Integrated DS18B20 1-wire but the Dallas
 * library supports others as well.
 * 
 * Libraries needed:
 * OneWire
 * DallasTemperature
 * Adafruit LED Backpack
 */
 
//#include <Wire.h>

// for HT16K33 driver and 14-seg display
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4();

// for temp sensor
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 2 // data line from temp sensor
#define FIVE_V_OUT 7
OneWire oneWire( ONE_WIRE_BUS );
DallasTemperature sensors( &oneWire );

long C; // celsius value holder
long F; // fahrenheit value holder

void setup() {
  // give 5V to temp sensor
  pinMode(FIVE_V_OUT, OUTPUT);
  digitalWrite(FIVE_V_OUT, HIGH);
  sensors.begin();
  
  Serial.begin(115200);
  Serial.println("Beginning LED setup...");

  // begin 14-seg quad LED setup...
  alpha4.begin(0x70);  // pass in the address
  alpha4.clear();
  
/*
 * This turns on all segments for each LED for testing.
 * And it looks pretty at power-on.
 */ 
  alpha4.writeDigitRaw(0, 0xFFFF);
  alpha4.writeDisplay();
  delay(200);
  alpha4.writeDigitRaw(0, 0x0);
  alpha4.writeDigitRaw(1, 0xFFFF);
  alpha4.writeDisplay();
  delay(200);
  alpha4.writeDigitRaw(1, 0x0);
  alpha4.writeDigitRaw(2, 0xFFFF);
  alpha4.writeDisplay();
  delay(200);
  alpha4.writeDigitRaw(2, 0x0);
  alpha4.writeDigitRaw(3, 0xFFFF);
  alpha4.writeDisplay();
  delay(200);

  alpha4.clear();
  alpha4.writeDisplay();
} /* end setup() */

void loop() {

  Serial.println();
  Serial.print(" Requesting temperatures... ");
  sensors.requestTemperatures();
  Serial.println("DONE");


  C = (long)sensors.getTempCByIndex(0);
  F = C*9/5 + 32; // I figure it was safer to do conversion than polling again
  
/*
 * 17 Aug 2021 1100hrs
 * Getting 'ERRF' somewhere around a -16C reading. Not sure what's going on
 * so trying to use int C instead to see if that fixes it.
 * 
 * 19 Aug 1400hrs -
 * Switching to 'long ints' solved issue. Still not sure why 'F' was getting garbage
 * data but ints seemed to have solved it.
 */
  // base 10 (long) int to String
  String stringC = String(C);
  String stringF = String(F);
  
  alpha4.clear();
  // is either temp value longer than 3?
  if( ( stringF.length() > 3 ) || ( stringC.length() > 3 ) ){
    writeTooLong(stringF.length() > 3 ? 'F' : 'C');
    alpha4.writeDisplay();
  } else {
    int strlen = stringF.length() - 1;
    alpha4.writeDigitAscii(3, 'F');
    int i = 2; // start at LED position 3 (index 1)
    for( strlen; strlen >= 0; strlen-- ){
      // checks for garbage data
      if( !isDigit(stringF[strlen]) && (strlen > 0) ){
        Serial.print(stringF[strlen]);
        Serial.println("  NOT AN INTEGER for strlen > 0");
        writeErr();
        delay(2000);
        break;
        // checks for '-' or int at position 0 (garbage check)
      } else if( strlen == 0 ) {
          if( stringF[0] != '-' ){
            if( !isDigit(stringF[0]) ) {
              Serial.print(stringF[strlen]);
              Serial.println("  NOT AN INTEGER or NOT '-' for strlen == 0");
              writeErr();
              delay(2000);
              break;
            }
          }
      }     
      alpha4.writeDigitAscii(i, stringF[strlen]);
      i--;
    }
    alpha4.writeDisplay();
    delay(2000);
    alpha4.clear();
    /* END work on stringF */

    /* BEGIN work on stringC */
    strlen = stringC.length() - 1;
    i = 2;
    alpha4.writeDigitAscii(3, 'C');
    for( strlen; strlen >= 0; strlen-- ){
      // checks for garbage data
      // TODO: move this and stringF segment to a function
      if( !isDigit(stringC[strlen]) && (strlen > 0) ){
        Serial.print(stringC[strlen]);
        Serial.println("  NOT AN INTEGER for strlen > 0 in stringC");
        Serial.print("strlen: ");
        Serial.println(strlen);
        writeErr();
        delay(2000);
        break;
      } else if( strlen == 0 ) {
        if( stringC[0] != '-' ){
          if( !isDigit(stringC[0]) ) {
            Serial.print(stringC[strlen]);
            Serial.println("  NOT AN INTEGER or NOT '-' for strlen == 0 in stringC");
            writeErr();
            delay(2000);
            break;
            }
          }
      }
      alpha4.writeDigitAscii(i, stringC[strlen]);
      i--;
    }
    alpha4.writeDisplay();
    delay(2000);
    alpha4.clear();
    /* END work on stringC */
  } /* end } else { */
} /* end loop() */

void writeErr(){
  alpha4.writeDigitAscii(0, 'E');
  alpha4.writeDigitAscii(1, 'R');
  alpha4.writeDigitAscii(2, 'R');
  alpha4.writeDisplay();
}

void writeTooLong(char unit){ // 'unit' is either 'C' or 'F'
  alpha4.clear();
  String str = "TOO LONG ";
  str += unit;
  int strlen = str.length()-1;
  for( int i = 0; i <= strlen; i++ ){
    
    alpha4.writeDigitAscii(0, str[i]);
    // each if/else ensures LED is turned off else
    // LED scrolls and repeats 'unit'
    if( (i+1) > strlen ) alpha4.writeDigitRaw(1, 0x00);
    else alpha4.writeDigitAscii(1, str[i+1]);
    if( (i+2) > strlen ) alpha4.writeDigitRaw(2, 0x00);
    else alpha4.writeDigitAscii(2, str[i+2]);
    if( (i+3) > strlen ) alpha4.writeDigitRaw(3, 0x00);
    else alpha4.writeDigitAscii(3, str[i+3]);
    delay(300);
    alpha4.writeDisplay();
  }
}
