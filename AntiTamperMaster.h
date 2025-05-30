#ifndef ANTI_TAMPER_MASTER_H
#define ANTI_TAMPER_MASTER_H

#include <Arduino.h>
#include <Wire.h>
#include <LoRa.h>  // Required for LoRaClass reference

// ==============================
// Configuration Sender Class
// ==============================

class AntiTamperConfig {
  uint8_t slaveAddress;  // I2C address of the ATtiny84 slave

public:
  // Constructor to initialize the I2C slave address
  AntiTamperConfig(uint8_t addr);

  // Initializes I2C communication
  void begin();

  // Sends individual configuration parameters via I2C
  void setLightThreshold(uint16_t threshold);
  void setCalibrationInterval(uint32_t interval);
  void setVibrationThreshold(float threshold);
  void setVibrationImpactThreshold(float threshold);
  void setVibrationStabilityTime(uint32_t time);
  void setVibrationDebounceTime(uint32_t time);

  // Sends all parameters in a single configuration packet
  void sendFullConfig(
    uint16_t lightThreshold,
    uint32_t calibrationInterval,
    float vibThreshold,
    float vibImpactThreshold,
    uint32_t vibStabilityTime,
    uint32_t vibDebounceTime
  );

private:
  // Utility function to send a float over I2C
  void writeFloat(float value);
};

// ==============================
// Master Controller Class
// ==============================

class AntiTamperMaster {
  uint8_t slaveAddress;      // I2C address of the ATtiny84 slave
  LoRaClass* _lora;          // Pointer to LoRa instance (passed during construction)

public:
  // Constructor accepts I2C address and reference to LoRa instance
  AntiTamperMaster(uint8_t addr, LoRaClass& lora);
  
  // Initializes communication (e.g., I2C)
  void begin();

  // Reads sensor data from the ATtiny84 over I2C
  // Returns true if successful
  bool readSensorData();

  // Sends tamper status and sensor readings via LoRa
  void sendViaLoRa(
    uint8_t lightTampered,
    int neutralLight,
    int currentLight,
    uint8_t hallTampered,
    bool steadyOrientation,
    bool noSuddenImpact,
    bool stableLongTerm,
    bool vibTamper,
    bool moduleTampered
  );

private:
  LoRaClass &lora; // Reference to the global LoRa instance (redundant but included for clarity)
};

#endif
