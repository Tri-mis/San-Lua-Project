#include "my_project_lib.h"

MyStepper bed_stepper(33, 25, 26, 27, 22, "BED");
MyStepper box_stepper(19, 21, 23, 15, 22, "BOX");

void handleSerialInput(bool &systemStart) {
  if (!Serial.available()) return;

  String input = Serial.readStringUntil('\n');
  input.trim();

  // Check for SYSTEM_START command
  if (input.equalsIgnoreCase("SYSTEM_START")) {
    systemStart = true;
    Serial.println("System start command received.");
    return;
  }

  int idx1 = input.indexOf('_');
  int idx2 = input.indexOf('_', idx1 + 1);
  int idx3 = input.indexOf('_', idx2 + 1);

  if (idx1 == -1 || idx2 == -1) {
    Serial.println("Invalid command format.");
    return;
  }

  String motor = input.substring(0, idx1);
  String command = input.substring(idx1 + 1, idx2);
  String param1Str = (idx3 != -1) ? input.substring(idx2 + 1, idx3) : input.substring(idx2 + 1);
  String param2Str = (idx3 != -1) ? input.substring(idx3 + 1) : "";

  // Handle DC motor separately
  if (motor == "DC" && command == "RUN") {
    int duty = param1Str.toInt();
    duty = constrain(duty, 0, 1023);  // Assuming 10-bit resolution
    ledcWrite(18, duty);
    Serial.println("DC motor running at duty: " + String(duty));
    return;
  }

  // Stepper motor commands
  MyStepper* target = nullptr;
  if (motor == "BED") {
    target = &bed_stepper;
  } else if (motor == "BOX") {
    target = &box_stepper;
  } else {
    Serial.println("Unknown motor name: " + motor);
    return;
  }

  if (command == "STEP") {
    int steps = param1Str.toInt();
    target->addCommand(StepperCommand(StepperMode::NORMAL_RUN, steps));
  } else if (command == "HOME") {
    bool towardFront = (param1Str == "TRUE" || param1Str == "true" || param1Str == "1");
    target->addCommand(StepperCommand(StepperMode::HOMING, towardFront));
  } else if (command == "VIBRATE") {
    int amp = param1Str.toInt();
    int times = param2Str.toInt();
    target->addCommand(StepperCommand(StepperMode::VIBRATE, amp, times));
  } else if (command == "SPEED") {
    int speedMillis = param1Str.toInt();  // e.g., 500 → 0.5 steps/ms
    target->addCommand(StepperCommand(StepperMode::SET_SPEED, speedMillis));
  } else {
    Serial.println("Unknown command type: " + command);
  }
}

