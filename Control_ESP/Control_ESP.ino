#include <Arduino.h>

class MyStepper {
public:
  int step_pin = 0;
  int dir_pin = 0;
  
  int target_step = 0;
  int current_step = 0;

  int vibrate_time = 0;
  int target_vibrate_time = 0;
  int vibrate_amplitude = 0;

  float speed = 0.5f; // step per millisecond
  unsigned long step_delay_micros;
  unsigned long last_step_time = 0;

  TaskHandle_t taskHandle = NULL;

  MyStepper(int step_pin, int dir_pin)
    : step_pin(step_pin), dir_pin(dir_pin) {
    step_delay_micros = (unsigned long)(1000.0f / speed);
    pinMode(step_pin, OUTPUT);
    pinMode(dir_pin, OUTPUT);
  }

  void set_speed(float new_speed) {
    this->speed = new_speed;
    this->step_delay_micros = (unsigned long)(1000.0f / speed);
  }

  void set_vibration(int vibrate_amplitude, int vibrate_times) {
    this->vibrate_amplitude = vibrate_amplitude;
    this->target_vibrate_time = vibrate_times;
    this->vibrate_time = 0; // reset when starting new vibration task
  }

  void set_target_step(int target_step) {
    this->target_step = target_step;
  }

  void stepMotor(bool dir) {
    digitalWrite(dir_pin, dir);
    digitalWrite(step_pin, HIGH);
    delayMicroseconds(50);
    digitalWrite(step_pin, LOW);

    current_step += dir ? 1 : -1;
  }

  void run() {
    int diff = target_step - current_step;
    bool dir = diff > 0;

    if (diff == 0 && vibrate_time < target_vibrate_time) {
      if ((vibrate_time % 2) == 0)
        target_step = current_step + vibrate_amplitude;
      else
        target_step = current_step - vibrate_amplitude;

      vibrate_time++;
    }

    if (diff != 0) {
      unsigned long current_step_time = micros();
      if (current_step_time - last_step_time >= step_delay_micros) {
        stepMotor(dir);
        last_step_time = current_step_time;
      }
    }
  }

  void start_task() {
    xTaskCreatePinnedToCore(
      [](void* param) {
        MyStepper* stepper = static_cast<MyStepper*>(param);
        for (;;) {
          stepper->run();
          delayMicroseconds(100);
        }
      },
      NULL,    
      4096,        
      this,        
      1,           
      &taskHandle, 
      1       
    );
  }
};


MyStepper myStepper_A(33, 25);

void setup() {
  Serial.begin(115200);
  myStepper_A.start_task();

  myStepper_A.set_speed(0.05);
  myStepper_A.set_target_step(1000);
  myStepper_A.set_vibration(1, 500);
}

int current_step = 0;
int old_step = 0;


void loop() {

  current_step = myStepper_A.current_step;

  if (current_step != old_step)
  {
    Serial.println("current_step: " + String(current_step));
    old_step = current_step;
  }

}
