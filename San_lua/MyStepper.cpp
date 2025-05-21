#include "MyStepper.h"

MyStepper::MyStepper(char namePrefix, int stepPin, int dirPin, int frontLimitPin, int rearLimitPin)
  : _namePrefix(namePrefix), _frontLimitPin(frontLimitPin), _rearLimitPin(rearLimitPin) {

  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(_frontLimitPin, INPUT_PULLUP);
  pinMode(_rearLimitPin, INPUT_PULLUP);

  _stepper.connectToPins(stepPin, dirPin);
  _stepper.setSpeedInStepsPerSecond(_speed);
  _stepper.setAccelerationInStepsPerSecondPerSecond(_acceleration);

  xTaskCreatePinnedToCore(
    stepperTask,
    "StepperTask",
    2048,
    this,
    1,
    &_taskHandle,
    1
  );
}

void MyStepper::stepperTask(void* parameter) {
  MyStepper* self = static_cast<MyStepper*>(parameter);

  while (true) {
    long step_left = self->_stepper.getDistanceToTargetSigned();
    int direction = self->_stepper.getDirectionOfMotion();
    bool rear_reached = !digitalRead(self->_rearLimitPin);
    bool front_reached = !digitalRead(self->_frontLimitPin);

    if ((rear_reached && direction == -1)) {
      Serial.println(String(self->_namePrefix) + " motor: REAR LIMIT REACHED");
      self->_stepper.setTargetPositionRelativeInSteps(0);
    } else if ((front_reached && direction == 1)) {
      Serial.println(String(self->_namePrefix) + " motor: FRONT LIMIT REACHED");
      self->_stepper.setTargetPositionRelativeInSteps(0);
    }

    if (step_left != 0) {
      Serial.println(String(self->_namePrefix) + " motor step left: " + String(step_left));
    }

    self->_stepper.processMovement();
    delay(1);
  }
}

void MyStepper::handleCommand(String cmd) {
  if (cmd.length() < 2 || cmd[1] != ':') return;
  cmd = cmd.substring(2);

  if (cmd.startsWith("S") || cmd.startsWith("s")) {
    int newSpeed = cmd.substring(1).toInt();
    if (newSpeed > 0) {
      _speed = newSpeed;
      _stepper.setSpeedInStepsPerSecond(_speed);
      Serial.println(String(_namePrefix) + " motor: Speed set to " + String(_speed) + " steps/sec");
    }
  } else if (cmd.startsWith("A") || cmd.startsWith("a")) {
    int newAccel = cmd.substring(1).toInt();
    if (newAccel > 0) {
      _acceleration = newAccel;
      _stepper.setAccelerationInStepsPerSecondPerSecond(_acceleration);
      Serial.println(String(_namePrefix) + " motor: Acceleration set to " + String(_acceleration) + " steps/sec²");
    }
  } else if (cmd.startsWith("V") || cmd.startsWith("v")) {
    int underscoreIndex = cmd.indexOf('_');
    if (underscoreIndex > 1 && underscoreIndex < cmd.length() - 1) {
      int count = cmd.substring(1, underscoreIndex).toInt();
      int amplitude = cmd.substring(underscoreIndex + 1).toInt();

      if (count > 0 && amplitude > 0) {
        Serial.println(String(_namePrefix) + " motor: Vibrating " + String(count) + " times with amplitude " + String(amplitude) + " steps");
        for (int i = 0; i < count; i++) {
          _stepper.setTargetPositionRelativeInSteps(amplitude);
          while (!_stepper.motionComplete()) delay(1);
          _stepper.setTargetPositionRelativeInSteps(-amplitude);
          while (!_stepper.motionComplete()) delay(1);
        }
      } else {
        Serial.println("Invalid vibration parameters.");
      }
    } else {
      Serial.println("Invalid V command format. Use V<count>_<amplitude>.");
    }
  } else {
    long steps = cmd.toInt();
    if (cmd == "0" || steps != 0) {
      Serial.println(String(_namePrefix) + " motor: Moving by " + String(steps) + " steps");
      _stepper.setTargetPositionRelativeInSteps(steps);
    } else {
      Serial.println(String(_namePrefix) + " motor: Invalid step command");
    }
  }
}
