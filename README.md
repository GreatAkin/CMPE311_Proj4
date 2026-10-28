CMPE 311 Project 4: PWM Motor & Cyclic Executive Task Manager

Description

This project implements a 5-task, round-robin Cyclic Executive on an Arduino Uno R3. It features a completely bare-metal, custom timing infrastructure (myMilli()) utilizing Timer 2 and a custom ISR to avoid third-party IP. The system asynchronously manages two independently blinking LEDs (via Serial Monitor input) alongside a PWM-driven DC motor fan. The fan's 6-step speed cycle (OFF -> LOW -> MED -> HIGH -> MED -> LOW -> OFF) is controlled by a non-blocking, software-debounced push button.

Repository Contents

PROJECT_ADDMOTOR.ino: Main application containing the 5 tasks, state machine, and dispatcher.

myMilli.h: Custom header containing physical memory-mapped register pointers and the Timer 2 ISR.

ECN_0.3.pdf: Engineering Change Note documenting the addition of the motor and button.

CMPE311_ADDMOTOR_Proj4.pdf: System Design Document and justified CFM/sound level test calculations.


Compile and Run

Open PROJECT_ADDMOTOR.ino in the Arduino IDE (ensure myMilli.h is in the same tab/folder).

Select Arduino Uno as the board and choose the correct COM port.

Upload to the board.

Open the Serial Monitor at 9600 baud to configure LED intervals.

Press the physical push button to cycle the fan speeds asynchronously.
