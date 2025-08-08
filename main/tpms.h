
#include <vector>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>


//
// Enter the ID's (BLE adress) for each sensor you have.
//
String ble_id[] = { "4a:24:00:00:9c:53", "4a:6b:00:00:de:58", "4a:40:00:00:9a:3b", "4a:42:00:00:9e:13" };
//
// To find what adresses your sensors have you can uncomment the Serial.printf line aprox twenty rows further below
// and watch the full info from every closeby BLE device.
//

String pos_id[] = { "FL ", "FR ", "RL ", "RR " };

int scanTime = 5;  //In seconds
BLEScan *pBLEScan;


#define NUMSENSORS 4
float voltage[NUMSENSORS];      // sensor battery voltage in V  (float)
int temperature[NUMSENSORS];    // sensor temperature in °C     (int)
float pressurePSI[NUMSENSORS];  // sensor pressure PSI          (float)
float pressureBAR[NUMSENSORS];  // sensor pressure BAR          (float)
bool updated[NUMSENSORS]; 

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {

    //Serial.printf("%s \n", advertisedDevice.toString().c_str()); // print all the received data to serial
  
    for (int s=0; s<NUMSENSORS; s++) {
      if (  
            advertisedDevice.getAddress().toString() == ble_id[s]
            && 
            advertisedDevice.haveManufacturerData() == true
          ) {
        String strManufacturerData = advertisedDevice.getManufacturerData(); // get the advertised data as String
  
        // Received Manufacturers Data Example
        // [80] [1f] [1a] [00] [92] [14] [74]
        //
        // meaning:
        // 80 - status - ignore for now
        // 1f - battery voltage - 0x1f hex = 31 = 3.1V
        // 1a - temperature 0x1a hex = 26°C
        // 0092 - pressure 0x0092 hex = 146 = 14.6 psi
        // 14 74 - checksum - ignore for now  
  
        pressurePSI[s] = (float)((uint16_t)strManufacturerData[3] << 8 | strManufacturerData[4]) / 10.0; 
        pressureBAR[s] = pressurePSI[s] / 14.5038;
        temperature[s] = strManufacturerData[2];
        voltage[s] = (float)strManufacturerData[1] / 10.0;
        updated[s] = true;
      }
    }
  }
};



void startTpms() {
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();  //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);    //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);          // less or equal setInterval value
}



void checkTpms() {
  BLEScanResults *foundDevices = pBLEScan->start(scanTime, false);
//Serial.print("Devices found: ");
//Serial.println(foundDevices->getCount());
  pBLEScan->clearResults();  // delete results fromBLEScan buffer to release memory

  for (int s=0; s<NUMSENSORS; s++) {
    if (updated[s]) {
      Serial.print(pos_id[s]);
      Serial.print(pressureBAR[s]); 
      Serial.print(" bar   ");
      Serial.print(temperature[s]);  
      Serial.print("°C   ");
      Serial.print(voltage[s]);
      Serial.print("V   ");    
      Serial.println();    
      updated[s] = false;
    }
  }
  delay(50);
}
