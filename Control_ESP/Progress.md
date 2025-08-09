================================================= 21:00 - 31/7 ================================================= 
- Today, I wrote a class called MyStepper. 
- I need to add: limit switch, emergency switch, mechanism to handle emergency pressed or limit switch pressed (I think I sould reset the parameters when there is an emergency)
- I need to add homing.
- I need to reorganise the library

================================================= 21:37 - 01/8 ================================================= 
- I think now I finished the stepper class, I can fully control it like so:
    - BED/BOX_HOME_0 or BED_HOME_1: homing the bed/box motor in the backward (0) or forward (1) direction, the motor will run in the corresponding direction and when it reached the limit switch, it set the current step to 0
    - BED/BOX_STEP_1000: Moving the box/bed motor by 1000 step (the box motor does not have the front limit switch so I temporarily assign pin 23. Because it does not have the limit switch, always run homing first and never let the motor run over 150 step from 0)
    - BED/BOX_SPEED_100: Set the speed of the bed/box motor to 100 step per millisecond
        - the speed of the bed motor for normal moving should be 1000 (it should be different for vibration), slower speed will cause alot of vibration, and higher speed cause alot of jerk
        - the speed of the box motor should be a constant 100 step per millisecond and should be set early in the code.
    - BED_VIBRATE_2_1000: Vibrate the bed motor 1000 times with the amplitude of 2 step
        - Before calling vibration, we should set the speed of the bed motor to around 500 per second, that will make:
            - Vibrate at 2 step amplitude makes the rice cluster in
            - Vibrate at 3 step amplitude makes the rice spread out
        - I dont think I ever need to vibrate the box motor.
    
- There is also an emergency pin, for the time being, the emergency pin for both stepper is pin GPIO22. When this pin is pull down, the motor jump out of its action and return to idle state
- The stepper action are blocking, meaning it can only do the second action after finishing the first action, except they are stoped by the emergency pin or limit switch, then the action is interrupt and terminate imidiately.
- There are some more things to do: 
    - recode to learn
    - adding the control of the DC motor to vibrate the box.
    - combine all the actuator actions together so that the rice can get spread out evenly.