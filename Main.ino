#ifndef ZIGBEE_MODE_ED
#error "Zigbee end device mode is not selected in Tools->Zigbee mode"
#endif

#include "Zigbee.h"

/* Zigbee smart switch configuration */
#define ZIGBEE_SWITCH_ENDPOINT 11
uint8_t relayPin = 12;
uint8_t button = BOOT_PIN;
uint8_t led = RGB_BUILTIN;

ZigbeeSwitch zbSwitch = ZigbeeSwitch(ZIGBEE_SWITCH_ENDPOINT);

/********************* LED and Relay functions **************************/
void setLEDColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t brightness) {
  rgbLedWrite(red, green, blue, brightness);
}

void toggleRelay(bool value) {
  digitalWrite(relayPin, value);
  if (value) {
    setLEDColor(0, 51, 0, 51);  // Green at 20%
  } else {
    setLEDColor(51, 51, 0, 51);  // Yellow at 20%
  }
}

/********************* Arduino functions **************************/
void setup() {
  Serial.begin(115200);

  // Init relay and turn it OFF
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);

  // Init LED and set it to OFF
  pinMode(led, OUTPUT);
  setLEDColor(0, 0, 0, 0);

  // Init button for factory reset
  pinMode(button, INPUT_PULLUP);

  // Optional: set Zigbee device name and model
  zbSwitch.setManufacturerAndModel("Espressif", "ZBSwitch");

  // Add endpoint to Zigbee Core
  Serial.println("Adding ZigbeeSwitch endpoint to Zigbee Core");
  Zigbee.addEndpoint(&zbSwitch);

  // When all EPs are registered, start Zigbee. By default acts as ZIGBEE_END_DEVICE
  if (!Zigbee.begin()) {
    Serial.println("Zigbee failed to start!");
    Serial.println("Rebooting...");
    ESP.restart();
  }
  Serial.println("Connecting to network");
  while (!Zigbee.connected()) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();
}

void loop() {
  // Checking button for factory reset
  if (digitalRead(button) == LOW) {  // Push button pressed
    // Key debounce handling
    delay(100);
    int startTime = millis();
    while (digitalRead(button) == LOW) {
      delay(50);
      if ((millis() - startTime) > 3000) {
        // If key pressed for more than 3secs, factory reset Zigbee and reboot
        Serial.println("Resetting Zigbee to factory and rebooting in 1s.");
        delay(1000);
        Zigbee.factoryReset();
      }
    }
    // Toggle relay by pressing the button
    bool currentState = digitalRead(relayPin);
    toggleRelay(!currentState);
  }
  delay(100);
}
