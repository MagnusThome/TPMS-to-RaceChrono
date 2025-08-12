#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "PacketIdInfo.h"
#include "tpms.h"


#define SERVICE_UUID "00001ff8-0000-1000-8000-00805f9b34fb"
#define CANBUS_MAIN_UUID "0001"  // UUID 16 bits pour la caractéristique principale
#define CANBUS_FILTER_UUID "0002"  // UUID 16 bits pour la caractéristique de filtre

BLEServer* pServer;
BLEAdvertising* pAdvertising;

BLECharacteristic* canBusMainCharacteristic;
BLECharacteristic* canBusFilterCharacteristic;

PacketIdInfo canBusPacketIdInfo;
bool canBusAllowUnknownPackets = false;
uint32_t canBusLastNotifyMs = 0;
boolean isCanBusConnected = false;


uint8_t tempData[20];
unsigned long lastSendTime = 0;
unsigned long lastNotifyTime = 0;
const long sendInterval = 1000; // 1 Hz


class FilterCallback : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* pCharacteristic) {
    String value = pCharacteristic->getValue();
    if (value.length() < 1) return;
    uint8_t command = value[0];
    switch (command) {
      case 0x00: // DENY_ALL
        if (value.length() == 1) {
          canBusPacketIdInfo.reset();
          canBusAllowUnknownPackets = false;
        }
        break;
      case 0x01: // ALLOW_ALL
        if (value.length() == 3) {
          canBusPacketIdInfo.reset();
          canBusPacketIdInfo.setDefaultNotifyInterval(sendInterval); // Forcer à 20 ms
          canBusAllowUnknownPackets = true;
        }
        break;
      case 0x02: // ADD_PID
        if (value.length() == 7) {
          uint16_t notifyIntervalMs = value[1] << 8 | value[2];
          uint32_t pid = value[3] << 24 | value[4] << 16 | value[5] << 8 | value[6];
          canBusPacketIdInfo.setNotifyInterval(pid, sendInterval); // Forcer à 20 ms
        }
        break;
    }
  }
};



void startAdvertising() {
  pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinInterval(0x20); // 20 ms
  pAdvertising->setMaxInterval(0x40); // 40 ms
  pAdvertising->start();
}



void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("TPMS bridge for RaceChrono");
  startRC();
  startTpms();
}


void startRC() {
  BLEDevice::init("RC DIY SIM");
  pServer = BLEDevice::createServer();
  BLEService* pService = pServer->createService(SERVICE_UUID);
  canBusMainCharacteristic = pService->createCharacteristic(
    CANBUS_MAIN_UUID,
    BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ
  );
  canBusMainCharacteristic->addDescriptor(new BLE2902());
  canBusFilterCharacteristic = pService->createCharacteristic(
    CANBUS_FILTER_UUID,
    BLECharacteristic::PROPERTY_WRITE
  );
  canBusFilterCharacteristic->setCallbacks(new FilterCallback());
  pService->start();
  startAdvertising();
}


void sendTyreData() {
  unsigned long currentTime = millis();

  checkTpms();

  if (currentTime - lastNotifyTime < sendInterval) return; // Respecter l'intervalle de notification
  lastNotifyTime = currentTime;

  PacketIdInfoItem* infoItem;
  uint32_t packetId;

  for (int i=0; i<NUMSENSORS; i++) {
    packetId = i + 1;
    uint16_t pressureInt = (uint16_t)(pressureBAR[i]*100.0);
    uint8_t pressureHigh = (pressureInt >> 8) & 0xFF;
    uint8_t pressureLow = pressureInt & 0xFF;
    tempData[4] = pressureHigh;
    tempData[5] = pressureLow;
    ((uint32_t*)tempData)[0] = packetId;
    infoItem = canBusPacketIdInfo.findItem(packetId, canBusAllowUnknownPackets);
    if (infoItem && infoItem->shouldNotify()) {
      canBusMainCharacteristic->setValue(tempData, 6);
      canBusMainCharacteristic->notify();
      infoItem->markNotified();
    }
  }

  for (int i=0; i<NUMSENSORS; i++) {
    packetId = i + 8;
    tempData[4] = (uint8_t)temperature[i];
    ((uint32_t*)tempData)[0] = packetId;
    infoItem = canBusPacketIdInfo.findItem(packetId, canBusAllowUnknownPackets);
    if (infoItem && infoItem->shouldNotify()) {
      canBusMainCharacteristic->setValue(tempData, 5);
      canBusMainCharacteristic->notify();
      infoItem->markNotified();
    }
  }
}



void loop() {
  unsigned long currentTime = millis();
  if (currentTime - lastSendTime >= sendInterval) {
    lastSendTime = currentTime;
    if (isCanBusConnected) {
      sendTyreData();
    }
  }

  if (!isCanBusConnected && pServer->getConnectedCount() > 0) {
    isCanBusConnected = true;
    Serial.println("BLE connected");
    canBusPacketIdInfo.reset();
  } else if (isCanBusConnected && pServer->getConnectedCount() == 0) {
    isCanBusConnected = false;
    Serial.println("BLE disconnected");
    pAdvertising->stop();
    delay(100);
    startAdvertising();
  }
}



// ----------------------
