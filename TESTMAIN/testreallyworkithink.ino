#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

BLEServer *pServer;
BLEService *pService;
BLECharacteristic *pCharacteristic;

const char* serviceUUID = "0000FFFF-0000-1000-8000-00805F9B34FB";
const char* characteristicUUID = "0000FF01-0000-1000-8000-00805F9B34FB";

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        // Handle connection event
    }

    void onDisconnect(BLEServer* pServer) {
        // Handle disconnection event
    }
};

void setup() {
    Serial.begin(115200);

    BLEDevice::init("ESP32");
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    pService = pServer->createService(BLEUUID(serviceUUID));

    pCharacteristic = pService->createCharacteristic(
        BLEUUID(characteristicUUID),
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
    );

    pCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
    pService->start();

    BLEAdvertising *pAdvertising = pServer->getAdvertising();
    pAdvertising->start();
}

void loop() {
    // Handle other tasks if needed
}
