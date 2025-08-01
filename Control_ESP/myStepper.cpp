// myStepper.cpp
#include "myStepper.h"

MyStepper::MyStepper(int stepPin, int dirPin, int frontLimitPin, int rearLimitPin, int emergencyPin, String stepperName)
    : stepPin(stepPin), dirPin(dirPin), stepperName(stepperName),
      frontLimitPin(frontLimitPin), rearLimitPin(rearLimitPin), emergencyPin(emergencyPin) 
{
    pinMode(stepPin, OUTPUT);
    pinMode(dirPin, OUTPUT);
    pinMode(frontLimitPin, INPUT_PULLUP);
    pinMode(rearLimitPin, INPUT_PULLUP);
    pinMode(emergencyPin, INPUT_PULLUP);
    setSpeed(1.0);
}

void MyStepper::beginTask() {
    if (taskHandle == nullptr) {
        xTaskCreatePinnedToCore(taskLoop, "StepperTask", 5000, this, 1, &taskHandle, 1);
    }
}

void MyStepper::setSpeed(float newSpeed) {
    speed = newSpeed / 1000.0f;
    stepDelayMicros = (unsigned long)(1000.0f / speed);
}

void MyStepper::addCommand(StepperCommand cmd) {
    commandQueue.push(cmd);
}

int MyStepper::getCurrentStep() const {
    return currentStep;
}

float MyStepper::getSpeed() const {
    return speed;
}

bool MyStepper::checkFrontLimit() {
    return digitalRead(frontLimitPin) == LOW;
}

bool MyStepper::checkRearLimit() {
    return digitalRead(rearLimitPin) == LOW;
}

bool MyStepper::checkEmergency() {
    return digitalRead(emergencyPin) == LOW;
}

void MyStepper::doStep(bool dir) {
    digitalWrite(dirPin, dir);
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(50);
    digitalWrite(stepPin, LOW);
    currentStep += dir ? 1 : -1;
}

void MyStepper::moveToTarget(int targetStep) {
    unsigned long lastStepTime = 0;
    while (currentStep != targetStep) {
        if (checkEmergency()) break;
        if ((targetStep > currentStep && checkFrontLimit()) || 
            (targetStep < currentStep && checkRearLimit())) break;

        unsigned long now = micros();
        if (now - lastStepTime >= stepDelayMicros) {
            bool dir = (targetStep > currentStep);
            doStep(dir);
            lastStepTime = now;
        }
    }
}

void MyStepper::homing(bool towardFront) {
    unsigned long lastStepTime = 0;
    while (true) {
        if (checkEmergency()) break;
        if (towardFront && checkFrontLimit()) break;
        if (!towardFront && checkRearLimit()) break;

        unsigned long now = micros();
        if (now - lastStepTime >= stepDelayMicros) {
            doStep(towardFront);
            lastStepTime = now;
        }
    }
    currentStep = 0;
}

void MyStepper::vibrate(int amplitude, int times) {
    for (int i = 0; i < times; i++) {
        moveToTarget(currentStep + amplitude);
        delayMicroseconds(500);
        moveToTarget(currentStep - amplitude);
        delayMicroseconds(500);
    }
}

void MyStepper::taskLoop(void* param) {
    MyStepper* self = static_cast<MyStepper*>(param);
    for (;;) {
        if (!self->commandQueue.empty()) {
            StepperCommand cmd = self->commandQueue.front();
            self->commandQueue.pop();

            switch (cmd.mode) {
                case StepperMode::IDLE:
                    delay(10);
                    break;
                case StepperMode::NORMAL_RUN:
                    self->moveToTarget(cmd.param1);
                    break;
                case StepperMode::HOMING:
                    self->homing(cmd.param1 != 0);
                    break;
                case StepperMode::VIBRATE:
                    self->vibrate(cmd.param1, cmd.param2);
                    break;
                case StepperMode::SET_SPEED:
                    self->speed = cmd.param1 / 1000.0f;
                    self->stepDelayMicros = (unsigned long)(1000.0f / self->speed);
                    break;
            }
        } else {
            delay(5);
        }
    }
}
