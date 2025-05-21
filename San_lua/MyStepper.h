#pragma once
#include <ESP_FlexyStepper.h>

class MyStepper {
public:
  MyStepper(char namePrefix, int stepPin, int dirPin, int frontLimitPin, int rearLimitPin);

  void handleCommand(String cmd);

private:
  static void stepperTask(void* parameter);

  char _namePrefix;
  int _frontLimitPin;
  int _rearLimitPin;
  ESP_FlexyStepper _stepper;
  TaskHandle_t _taskHandle = nullptr;

  int _speed = 300;
  int _acceleration = 800;
};
