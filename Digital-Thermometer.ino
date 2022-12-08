#include <DHT.h>

// 4 digit multiplexed 7-segment display

#define DEBUG         true

// digit selector pins
#define PIN_DIGIT_0   4
#define PIN_DIGIT_1   3
#define PIN_DIGIT_2   2
#define PIN_DIGIT_3   5

// display segment pins
#define PIN_LED_A     11
#define PIN_LED_B     10
#define PIN_LED_C     9
#define PIN_LED_D     7
#define PIN_LED_E     6
#define PIN_LED_F     12
#define PIN_LED_G     8
#define PIN_LED_DOT   13

#define PIN_DHT       A0      // DHT sensor pin
#define SENSOR_TYPE   DHT11   // sensor type

// data structures
struct DisplayText {
  String temperature;
  String humidity;
  String heatIndex;
  byte temperatureDotPos;
  byte humidityDotPos;
  byte heatIndexDotPos;
};

enum eDataMode { NONE, TEMPERATURE, HUMIDITY, HEAT_INDEX };


// variables
DHT dht(PIN_DHT, SENSOR_TYPE);    // inicializar sensor DHT11

unsigned long int currentMillis = 0;
unsigned long int nextSensorTick = 0;
unsigned long int nextDataModeTick = 0;

float temperature = 0;
float humidity = 0;
float heatIndex = 0;

eDataMode dataMode = NONE;
DisplayText displayText;
byte currentDigit = 0;
char currentChar = ' ';
bool dotEnabled = false;


// initial setup
void setup() {

  // inicializar comunicacion serie
  #if DEBUG
  Serial.begin(9600);
  #endif
  
  // iniciar sensor DHT
  dht.begin();

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
}


// main loop
void loop() {
  
  currentMillis = millis();

  // sensor reading
  if (currentMillis > nextSensorTick) {
    nextSensorTick = currentMillis + 10000; // next sensor read in 10 secods

    bool valid = readSensor();
    if (valid) {
      #if DEBUG
      updateSerialPort();
      #endif
      displayText = buildDisplayText();
    }
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
  
  // update display
  updateDisplay();
  delay(5);
}


bool readSensor() {
  temperature = dht.readTemperature();  // temperatura (en grados centígrados)
  humidity = dht.readHumidity();        // humedad relativa

  // comprobar que no haya habido algún error en la lectura
  if (isnan(temperature) || isnan(humidity)) {
    #if DEBUG
    Serial.println("Error obteniendo los datos del sensor DHT11");
    #endif
    return false;
  }

  // calcular la sensación térmica (en grados centígrados)
  heatIndex = dht.computeHeatIndex(temperature, humidity, false);
  return true;
}


DisplayText buildDisplayText() {
  DisplayText displayText;
  
  displayText.temperature = String(temperature, 1);
  displayText.temperature.replace(".", "");
  displayText.temperature += "c";
  displayText.temperatureDotPos = 1;
  
  displayText.humidity = String(humidity, 1);
  displayText.humidity.replace(".", "");
  displayText.humidity.setCharAt(2, " ");
  displayText.humidity += "h";
  displayText.humidityDotPos = -1;
  
  displayText.heatIndex = String(heatIndex, 1);
  displayText.heatIndex.replace(".", "");
  displayText.heatIndex += "C";  
  displayText.heatIndexDotPos = 1;
 
  return displayText;
}


void updateDisplay() {

  // enable display current digit
  digitalWrite(PIN_DIGIT_0, (currentDigit == 0) ? HIGH : LOW);
  digitalWrite(PIN_DIGIT_1, (currentDigit == 1) ? HIGH : LOW);
  digitalWrite(PIN_DIGIT_2, (currentDigit == 2) ? HIGH : LOW);
  digitalWrite(PIN_DIGIT_3, (currentDigit == 3) ? HIGH : LOW);

  // retrieve character for the current digit
  if (dataMode == TEMPERATURE) {
    currentChar = displayText.temperature.charAt(currentDigit);
    dotEnabled = (displayText.temperatureDotPos == currentDigit) ? true : false;
  } else if (dataMode == HUMIDITY) {
    currentChar = displayText.humidity.charAt(currentDigit);
    dotEnabled = (displayText.humidityDotPos == currentDigit) ? true : false;
  } else if (dataMode == HEAT_INDEX) {
    currentChar = displayText.heatIndex.charAt(currentDigit);
    dotEnabled = (displayText.heatIndexDotPos == currentDigit) ? true : false;
  }

  // display character
  displayCharacter(currentChar, dotEnabled);

  // move to next digit
  currentDigit = (++currentDigit) % 4; // valid range 0-3
}


void displayCharacter(char character, bool dotEnabled) {

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

  digitalWrite(PIN_LED_DOT, !dotEnabled);
}


#if DEBUG
void updateSerialPort() {
  Serial.print("Temperatura: ");
  Serial.print(temperature);
  Serial.print(" ºC \t");
  Serial.print("Humedad: ");
  Serial.print(humidity);
  Serial.print(" % \t");
  Serial.print("Sensación térmica: ");
  Serial.print(heatIndex);
  Serial.println(" ºC");
}
#endif
