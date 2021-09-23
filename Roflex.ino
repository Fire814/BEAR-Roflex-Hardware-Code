#include "Wire.h"
#include "Math.h"

#include "Adafruit_Sensor.h"
#include "Adafruit_BNO055.h"
#include "utility/imumaths.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1331.h"
#include "SPI.h"
#include "BluetoothSerial.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

//Brendan and Theo worked on Bluetooth definitions
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define XCHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define YCHARACTERISTIC_UUID "3be5a62a-2e33-4d01-b67b-b5b454a9e316"
#define ZCHARACTERISTIC_UUID "5c0f6878-b650-4438-9dec-594fc9874cd7"

BLECharacteristic *xCharacteristic;
BLECharacteristic *yCharacteristic;
BLECharacteristic *zCharacteristic;

BLECharacteristic *goodRepCharacteristic;
BLECharacteristic *avgAngleCharacteristic;
BLECharacteristic *elapsedTimeCharacteristic;
//End of Bluetooth definitions
//Brendan, Aiden, Sean sensor intialization and calibration code
Adafruit_BNO055 bno = Adafruit_BNO055(55);

int xAvg = 0, yAvg = 0, zAvg = 0;

void setup() {
  Serial.begin(115200);

  BLEConnection();
  
  if (!bno.begin())
  {
    Serial.print("Oops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while (1);
  }

  delay(1000);

  // Orientation calibration
  calibrate();
  bno.setExtCrystalUse(true);
}

void loop() {
  sensors_event_t event;
  bno.getEvent(&event);

  float x = event.orientation.x; //Gives angle from north
  float y = event.orientation.y; //Gives angle from horizontal, +90 up, -90 down
  float z = event.orientation.z; //Gives angle of rotation along device, +180 CCW, -180 CW

  xCharacteristic->setValue(x);
  yCharacteristic->setValue(y);
  zCharacteristic->setValue(z);

  Serial.println(curlCounter(10));
 
}
// Creates a bluetooth device, with services and characteristics as specified in documentation
// Begins advertisement
// More Theo and Brendan
// Bluetooth code start
void BLEConnection() {
  char deviceName[] = "Roflex";
  Serial.println("Starting BLE Connection!");
  
  BLEDevice::init(deviceName);
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  xCharacteristic  = pService->createCharacteristic(
                                         XCHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  xCharacteristic->setValue("X Value");

  yCharacteristic = pService->createCharacteristic(
                                         YCHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  yCharacteristic->setValue("Y Value");

  zCharacteristic = pService->createCharacteristic(
                                         ZCHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  zCharacteristic->setValue("Z Value");
  
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  char string1[] = "Devicce Name: ";
  char string2[] = "Service created with UUID: ";
  char string3[] = "Characteristic created with UUID: ";
  
  Serial.printf(string1, deviceName);
  Serial.printf(string2, SERVICE_UUID);
  Serial.printf(string3, XCHARACTERISTIC_UUID);
}
// Bluetooth code end

// Calibrates the device to a proper xyz plane
// Brendan, Aiden, Theo device calibration 
void calibrate() {
  float total = 0;
  float angtot = 0;
  float rotTot = 0;
  int len = 400;
  //Create integer array to hold temporary sensor values for direction, a larger array should make the user remain stable for longer
  int headerVals [len];
  int angleVals [len];
  int rotVals [len];
  
  //The device will fill the headerVals and angleVals arrays with sensor derived numbers
  for (int i = 0; i <= len - 1; i++) {
    sensors_event_t event;
    bno.getEvent(&event);

    float x = event.orientation.x; //Gives angle from north
    float y = event.orientation.y; //Gives angle from horizontal, +90 up, -90 down
    float z = event.orientation.z; //Gives angle of rotation along device, +180 CCW, -180 CW

    headerVals[i] = x;
    angleVals[i] = y;
    rotVals[i] = z;
    //Each value that is added to the arrays is added to the total variables
    total += headerVals[i];
    angtot += angleVals[i];
    rotTot += rotVals[i];
  }
  // Brendan, Aiden, Sean device angle references 
  //The total is then divided by the total number of values in the array to find the average
  //This value can then be used as a reference for the remainder of the exercise
  xAvg = total / len;
  yAvg = angtot / len;
  zAvg = rotTot /len;

  // We do not use stDev anywhere, could delete it
  
  //To find the standard deviation of the array values, the diffrence between each value and the average will be calculated and summed
  float sqDevSum = 0.0;

  for (int i = 0; i <= len - 1; i++) {
    sqDevSum += pow((xAvg - float(headerVals[i])), 2);
  }

  //The sum will then be ivided by the total number of values in the array
  float stDev = sqrt(sqDevSum / 100);
}

// Counts number of good curls (as defined by documentation)
// @param totalReps - the total number of reps to complete
// @return totalGoodReps - the number of good reps per set of curls
// Brendan, Aiden, Sean, device rep counting, good rep counting 
int curlCounter(int totalReps) {
  Serial.print("Hi");
  // Per rep scoring; 1 == perfect
  double goodRep = 0;
  float y = 0;
  // Counter of total good reps in a set
  double totalGoodReps = 0;
  int topRep = 0, reps = 0, hesStart;

  sensors_event_t event;

  while(reps < totalReps){
    bno.getEvent(&event);
    y = event.orientation.y;
    //Count each rep once the arm has been curled up beyond a given angle (50)
    //Ensure that only 10 reps have been completed and that the arm has returned to the bottom portion of the rep previously (<5)
    if (y > 50 && topRep == 0) {
      //Toggle topRep value holder and increase reps by one
      topRep = 1;
      
      delay(100);
      //Initiate hesitation timer
      hesStart = millis();
      goodRep += .5;
    }
  
    else if (y > 50 && topRep == 1 && (millis()-hesStart) > 1500) {
      goodRep += .25;
    }
    
    //Once the arm has returned to near flat, toggle the topRep value
    else if (y < 5 && topRep == 1) {
      delay(100);
      topRep = 0;
      reps++;
      //Initiate hesitation timer
      //hesStart = millis();
      goodRep += .25;
  
      if(goodRep == 1){
        totalGoodReps++;
        goodRep = 0;
      }
    }
    Serial.println(reps);
  }
  return totalGoodReps;
}

// Returns the elapsed time of the workout
// @return - the elapsed time of the workout in seconds
// Brendan, Theo elapsed time code
double elapsedTime(int startTime) {
  return (millis() - startTime) / 1000;
}

// Returns the error percentage of the reps in the workout
// @return - the percent error
double errorPercentage(int errorTot, int formTot) {
  return ((float) errorTot / formTot);
}

// Average time taken for each rep
// @return - the average time in seconds
double avgRepTime(int elapsedSecs, int reps) {
  return elapsedSecs / reps;
}

/*
// Resets the fields used
void resetTrackers() {
  reps = 0;
  topRep = 0;
  errorTot = 0;
  formTot = 0;
}
*/
