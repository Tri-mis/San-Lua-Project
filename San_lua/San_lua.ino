#include <ESP_FlexyStepper.h>
#include "esp_log.h"

// IO pin assignments
const int MOTOR_STEP_PIN = 33;
const int MOTOR_DIRECTION_PIN = 25;
const int EMERGENCY_STOP_PIN_REAR = 27;
const int EMERGENCY_STOP_PIN_FRONT = 26;

// Default values
int speed = 300;
int acceleration = 800;

// Stepper object
ESP_FlexyStepper stepper;

// Serial input
String inputString = "";
bool stringComplete = false;

// Task handle
TaskHandle_t stepperTaskHandle;

// Stepper processing task
void stepperTask(void *parameter) {
  while (true) {
    long step_left = stepper.getDistanceToTargetSigned();
    int direction = stepper.getDirectionOfMotion(); // 1 = forward, -1 = reversed
    bool rear_reached = !digitalRead(EMERGENCY_STOP_PIN_REAR);
    bool front_reached = !digitalRead(EMERGENCY_STOP_PIN_FRONT);

    if ((rear_reached == true && direction == -1)) {
      Serial.println("REAR LIMIT REACHED");
      stepper.setTargetPositionRelativeInSteps(0);
    } else if ((front_reached == true && direction == 1)) {
      stepper.setTargetPositionRelativeInSteps(0);
      Serial.println("FRONT LIMIT REACHED");
    }

    if (step_left > 0){
      Serial.println("Step" + String(step_left));
    }
    
    stepper.processMovement();
    delay(1);
  }
}

void setup() {
  Serial.begin(115200);
  esp_log_level_set("*", ESP_LOG_INFO);  // Or ESP_LOG_VERBOSE for more detail
  Serial.println("Enter a step count (e.g., 200 or -200), or S### to set speed, A### to set acceleration");

  pinMode(MOTOR_STEP_PIN, OUTPUT);
  pinMode(MOTOR_DIRECTION_PIN, OUTPUT);
  pinMode(EMERGENCY_STOP_PIN_REAR, INPUT_PULLUP);
  pinMode(EMERGENCY_STOP_PIN_FRONT, INPUT_PULLUP);

  stepper.connectToPins(MOTOR_STEP_PIN, MOTOR_DIRECTION_PIN);
  stepper.setSpeedInStepsPerSecond(speed);
  stepper.setAccelerationInStepsPerSecondPerSecond(acceleration);

  // Start stepper processing task
  xTaskCreatePinnedToCore(
    stepperTask,
    "StepperTask",
    2048,
    NULL,
    1,
    &stepperTaskHandle,
    1
  );
}

void loop() {
  // Read serial characters
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

  // When complete string received
  if (stringComplete) {
    inputString.trim();  // Remove whitespace
    handleCommand(inputString);
    inputString = "";
    stringComplete = false;
  }
}

void handleCommand(String cmd) {
  if (cmd.length() == 0) return;

  if (cmd.startsWith("S") || cmd.startsWith("s")) {
    int newSpeed = cmd.substring(1).toInt();
    if (newSpeed > 0) {
      speed = newSpeed;
      stepper.setSpeedInStepsPerSecond(speed);
      Serial.printf("Speed set to %d steps/sec\n", speed);
    } else {
      Serial.println("Invalid speed value.");
    }
  } else if (cmd.startsWith("A") || cmd.startsWith("a")) {
    int newAccel = cmd.substring(1).toInt();
    if (newAccel > 0) {
      acceleration = newAccel;
      stepper.setAccelerationInStepsPerSecondPerSecond(acceleration);
      Serial.printf("Acceleration set to %d steps/sec²\n", acceleration);
    } else {
      Serial.println("Invalid acceleration value.");
    }
  } else if (cmd.startsWith("V") || cmd.startsWith("v")) {
    int underscoreIndex = cmd.indexOf('_');
    if (underscoreIndex > 1 && underscoreIndex < cmd.length() - 1) {
      int count = cmd.substring(1, underscoreIndex).toInt();
      int amplitude = cmd.substring(underscoreIndex + 1).toInt();

      if (count > 0 && amplitude > 0) {
        Serial.printf("Vibrating %d times with amplitude %d steps\n", count, amplitude);
        for (int i = 0; i < count; i++) {
          stepper.setTargetPositionRelativeInSteps(amplitude);
          while (!stepper.motionComplete()) delay(1);
          stepper.setTargetPositionRelativeInSteps(-amplitude);
          while (!stepper.motionComplete()) delay(1);
        }
      } else {
        Serial.println("Invalid vibration parameters.");
      }
    } else {
      Serial.println("Invalid V command format. Use V<count>_<amplitude>.");
    }
  } else {
    // Handle step command (positive or negative)
    long steps = cmd.toInt();  // Handles negative numbers correctly
    if (cmd == "0" || steps != 0) {
      Serial.printf("Moving motor by %ld steps...\n", steps);
      stepper.setTargetPositionRelativeInSteps(steps);
    } else {
      Serial.println("Invalid step input.");
    }
  }
}
