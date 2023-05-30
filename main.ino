#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
float txValue = 0;
const int readPin = 32;    // analog pin G32
const int relayPin = 5;    // relay control pin
bool manualMode = false;   // flag for manual mode
bool relayState = false;   // current relay state

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
      if (rxValue[0] == 'm') {
        // Switch to manual mode
        manualMode = true;
        Serial.println("Switched to manual mode");
      }
      else if (rxValue[0] == 'a') {
        // Switch to auto mode
        manualMode = false;
        Serial.println("Switched to auto mode");
      }
      else if (rxValue[0] == '1') {
        // Turn on relay in manual mode
        relayState = true;
        Serial.println("Relay turned on");
      }
      else if (rxValue[0] == '0') {
        // Turn off relay in manual mode
        relayState = false;
        Serial.println("Relay turned off");
      }
    }
  }
};

// ***************************** SETUP *******************************
void setup() {
  Serial.begin(115200);
  pinMode(relayPin, OUTPUT);

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
  Serial.println("Waiting for client connection...");
  Serial.println();
}

// *************************** MAIN PROGRAM *********************************
void loop() {
  if (deviceConnected) {
    if (manualMode) {
      // Manual mode: Relay state is controlled by app commands
      digitalWrite(relayPin, relayState ? HIGH : LOW);
    }
    else {
      // Auto mode: Relay state is determined by sensor value
      txValue = analogRead(readPin);        // pick up value on readPin
      if (txValue > 2000) {
        digitalWrite(relayPin, HIGH);
        relayState = true;
      }
      else {
        digitalWrite(relayPin, LOW);
        relayState = false;
      }
    }

    // Notify app about the relay state and sensor value
    char txString[8];
    sprintf(txString, "%d:%.2f", relayState, txValue);
    pCharacteristic->setValue(txString);
    pCharacteristic->notify();

    delay(100);
  }
}
