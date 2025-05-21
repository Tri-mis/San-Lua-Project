#include "MyStepper.h"

MyStepper bed_motor('B', 33, 25, 26, 27);
MyStepper lid_motor('L', 14, 12, 32, 13);

String inputString = "";
bool stringComplete = false;

void setup() {
  Serial.begin(115200);
  Serial.println("Ready to receive commands");
}

void loop() {
  // Read serial input
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == '\n' || inChar == '\r') {
      if (inputString.length() > 0) {
        stringComplete = true;
      }
    } else {
      inputString += inChar;
    }
  }

  // Process command
  if (stringComplete) {
    inputString.trim();

    if (inputString.startsWith("B:")) {
      bed_motor.handleCommand(inputString);
    } else if (inputString.startsWith("L:")) {
      lid_motor.handleCommand(inputString);
    } else {
      Serial.println("Invalid command. Use B:[...] or L:[...]");
    }

    inputString = "";
    stringComplete = false;
  }
}
