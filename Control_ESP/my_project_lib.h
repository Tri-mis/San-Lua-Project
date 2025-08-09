#pragma once
#include <Arduino.h>
#include "myStepper.h"

extern MyStepper bed_stepper;
extern MyStepper box_stepper;

void handleSerialInput(bool &systemStart);