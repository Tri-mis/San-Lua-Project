#include <Arduino.h>
#include <queue>

enum class MoveMode { STEP, DISTANCE };

class Command 
{
  private:
    MoveMode mode;
    float value;

  public:
    Command(MoveMode mode, float value)
      : mode(mode), value(value) {}

    MoveMode getMode() const { return mode; }
    float getValue() const { return value; }
};

class StepperController {
  public:
    float speed;                  // steps/ms
    float distPerStep;            // mm/step
    int stepPin, dirPin;
    int limitPin1, limitPin2;
    int emgPin;

    long currentStep = 0;
    long targetStep = 0;

    std::queue<Command> commandQueue; //"std::" is the offical library call, "queue" is a type in the library call, this type is a FIFO queue, "<Command>" let the complier know that Command is the type in the queue
    TaskHandle_t runTaskHandle;
    
  public:
    StepperController(float speed, float distPerStep,
                      int stepPin, int dirPin,
                      int limitPin1, int limitPin2, int emgPin);

    void homing(bool direction);
    void run();
    void readCommand();

    void addCommand(MoveMode mode, float value);
    void getStatus(MoveMode mode, float &target, float &remaining);
    bool isBusy() const;

  private:
    void stepMotor(bool dir);
    void stopMotor();
    bool checkEmergency();
    bool checkLimit(bool dir);

};

// ------------------ Implementation ------------------

StepperController::StepperController(float speed, float distPerStep,
                                     int stepPin, int dirPin,
                                     int limitPin1, int limitPin2, int emgPin)
    : speed(speed), distPerStep(distPerStep),
      stepPin(stepPin), dirPin(dirPin),
      limitPin1(limitPin1), limitPin2(limitPin2), emgPin(emgPin) {

  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(limitPin1, INPUT_PULLUP);
  pinMode(limitPin2, INPUT_PULLUP);
  pinMode(emgPin, INPUT_PULLUP);

  xTaskCreatePinnedToCore(
    [](void *param){static_cast<StepperController*>(param)->run();}, 
    "StepperRun", 
    4096, 
    this, 
    1, 
    &runTaskHandle, 
    1);
}

void StepperController::addCommand(MoveMode mode, float value) {
  commandQueue.push(Command(mode, value));
}

void StepperController::readCommand() {
  if (!commandQueue.empty() && currentStep == targetStep) {
    Command cmd = commandQueue.front();
    commandQueue.pop();

    if (cmd.getMode() == MoveMode::STEP) {
      targetStep = currentStep + static_cast<long>(cmd.getValue());
    } else {
      targetStep = currentStep + static_cast<long>(cmd.getValue() / distPerStep);
    }
  }
}

void StepperController::run() {
  int stepDelayMicros = (int)(1000.0f / speed);  // 500 µs if speed = 2.0
  unsigned long lastStepTime = micros();

  while (true) {
    if (checkEmergency()) 
    {
      stopMotor();
      vTaskDelay(pdMS_TO_TICKS(10));
      continue;
    }

    readCommand();

    long diff = targetStep - currentStep;
    bool dir = diff > 0;

    if (diff != 0 && !checkLimit(dir)) {
      unsigned long now = micros();
      if ((now - lastStepTime) >= (unsigned long)stepDelayMicros) {
        stepMotor(dir);  // 50 µs pulse only
        lastStepTime = now;
      }
    }

    yield();  // Let other tasks run
  }
}


void StepperController::homing(bool direction) {
  digitalWrite(dirPin, direction);
  while (!checkLimit(direction)) {
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(2000);
    yield();
  }
  currentStep = 0;
  targetStep = 0;
}

void StepperController::stepMotor(bool dir) {
  digitalWrite(dirPin, dir);
  digitalWrite(stepPin, HIGH);
  delayMicroseconds(50);  // short reliable pulse
  digitalWrite(stepPin, LOW);
  currentStep += (dir ? 1 : -1);
}

void StepperController::stopMotor() {
  // Optional: disable motor driver here
}

bool StepperController::checkEmergency() {
  return digitalRead(emgPin) == LOW;
}

bool StepperController::checkLimit(bool dir) {
  return digitalRead(dir ? limitPin2 : limitPin1) == LOW;
}

void StepperController::getStatus(MoveMode mode, float &target, float &remaining) {
  if (mode == MoveMode::STEP) {
    target = static_cast<float>(targetStep);
    remaining = static_cast<float>(targetStep - currentStep);
  } else {
    target = targetStep * distPerStep;
    remaining = (targetStep - currentStep) * distPerStep;
  }
}

bool StepperController::isBusy() const {
  return currentStep != targetStep;
}

// ------------------ Instantiate Controller ------------------

StepperController motor(
  2.0,        // 2 steps/ms → 500 µs per step
  0.1,        // 0.1 mm per step
  33, 25,     // stepPin, dirPin
  26, 27,     // limitPin1, limitPin2
  18          // emergencyPin
);

long old_current_step = 0;

void setup() {
  Serial.begin(115200);
  motor.addCommand(MoveMode::DISTANCE, -100.0);  // Example command
}

void loop() {
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint >= 200) {
    Serial.printf("Current step: %ld\n", motor.currentStep);
    lastPrint = millis();
  }
}
