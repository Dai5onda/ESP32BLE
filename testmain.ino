#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
float txValue = 0;
const int button = 0;      // button on PIN G0
const int readPin = 32;    // analog pin G32
const int LEDpin = 2;      // LED on pin G2
bool convert = false;
String rxString = "";
std::string rxValue;       // rxValue gathers input data

// UART service UUID data
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" 
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string rxValue = pCharacteristic->getValue();
    if (rxValue.length() > 0)  {
      convert = true;      // flag to invoke convertControlpad routine
      Serial.println(" ");
      Serial.print("Received data: ");
      for (int i = 0; i < rxValue.length(); i++) { 
        Serial.print(rxValue[i]);
        rxString = rxString + rxValue[i]; // build string from received data 
      } 
    } 
  } 
}; 

// ***************************** SETUP *******************************
void setup() { 
  Serial.begin(115200); 
  pinMode(LEDpin, OUTPUT); 
  pinMode(button, INPUT);
  
  BLEDevice::init("Wim's ESP32 UART"); // give the BLE device a name
  
  BLEServer *pServer = BLEDevice::createServer(); // create BLE server
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_NOTIFY);                    
  pCharacteristic->addDescriptor(new BLE2902());
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_RX,
                                         BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->setCallbacks(new MyCallbacks());
  
  pService->start(); // start the service

  pServer->getAdvertising()->start(); // start advertising
  Serial.println("Waiting a client connection to notify...");
  Serial.println(" ");
}

// *************************** MAIN PROGRAM *********************************
void loop() {
  if (deviceConnected) {
    txValue = analogRead(readPin);        // pick up value on readPin
    char txString[8];                     // convert the value to a char array
    dtostrf(txValue, 1, 2, txString); // float_val, min_width, decimal_digits, char_buffer   
//    pCharacteristic->setValue(&txValue, 1); // to send the integer value
//    pCharacteristic->setValue("Hello!");    // to send a test message
    pCharacteristic->setValue(txString);  // prepare to send array
    if (digitalRead(button) == LOW) {     // send when button is pressed
      pCharacteristic->notify();          // send the value to the app!
      pCharacteristic->setValue(" ");     // send a space
      pCharacteristic->notify();
      digitalWrite(LEDpin, HIGH);         // switch on the LED
      Serial.println(" ");
      Serial.print("*** Peripheral sends: "); 
      Serial.print(txString);             // report value on serial line
      Serial.println(" ***");
    }
    else digitalWrite(LEDpin, LOW);
    if (convert) convertControlpad();
  }
  delay(50);
}

// ************************* CONVERT CONTROLPAD CODE ************************
void convertControlpad() {
  convert = false;
  Serial.print("      ");
  if (rxString == "!B11:") Serial.println("********** Start Action 1");
  else if (rxString == "!B219") Serial.println("********** Start Action 2");
  else if (rxString == "!B318") Serial.println("********** Start Action 3");
  else if (rxString == "!B417") Serial.println("********** Start Action 4");
  else if (rxString == "!B516") Serial.println("********** Start Action UP");
  else if (rxString == "!B615") Serial.println("********** Start Action DOWN");
  else if (rxString == "!B714") Serial.println("********** Start Action LEFT");
  else if (rxString == "!B813") Serial.println("********** Start Action RIGHT");
  else if (rxString == "!B10;") Serial.println("********** Stop Action 1");
  else if (rxString == "!B20:") Serial.println("********** Stop Action 2");
  else if (rxString == "!B309") Serial.println("********** Stop Action 3");
  else if (rxString == "!B408") Serial.println("********** Stop Action 4");
  else if (rxString == "!B507") Serial.println("********** Stop Action UP");
  else if (rxString == "!B606") Serial.println("********** Stop Action DOWN");
  else if (rxString == "!B705") Serial.println("********** Stop Action LEFT");
  else if (rxString == "!B804") Serial.println("********** Stop Action RIGHT");  
  rxString = "";
}
