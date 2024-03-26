//Temperature

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

HardwareSerial SerialESP2(2);

BLEClient* pClient;
BLERemoteCharacteristic* pRemoteCharacteristic;
bool deviceConnected = false;

BLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b"); //temp
BLEUUID charUUID("beb5483e-36e1-4688-b7f5-ea07361b26a8"); //temp


class MyClientCallbacks : public BLEClientCallbacks {
    void onConnect(BLEClient* client) {
        deviceConnected = true;
        Serial.println("Connected to client");
    }

void onDisconnect(BLEClient* client) {
        deviceConnected = false;
        Serial.println("Disconnected from client");
        pRemoteCharacteristic = nullptr;

        BLEScan* pBLEScan = BLEDevice::getScan();
        pBLEScan->setActiveScan(true);
        pBLEScan->setInterval(100);
        pBLEScan->setWindow(99);

        BLEScanResults foundDevices = pBLEScan->start(5, false);

        for (int i = 0; i < foundDevices.getCount(); i++) {
            BLEAdvertisedDevice device = foundDevices.getDevice(i);
            if (device.haveServiceUUID() && device.isAdvertisingService(serviceUUID)) {
                BLEAddress address = device.getAddress();
                esp_ble_addr_type_t addressType = device.getAddressType();
                pClient->connect(address, addressType);
                break;
            }
        }
    }
};

void setup() {
    Serial.begin(115200);
    SerialESP2.begin(115200);
    Serial.println("Scanning for BLE server...");

    BLEDevice::init("ESP32 Receiver");
    pClient = BLEDevice::createClient();
    pClient->setClientCallbacks(new MyClientCallbacks());

    BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);

    BLEScanResults foundDevices = pBLEScan->start(5, false);

    for (int i = 0; i < foundDevices.getCount(); i++) {
        BLEAdvertisedDevice device = foundDevices.getDevice(i);
        if (device.haveServiceUUID() && device.isAdvertisingService(serviceUUID)) {
            BLEAddress address = device.getAddress();
            esp_ble_addr_type_t addressType = device.getAddressType();
            pClient->connect(address, addressType);
            break;
        }
    }
}

void loop() {
  if (SerialESP2.available()) {
    String receivedData = SerialESP2.readStringUntil('\n');
    Serial.println(receivedData);
  }
    if (deviceConnected) {
        if (pRemoteCharacteristic == nullptr) {
            pRemoteCharacteristic = pClient->getService(serviceUUID)->getCharacteristic(charUUID);
            pRemoteCharacteristic->registerForNotify(nullptr);
        } else {
            if (pRemoteCharacteristic->canRead()) {
                std::string value = pRemoteCharacteristic->readValue();
                Serial.print("Received Value: ");
                Serial.println(value.c_str());

                String stringValue(value.c_str());

                SerialESP2.println(stringValue);
            }
        }
    }
    delay(2000);
}