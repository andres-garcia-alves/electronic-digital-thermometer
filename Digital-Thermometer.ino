/*
Digital Thermometer v1.1

Created by: Andres Garcia Alves <andres.garcia.alves@gmail.com>
Microcontroller: Arduino Nano (ATMega328 @16Mhz)

Version 1.0, 2022.12.08 - Initial release.
Version 1.1, 2022.12.10 - Re-arrange LEDs pin-out to simplefy PCB.

This source code is licensed under GPL v3.0

Pinout (4 digit multiplexed 7-segment display):

     ***(A)***
  (F)*       *(B)
     ***(G)***
  (E)*       *(C)
     ***(D)***

D00 -> debug (serial port)
D01 -> debug (serial port)
D02 -> output to F segment of selected digit
D03 -> output to A segment of selected digit
D04 -> output to selector pin for digit 0
D05 -> output to E segment of selected digit
D06 -> output to D segment of selected digit
D07 -> output to dot point of selected digit
D08 -> output to C segment of selected digit
D09 -> output to G segment of selected digit
D10 -> output to selector pin for digit 1
D11 -> output to B segment of selected digit
D12 -> output to selector pin for digit 2
D13 -> output to selector pin for digit 3

*/

#include <DHT.h>

#define DEBUG         false

// digit selector pins
#define PIN_DIGIT_0   4
#define PIN_DIGIT_1   13
#define PIN_DIGIT_2   12
#define PIN_DIGIT_3   10

// display segment pins
#define PIN_LED_A     3
#define PIN_LED_B     11
#define PIN_LED_C     8
#define PIN_LED_D     6
#define PIN_LED_E     5
#define PIN_LED_F     2
#define PIN_LED_G     9
#define PIN_LED_DOT   7

// DHT sensor pin & sensor type
#define PIN_DHT       A4
#define SENSOR_TYPE   DHT11

// data structures
struct SensorMeasure {
  bool valid = false;
  float temperature = 0;
  float humidity = 0;
  float heatIndex = 0;
} sensorMeasure;

struct DisplayText {
  String temperature;
  String humidity;
  String heatIndex;
  byte temperatureDotPos;
  byte humidityDotPos;
  byte heatIndexDotPos;
} displayText;

enum eDataMode { NONE, TEMPERATURE, HUMIDITY, HEAT_INDEX };


// variables
DHT dht(PIN_DHT, SENSOR_TYPE);    // inicializar sensor DHT11

unsigned long int currentMillis = 0;
unsigned long int nextSensorTick = 0;
unsigned long int nextDataModeTick = 0;

eDataMode dataMode = NONE;
byte currentDigit = 0;

// initial setup
void setup() {

  // inicializar comunicacion serie
  #if DEBUG
  Serial.begin(9600);
  #endif

  // inicializar sensor DHT
  dht.begin();

  // modo de los pines
  pinMode(PIN_DIGIT_0, OUTPUT);
  pinMode(PIN_DIGIT_1, OUTPUT);
  pinMode(PIN_DIGIT_2, OUTPUT);
  pinMode(PIN_DIGIT_3, OUTPUT);
  
  pinMode(PIN_LED_A, OUTPUT);
  pinMode(PIN_LED_B, OUTPUT);
  pinMode(PIN_LED_C, OUTPUT);
  pinMode(PIN_LED_D, OUTPUT);
  pinMode(PIN_LED_E, OUTPUT);
  pinMode(PIN_LED_F, OUTPUT);
  pinMode(PIN_LED_G, OUTPUT);
  pinMode(PIN_LED_DOT, OUTPUT);

  delay(300);
}


// main loop
void loop() {
  
  currentMillis = millis();

  // sensor reading
  if (currentMillis > nextSensorTick) {
    nextSensorTick = currentMillis + 15000; // next sensor read in 15 secods

    sensorMeasure = readSensor();

    if (sensorMeasure.valid) {
      displayText = buildDisplayText(sensorMeasure);
    }      

    #if DEBUG
    updateSerialPort(sensorMeasure);
    #endif
  }

  // data mode
  if (currentMillis > nextDataModeTick) {
    nextDataModeTick = currentMillis + 5000; // change mode in 5 seconds

    switch (dataMode) {
      case NONE:        dataMode = TEMPERATURE; break;
      case TEMPERATURE: dataMode = HUMIDITY;    break;
      case HUMIDITY:    dataMode = HEAT_INDEX;  break;
      case HEAT_INDEX:  dataMode = TEMPERATURE; break;
    }
  }

  // update display & move to next digit
  if (sensorMeasure.valid) {
    updateDisplay(dataMode, currentDigit);
    currentDigit = (++currentDigit) % 4; // valid range 0-3
  }

  delay(5);
}


SensorMeasure readSensor() {

  SensorMeasure aux;
  aux.valid = true;
  aux.temperature = dht.readTemperature();  // temperatura (en grados centígrados)
  aux.humidity = dht.readHumidity();        // humedad relativa

  // comprobar que no haya habido algún error en la lectura
  if (isnan(aux.temperature) || isnan(aux.humidity)) {
    aux.valid = false;

    #if DEBUG
    Serial.println("Error obteniendo los datos del sensor DHT11");
    #endif
  }

  // calcular la sensación térmica (en grados centígrados)
  aux.heatIndex = dht.computeHeatIndex(aux.temperature, aux.humidity, false);

  return aux;
}


DisplayText buildDisplayText(SensorMeasure sensorMeasure) {

  DisplayText aux;
  aux.temperature = String(sensorMeasure.temperature, 1);
  aux.temperature.replace(".", "");
  aux.temperature += "c";
  aux.temperatureDotPos = 1;
  
  aux.humidity = String(sensorMeasure.humidity, 1);
  aux.humidity.replace(".", "");
  aux.humidity.setCharAt(2, " ");
  aux.humidity += "h";
  aux.humidityDotPos = -1;
  
  aux.heatIndex = String(sensorMeasure.heatIndex, 1);
  aux.heatIndex.replace(".", "");
  aux.heatIndex += "C";  
  aux.heatIndexDotPos = 1;

  return aux;
}


void updateDisplay(eDataMode dataMode, byte currentDigit) {
  char character = ' ';
  bool dotEnabled = false;

  // enable current digit
  digitalWrite(PIN_DIGIT_0, (currentDigit == 0) ? HIGH : LOW);
  digitalWrite(PIN_DIGIT_1, (currentDigit == 1) ? HIGH : LOW);
  digitalWrite(PIN_DIGIT_2, (currentDigit == 2) ? HIGH : LOW);
  digitalWrite(PIN_DIGIT_3, (currentDigit == 3) ? HIGH : LOW);

  // retrieve character for the current digit
  if (dataMode == TEMPERATURE) {
    character = displayText.temperature.charAt(currentDigit);
    dotEnabled = (displayText.temperatureDotPos == currentDigit) ? true : false;
  } else if (dataMode == HUMIDITY) {
    character = displayText.humidity.charAt(currentDigit);
    dotEnabled = (displayText.humidityDotPos == currentDigit) ? true : false;
  } else if (dataMode == HEAT_INDEX) {
    character = displayText.heatIndex.charAt(currentDigit);
    dotEnabled = (displayText.heatIndexDotPos == currentDigit) ? true : false;
  }

  // display character
  displayCharacter(character, dotEnabled);
}


void displayCharacter(char character, bool dotEnabled) {

  digitalWrite(PIN_LED_DOT, !dotEnabled);

  if (character == '0') {
    digitalWrite(PIN_LED_A, LOW);
    digitalWrite(PIN_LED_B, LOW);
    digitalWrite(PIN_LED_C, LOW);
    digitalWrite(PIN_LED_D, LOW);
    digitalWrite(PIN_LED_E, LOW);
    digitalWrite(PIN_LED_F, LOW);
    digitalWrite(PIN_LED_G, HIGH);
  } else if (character == '1') {
    digitalWrite(PIN_LED_A, HIGH);
    digitalWrite(PIN_LED_B, LOW);
    digitalWrite(PIN_LED_C, LOW);
    digitalWrite(PIN_LED_D, HIGH);
    digitalWrite(PIN_LED_E, HIGH);
    digitalWrite(PIN_LED_F, HIGH);
    digitalWrite(PIN_LED_G, HIGH);
  } else if (character == '2') {
    digitalWrite(PIN_LED_A, LOW);
    digitalWrite(PIN_LED_B, LOW);
    digitalWrite(PIN_LED_C, HIGH);
    digitalWrite(PIN_LED_D, LOW);
    digitalWrite(PIN_LED_E, LOW);
    digitalWrite(PIN_LED_F, HIGH);
    digitalWrite(PIN_LED_G, LOW);
  } else if (character == '3') {
    digitalWrite(PIN_LED_A, LOW);
    digitalWrite(PIN_LED_B, LOW);
    digitalWrite(PIN_LED_C, LOW);
    digitalWrite(PIN_LED_D, LOW);
    digitalWrite(PIN_LED_E, HIGH);
    digitalWrite(PIN_LED_F, HIGH);
    digitalWrite(PIN_LED_G, LOW);
  } else if (character == '4') {
    digitalWrite(PIN_LED_A, HIGH);
    digitalWrite(PIN_LED_B, LOW);
    digitalWrite(PIN_LED_C, LOW);
    digitalWrite(PIN_LED_D, HIGH);
    digitalWrite(PIN_LED_E, HIGH);
    digitalWrite(PIN_LED_F, LOW);
    digitalWrite(PIN_LED_G, LOW);
  } else if (character == '5') {
    digitalWrite(PIN_LED_A, LOW);
    digitalWrite(PIN_LED_B, HIGH);
    digitalWrite(PIN_LED_C, LOW);
    digitalWrite(PIN_LED_D, LOW);
    digitalWrite(PIN_LED_E, HIGH);
    digitalWrite(PIN_LED_F, LOW);
    digitalWrite(PIN_LED_G, LOW);
  } else if (character == '6') {
    digitalWrite(PIN_LED_A, LOW);
    digitalWrite(PIN_LED_B, HIGH);
    digitalWrite(PIN_LED_C, LOW);
    digitalWrite(PIN_LED_D, LOW);
    digitalWrite(PIN_LED_E, LOW);
    digitalWrite(PIN_LED_F, LOW);
    digitalWrite(PIN_LED_G, LOW);
  } else if (character == '7') {
    digitalWrite(PIN_LED_A, LOW);
    digitalWrite(PIN_LED_B, LOW);
    digitalWrite(PIN_LED_C, LOW);
    digitalWrite(PIN_LED_D, HIGH);
    digitalWrite(PIN_LED_E, HIGH);
    digitalWrite(PIN_LED_F, HIGH);
    digitalWrite(PIN_LED_G, HIGH);
  } else if (character == '8') {
    digitalWrite(PIN_LED_A, LOW);
    digitalWrite(PIN_LED_B, LOW);
    digitalWrite(PIN_LED_C, LOW);
    digitalWrite(PIN_LED_D, LOW);
    digitalWrite(PIN_LED_E, LOW);
    digitalWrite(PIN_LED_F, LOW);
    digitalWrite(PIN_LED_G, LOW);
  } else if (character == '9') {
    digitalWrite(PIN_LED_A, LOW);
    digitalWrite(PIN_LED_B, LOW);
    digitalWrite(PIN_LED_C, LOW);
    digitalWrite(PIN_LED_D, HIGH);
    digitalWrite(PIN_LED_E, HIGH);
    digitalWrite(PIN_LED_F, LOW);
    digitalWrite(PIN_LED_G, LOW);
  } else if (character == 'c') {
    digitalWrite(PIN_LED_A, HIGH);
    digitalWrite(PIN_LED_B, HIGH);
    digitalWrite(PIN_LED_C, HIGH);
    digitalWrite(PIN_LED_D, LOW);
    digitalWrite(PIN_LED_E, LOW);
    digitalWrite(PIN_LED_F, HIGH);
    digitalWrite(PIN_LED_G, LOW);
  } else if (character == 'h') {
    digitalWrite(PIN_LED_A, HIGH);
    digitalWrite(PIN_LED_B, HIGH);
    digitalWrite(PIN_LED_C, LOW);
    digitalWrite(PIN_LED_D, HIGH);
    digitalWrite(PIN_LED_E, LOW);
    digitalWrite(PIN_LED_F, LOW);
    digitalWrite(PIN_LED_G, LOW);
  } else if (character == 'C') {
    digitalWrite(PIN_LED_A, LOW);
    digitalWrite(PIN_LED_B, HIGH);
    digitalWrite(PIN_LED_C, HIGH);
    digitalWrite(PIN_LED_D, LOW);
    digitalWrite(PIN_LED_E, LOW);
    digitalWrite(PIN_LED_F, LOW);
    digitalWrite(PIN_LED_G, HIGH);
  } else {
    digitalWrite(PIN_LED_A, HIGH);
    digitalWrite(PIN_LED_B, HIGH);
    digitalWrite(PIN_LED_C, HIGH);
    digitalWrite(PIN_LED_D, HIGH);
    digitalWrite(PIN_LED_E, HIGH);
    digitalWrite(PIN_LED_F, HIGH);
    digitalWrite(PIN_LED_G, HIGH);
  }
}


#if DEBUG
void updateSerialPort(SensorMeasure sensorMeasure) {

  Serial.print("Temperatura: ");
  Serial.print(sensorMeasure.temperature);
  Serial.print(" ºC \t");
  Serial.print("Humedad: ");
  Serial.print(sensorMeasure.humidity);
  Serial.print(" % \t");
  Serial.print("Sensación térmica: ");
  Serial.print(sensorMeasure.heatIndex);
  Serial.println(" ºC");
}
#endif
