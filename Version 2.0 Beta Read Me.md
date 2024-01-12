# This branch is for development and testing of PulseSensor Playground Library v2.0

High-level goals for v2.0:

- Remove user requirement for defining USE ARDUINO INTERRUPTS

Files effected:

- All example sketches will need to accomodate changes
- Interrupts.h will be superceded
- PulseSensorPlayground.h and PulseSensorPlayground.cpp will need to accomodate changes
- All comments in all files will be reviewed and editied to reflect paradigm shift


Notes 12/6/23
It works!
The file SelectTimer.h contains the most up to date hardware architecture that 
has hardware timer interrupt support.

## TO DO:

- Make adjustments to example sketches to accomodate library decisions
- Make a program PulseSensorFullSystem.ino to make hardware testing easy
- Test ATtiny85 using Digispark board






