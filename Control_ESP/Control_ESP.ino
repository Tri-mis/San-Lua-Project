#include <Arduino.h>
#include "myStepper.h"
#include "my_project_lib.h"

#define A 2000 // box vibrate
#define B 2500 // open lid
#define C 5800 // close lid
#define D 6300 // stop box vibrate
#define E 7300 // bed vibrate
#define BOX_OPEN 400
#define BOX_CLOSE 150
#define BOX_VIBRATE_SPEED 150


enum class State{
  START,
  HOMING,
  MOVE_BED_TO_A,
  VIBRATE_BOX,
  MOVE_BED_TO_B,
  OPEN_LID,
  MOVE_BED_TO_C,
  CLOSE_LID,
  MOVE_BED_TO_D,
  STOP_VIBRATE_BOX,
  MOVE_BED_TO_E,
  VIBRATE_BED,
  END,
};

State state = State::START;

void setup() {
  Serial.begin(115200);
  bed_stepper.beginTask();
  box_stepper.beginTask();
  bed_stepper.setSpeed(1000);
  box_stepper.setSpeed(500);
  ledcAttach(18, 5000, 10);
}

bool system_start = false;

void loop() 
{
  handleSerialInput(system_start);

  if (system_start)
  {
    switch (state){
        case State::START:
          bed_stepper.addCommand(StepperCommand(StepperMode::HOMING, 0));
          box_stepper.addCommand(StepperCommand(StepperMode::HOMING, 0));
          state = State::HOMING;
          delay(100);
          break;
        case State::HOMING:
          if (bed_stepper.isHomed && box_stepper.isHomed){
            box_stepper.addCommand(StepperCommand(StepperMode::NORMAL_RUN, BOX_CLOSE));
            bed_stepper.addCommand(StepperCommand(StepperMode::NORMAL_RUN, A));
            state = State::MOVE_BED_TO_A;
            delay(100);
          }
          break;
        case State::MOVE_BED_TO_A:
          if (bed_stepper.reachedTarget){
            ledcWrite(18, BOX_VIBRATE_SPEED);
            bed_stepper.addCommand(StepperCommand(StepperMode::NORMAL_RUN, B));
            state = State::MOVE_BED_TO_B;
            delay(100);
          }
          break;
        case State::MOVE_BED_TO_B:
          if (bed_stepper.reachedTarget){
            box_stepper.addCommand(StepperCommand(StepperMode::NORMAL_RUN, BOX_OPEN));
            bed_stepper.addCommand(StepperCommand(StepperMode::NORMAL_RUN, C));
            state = State::MOVE_BED_TO_C;
            delay(100);
          }
          break;
        case State::MOVE_BED_TO_C:
          if (bed_stepper.reachedTarget){
            box_stepper.addCommand(StepperCommand(StepperMode::NORMAL_RUN, BOX_CLOSE));
            bed_stepper.addCommand(StepperCommand(StepperMode::NORMAL_RUN, D));
            state = State::MOVE_BED_TO_D;
            delay(100);
          }
          break;
        case State::MOVE_BED_TO_D:
          if (bed_stepper.reachedTarget){
            ledcWrite(18, 0);
            bed_stepper.addCommand(StepperCommand(StepperMode::NORMAL_RUN, E));
            state = State::MOVE_BED_TO_E;
          }
          break;
        case State::MOVE_BED_TO_E:
          if (bed_stepper.reachedTarget){
            bed_stepper.addCommand(StepperCommand(StepperMode::SET_SPEED, 2000));
            bed_stepper.addCommand(StepperCommand(StepperMode::VIBRATE, 15, 500));
            state = State::VIBRATE_BED;
            delay(100);
          }
          break;
        case State::VIBRATE_BED:
          if (bed_stepper.doneVibrate){
            bed_stepper.addCommand(StepperCommand(StepperMode::SET_SPEED, 1000));
            state = State::END;
            delay(100);
          }
          break;
        default:
          system_start = false;
          break;
      }
  }
  
}