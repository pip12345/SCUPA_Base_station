#include <Arduino.h>
#include <BLE2902.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <LoRa.h>
#include <SPI.h>

#include "ReadSerial.h"

#define BAUD 115200

//// THE FOLLOWING CANNOT BE CHANGED
// MISO - VSPI MISO -- GPIO19
// MOSI - VSPI MOSI -- GPIO23
// SCK - VSPI CLK -- GPIO18
#define ss 15
#define rst 4
#define dio0 2

// Bluetooth
BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
BLEDescriptor *pDescr;
BLE2902 *pBLE2902;

bool deviceConnected = false;
bool oldDeviceConnected = false;

#define SERVICE_UUID "6bf6566b-b277-4573-ba8b-abd8a2378f71"
#define CHARACTERISTIC_UUID "658d3bc7-5ff3-44a7-8910-0fb0ef0a7b28"

String message = "";

void send_lora_packet(String message) {
    LoRa.beginPacket();
    LoRa.print(message);
    LoRa.endPacket();
    delay(50);
}

String receive_lora_packet() {
    // try to parse packet
    String received_string{};

    int packetSize = LoRa.parsePacket();
    if (packetSize) {
        // received a packet
        Serial.print("Received packet: ");

        // read packet
        while (LoRa.available()) {
            Serial.print((char)LoRa.read());
            received_string.concat((char)LoRa.read());
        }
    }

    return received_string;
}

// Bluetooth callback functions
class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer *pServer) {
        deviceConnected = true;
    };

    void onDisconnect(BLEServer *pServer) {
        deviceConnected = false;
    }
};

void sendMessageSerial(String message) {
    message = message.substring(1, message.length());
    Serial.println(message);
    send_lora_packet(message);
}

// Bluetooth callback function that handles incoming messages
class MyCharacteristicCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        uint8_t *retrieved = pCharacteristic->getData();
        const char *retrieved_message = (const char *)retrieved;
        // checkMessage(String(retrieved_message));
        sendMessageSerial(retrieved_message);
    }
};

// Bluetooth initilizing
void ble_init() {
    BLEDevice::init("ESP32");
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    BLEService *pService = pServer->createService(SERVICE_UUID);
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_NOTIFY);

    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);

    pCharacteristic->setCallbacks(new MyCharacteristicCallbacks()); // Assign the callbacks

    pCharacteristic->setReadProperty(true);
    pCharacteristic->setNotifyProperty(true);
    pServer->getAdvertising()->start();
    pService->start();
}

ReadSerial serialRead;

void setup() {
    Serial.begin(BAUD);
    ble_init();

    while (!Serial)
        ;
    delay(500);
    Serial.println("LoRa Transceiver");
    LoRa.setPins(ss, rst, dio0);
    if (!LoRa.begin(433E6)) {
        Serial.println("Starting LoRa failed!");
        while (1)
            ;
    }

    Serial.println("Startup finished");
}

void loop() {
    // Read new incoming messages if available
    // if (serialRead.readSerial()) {
    //     message = serialRead.getMessageReceived();
    // }

    message = "$" + receive_lora_packet();

    // Converting the message to bytes to send over bluetooth
    uint8_t *message_bytes = (uint8_t *)message.c_str();
    size_t length = message.length();

    if (deviceConnected) {
        // Send the message over bluetooth if the device is connected
        pCharacteristic->setValue(message_bytes, length);
    }

    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500);                  // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        oldDeviceConnected = deviceConnected;
    }

    

    // if (deviceConnected) {
    //   if(serialRead.readSerial()){
    //     message = (uint8_t*)serialRead.getMessageReceived().c_str();
    //     length = serialRead.getMessageReceived().length();
    //     Serial.println(serialRead.getMessageReceived());
    //     pCharacteristic->setValue(message, length);
    // }
    // }
    //   // disconnecting
    // if (!deviceConnected && oldDeviceConnected) {
    //     delay(500); // give the bluetooth stack the chance to get things ready
    //     pServer->startAdvertising(); // restart advertising
    //     oldDeviceConnected = deviceConnected;
    // }
    // // connecting
    // if (deviceConnected && !oldDeviceConnected) {
    //     oldDeviceConnected = deviceConnected;
    // }
    // delay(100);
}
