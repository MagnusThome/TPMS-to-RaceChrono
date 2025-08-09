# TPMS-to-RaceChrono
An ESP32 based bridge between cheap BLE TPMS sensors and RaceChrono

**Work in progress** and to be honest a bit of a mess put together from different sources. The code works with some different cheap TPMS but you might need to change the ID (BLE adress) in the code to match that of your own sensors, see the file tpms.h for more info. Any board based on ESP32 or ESP32-S3 should work. You only use BLE on it, both for the connections to the sensors and for the connection to Racechrono.


Thanks to these:  
https://github.com/NicoEFI/Racechrono-ESP32-S3  
https://github.com/andi38/TPMS  
https://github.com/upiir/arduino_tpms_tire_pressure  
  

<img width="400" src="https://github.com/user-attachments/assets/d188f58d-d76c-4e19-bc0c-c9e1b884d5b4" />  

<img width="400" src="https://github.com/user-attachments/assets/fcfa28c5-8430-4c20-b292-80c3b20232b8" />  
  
![PXL_20250808_121657496](https://github.com/user-attachments/assets/058de4e5-72ef-4998-9203-bde412110fdf)

## Setting up in Racechrono  
  
In Racechrono add the DIY BLE under "Add other device".   
Then add these eight channels in your car's settings:  
  
<img width="400" src="https://github.com/user-attachments/assets/73a66d05-20b3-4a0e-a827-9919c5fbdc06" />
  
## Pressure  
  
The four tyre *pressure* channels have the follwing PIDs  
0x01 = FL Front Left  
0x02 = FR Front Right  
0x03 = RL Rear Left  
0x04 = RR Rear Right  
Note the "Equation" also in the picture   
  
<img width="400" src="https://github.com/user-attachments/assets/825ac94f-d89e-4fb6-9f20-22d99df5e1cb" />
  
## Temperature  
  
The four tyre *temperature* channels have the follwing PIDs  
0x08 (08) = FL Front Left  
0x09 (09) = FR Front Right  
0x0A (10) = RL Rear Left  
0x0B (11) = RR Rear Right  
Note the "Equation" also in the picture    

When a sensor hasn't reported anything in five minutes an "alarm" is raised by reporting the temperature as 0 degrees but the previous older read of pressure is kept.
  
<img width="400" src="https://github.com/user-attachments/assets/04dd2139-6c74-41ba-9762-cf8261e29d31" />
