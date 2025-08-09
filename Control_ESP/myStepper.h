// myStepper.h
#pragma once
#include <Arduino.h>
#include <queue>

enum class StepperMode {
    IDLE,
    NORMAL_RUN,
    HOMING,
    VIBRATE,
    SET_SPEED
};

struct StepperCommand {
    StepperMode mode;
    int param1;
    int param2;

    StepperCommand(StepperMode mode, int param1 = 0, int param2 = 0)
        : mode(mode), param1(param1), param2(param2) {}
};

class MyStepper {
private:
    String stepperName;

    int stepPin, dirPin;
    int frontLimitPin, rearLimitPin, emergencyPin;

    volatile int currentStep = 0;
    volatile float speed = 1.0f; // steps per millisecond
    unsigned long stepDelayMicros;

    std::queue<StepperCommand> commandQueue;
    TaskHandle_t taskHandle = nullptr;

public:
    bool reachedTarget;
    bool isHomed;
    bool doneVibrate;

private:
    bool checkFrontLimit();
    bool checkRearLimit();
    bool checkEmergency();

    void doStep(bool dir);
    void moveToTarget(int targetStep);
    void homing(bool towardFront);
    void vibrate(int amplitude, int times);
    static void taskLoop(void* param);

public:
    MyStepper(int stepPin, int dirPin, int frontLimitPin, int rearLimitPin, int emergencyPin, String stepperName);
    void beginTask();
    void setSpeed(float newSpeed);
    void addCommand(StepperCommand cmd);
    int getCurrentStep() const;
    float getSpeed() const;
};
