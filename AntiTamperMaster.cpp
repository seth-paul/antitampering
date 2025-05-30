#include "AntiTamperMaster.h"

// ------------------------
// AntiTamperConfig Implementation
// ------------------------

// Constructor: stores the slave I2C address
AntiTamperConfig::AntiTamperConfig(uint8_t addr) : slaveAddress(addr) {}

// Initializes I2C communication
void AntiTamperConfig::begin() {
  Wire.begin();
}

// Sends all configuration parameters to the slave in one transaction
void AntiTamperConfig::sendFullConfig(
  uint16_t lightThreshold,
  uint32_t calibrationInterval,
  float vibThreshold,
  float vibImpactThreshold,
  uint32_t vibStabilityTime,
  uint32_t vibDebounceTime
) {
  Wire.beginTransmission(slaveAddress);

  // Send lightThreshold (2 bytes)
  Wire.write(highByte(lightThreshold));
  Wire.write(lowByte(lightThreshold));

  // Send calibrationInterval (4 bytes)
  Wire.write((calibrationInterval >> 24) & 0xFF);
  Wire.write((calibrationInterval >> 16) & 0xFF);
  Wire.write((calibrationInterval >> 8) & 0xFF);
  Wire.write(calibrationInterval & 0xFF);

  // Send vibThreshold (4 bytes as float)
  uint8_t *p = (uint8_t*)&vibThreshold;
  for (int i = 0; i < 4; i++) {
    Wire.write(p[i]);
  }

  // Send vibImpactThreshold (4 bytes as float)
  p = (uint8_t*)&vibImpactThreshold;
  for (int i = 0; i < 4; i++) {
    Wire.write(p[i]);
  }

  // Send vibStabilityTime (4 bytes)
  Wire.write((vibStabilityTime >> 24) & 0xFF);
  Wire.write((vibStabilityTime >> 16) & 0xFF);
  Wire.write((vibStabilityTime >> 8) & 0xFF);
  Wire.write(vibStabilityTime & 0xFF);

  // Send vibDebounceTime (4 bytes)
  Wire.write((vibDebounceTime >> 24) & 0xFF);
  Wire.write((vibDebounceTime >> 16) & 0xFF);
  Wire.write((vibDebounceTime >> 8) & 0xFF);
  Wire.write(vibDebounceTime & 0xFF);

  Wire.endTransmission();
}

// Individual setters for each configuration parameter

void AntiTamperConfig::setLightThreshold(uint16_t threshold) {
  Wire.beginTransmission(slaveAddress);
  Wire.write(0x01); // ID for lightThreshold
  Wire.write(highByte(threshold));
  Wire.write(lowByte(threshold));
  Wire.endTransmission();
}

void AntiTamperConfig::setCalibrationInterval(uint32_t interval) {
  Wire.beginTransmission(slaveAddress);
  Wire.write(0x02); // ID for calibrationInterval
  Wire.write((interval >> 24) & 0xFF);
  Wire.write((interval >> 16) & 0xFF);
  Wire.write((interval >> 8) & 0xFF);
  Wire.write(interval & 0xFF);
  Wire.endTransmission();
}

void AntiTamperConfig::setVibrationThreshold(float threshold) {
  Wire.beginTransmission(slaveAddress);
  Wire.write(0x03); // ID for vibrationThreshold
  writeFloat(threshold);
  Wire.endTransmission();
}

void AntiTamperConfig::setVibrationImpactThreshold(float threshold) {
  Wire.beginTransmission(slaveAddress);
  Wire.write(0x04); // ID for vibrationImpactThreshold
  writeFloat(threshold);
  Wire.endTransmission();
}

void AntiTamperConfig::setVibrationStabilityTime(uint32_t time) {
  Wire.beginTransmission(slaveAddress);
  Wire.write(0x05); // ID for vibrationStabilityTime
  Wire.write((time >> 24) & 0xFF);
  Wire.write((time >> 16) & 0xFF);
  Wire.write((time >> 8) & 0xFF);
  Wire.write(time & 0xFF);
  Wire.endTransmission();
}

void AntiTamperConfig::setVibrationDebounceTime(uint32_t time) {
  Wire.beginTransmission(slaveAddress);
  Wire.write(0x06); // ID for vibrationDebounceTime
  Wire.write((time >> 24) & 0xFF);
  Wire.write((time >> 16) & 0xFF);
  Wire.write((time >> 8) & 0xFF);
  Wire.write(time & 0xFF);
  Wire.endTransmission();
}

// Utility: writes a float (4 bytes) to the I2C buffer
void AntiTamperConfig::writeFloat(float value) {
  uint8_t *p = (uint8_t*)&value;
  for (int i = 0; i < 4; i++) {
    Wire.write(p[i]);
  }
}

// ------------------------
// AntiTamperMaster Implementation
// ------------------------

// Constructor: stores I2C address and LoRa reference
AntiTamperMaster::AntiTamperMaster(uint8_t addr, LoRaClass& loraRef)
  : slaveAddress(addr), lora(loraRef) {}

// Initializes I2C communication
void AntiTamperMaster::begin() {
  Wire.begin();
}

// Sends structured sensor data over LoRa in CSV format
void AntiTamperMaster::sendViaLoRa(
  uint8_t lightTampered,
  int neutralLight,
  int currentLight,
  uint8_t hallTampered,
  bool steadyOrientation,
  bool noSuddenImpact,
  bool stableLongTerm,
  bool vibTamper,
  bool moduleTampered
) {
  String message = String(lightTampered) + "," +
                   String(neutralLight) + "," +
                   String(currentLight) + "," +
                   String(hallTampered) + "," +
                   String(steadyOrientation) + "," +
                   String(noSuddenImpact) + "," +
                   String(stableLongTerm) + "," +
                   String(vibTamper) + "," +
                   String(moduleTampered);

  lora.beginPacket();
  lora.print(message);
  lora.endPacket();
}

// Reads tamper data from slave over I2C and prints to Serial
bool AntiTamperMaster::readSensorData() {
  Wire.requestFrom(slaveAddress, 11); // Expecting 11 bytes total
  if (Wire.available() != 11) {
    Serial.println("Error: Incomplete data from slave");
    return false;
  }

  // Parse data in order
  uint8_t lightTampered = Wire.read();
  int neutralLight = Wire.read() << 8 | Wire.read();
  int currentLight = Wire.read() << 8 | Wire.read();
  uint8_t hallTampered = Wire.read();
  bool steadyOrientation = Wire.read();
  bool nosuddenImpact = Wire.read();
  bool stableLongTerm = Wire.read();
  bool vibTamper = Wire.read();
  bool moduleTampered = Wire.read();

  // Output results
  Serial.println("=== Light Sensor ===");
  Serial.print("Tampered: "); Serial.println(lightTampered ? "YES" : "NO");
  Serial.print("Neutral Light: "); Serial.println(neutralLight);
  Serial.print("Current Light: "); Serial.println(currentLight);

  Serial.println("\n=== Hall Effect Sensor ===");
  Serial.print("Tampered: "); Serial.println(hallTampered ? "YES" : "NO");

  Serial.println("\n=== Vibration Sensor ===");
  Serial.print("Orientation Steady: "); Serial.println(steadyOrientation ? "YES" : "NO");
  Serial.print("No Sudden Impact: "); Serial.println(!nosuddenImpact ? "YES" : "NO");
  Serial.print("Stable Long-Term: "); Serial.println(stableLongTerm ? "YES" : "NO");
  Serial.print("Tampering Detected: "); Serial.println(vibTamper ? "YES" : "NO");

  Serial.println("\n=== MODULE STATUS ===");
  Serial.print("Tampered (2/3 sensors): "); Serial.println(moduleTampered ? "YES" : "NO");
  Serial.println("---------------------------\n");

  // Send data over LoRa
  sendViaLoRa(
    lightTampered,
    neutralLight,
    currentLight,
    hallTampered,
    steadyOrientation,
    nosuddenImpact,
    stableLongTerm,
    vibTamper,
    moduleTampered
  );

  return true;
}
