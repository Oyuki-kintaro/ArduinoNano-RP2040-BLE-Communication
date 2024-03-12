#include <ArduinoBLE.h>
#include <Arduino_LSM6DSOX.h>
#include <PDM.h>

static const int maxAttempts = 5; 
static const char channels = 1;
static const int pdm_frequency = 16000;

BLEService ledService("19B10001-E8F2-537E-4F6C-D104768A1214"); // BLE LED Service
BLEStringCharacteristic stringCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLEWrite | BLEWriteWithoutResponse | BLENotify, 30);

// Buffer to read samples into, each sample is 16-bits
short sampleBuffer[512];
// Number of audio samples read
volatile int samplesRead;

// IMU
float Ax, Ay, Az; // accelerometer
float Gx, Gy, Gz; //gyro
int temp;
int attempts;

// String data to send
String mic = "";
String accel = "";
String gyro = "";
String temperature = "";


void setup() {
  Serial.begin(9600);

  initBLE();
  initIMU();
  initPDM();
}

void loop() {
  BLEDevice central = BLE.central();
  if (central) {
    if (samplesRead) {

      // Print samples to the serial monitor or plotter
      for (int i = 0; i < samplesRead; i++) {

        String mic = "Mic Hz: " + String(sampleBuffer[i]);

        if (IMU.accelerationAvailable()) {
          IMU.readAcceleration(Ax, Ay, Az);

          accel = "Accel: " + String(Ax) + " " + String(Ay) + " " + String(Az);
        }

        if (IMU.gyroscopeAvailable()) {
          IMU.readGyroscope(Gx, Gy, Gz);
          
          gyro = "Gyro: " + String(Gx) + " " + String(Gy) + " " + String(Gz);
        }

        if (IMU.temperatureAvailable())
        {
          IMU.readTemperature(temp);

          temperature = "Temp: " + String(temp) + " °C";
        }

        stringCharacteristic.writeValue(mic.c_str());
        stringCharacteristic.writeValue(accel.c_str());
        stringCharacteristic.writeValue(gyro.c_str());
        stringCharacteristic.writeValue(temperature.c_str());
      }

      // Clear the read count
      samplesRead = 0;
    }
  }
}

void initBLE() {
  
  attempts = 0;
  while (!BLE.begin() && attempts < maxAttempts) {
      Serial.println("Starting Bluetooth® Low Energy failed. Retrying...");
      attempts++;
      delay(1000);  // Adjust delay as needed
  }

  if (attempts >= maxAttempts) {
      Serial.println("Max attempts reached. Bluetooth initialization failed.");
      while (1);  // Or handle the failure in an appropriate way
  }
  
  BLE.setLocalName("RP2040");
  BLE.setAdvertisedService(ledService);

  ledService.addCharacteristic(stringCharacteristic);
  BLE.addService(ledService);

  BLE.advertise();
}

void initIMU() {

  attempts = 0;
  while (!IMU.begin() && attempts < maxAttempts) {
      Serial.println("Failed to initialize IMU!");
      attempts++;
      delay(1000);  // Adjust delay as needed
  }

  if (attempts >= maxAttempts) {
      Serial.println("Max attempts reached. IMU initialization failed.");
      while (1);  // Or handle the failure in an appropriate way
  }
}

void initPDM() {
  
  // Configure the data receive callback
  PDM.onReceive(onPDMdata);

  attempts = 0;
  while (!PDM.begin(channels, pdm_frequency) && attempts < maxAttempts) {
      Serial.println("Failed to start PDM!");
      attempts++;
      delay(1000);  // Adjust delay as needed
  }

  if (attempts >= maxAttempts) {
      Serial.println("Max attempts reached. PDM initialization failed.");
      while (1);  // Or handle the failure in an appropriate way
  }
}

/**
   Callback function to process the data from the PDM microphone.
   NOTE: This callback is executed as part of an ISR.
   Therefore using `Serial` to print messages inside this function isn't supported.
 * */
void onPDMdata() {
  // Query the number of available bytes
  int bytesAvailable = PDM.available();

  // Read into the sample buffer
  PDM.read(sampleBuffer, bytesAvailable);

  // 16-bit, 2 bytes per sample
  samplesRead = bytesAvailable / 2;
}