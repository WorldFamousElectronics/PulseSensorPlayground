/*
	We will use pre-processor directives to determine the target hardware architecture
	and based on that we will define USE_HARDWARE_TIMER true or false accordingly.
	
	In the event that hardware timer interrupts are supported, 
	USE_HARDWARE_TIMER will be defined true, and the compilation process will proceed without warnings.

	In the event that hardware timer interrupts are not supported,
	USE_HARDWARE_TIMER will be defined false, and a warning saying this will be presented in Arduino IDE.

	If the target hardware is supported by a specific library example,
	the library will send an error in the IDE that directs the user to the exact library file name to use.

	These are the different architectures, and the boards that they cover.

		ARDUINO_ARCH_AVR
			ATmega328P
				Arduino UNO, Arduino Duemilinova, Arduino Pro Mini, Adafruit Metro, 
			ATmega32u4, ATmega16u4
				Leonardo, Lily Pad, Sparkfun Pro Micro, Adafruit Feather 32u4, etc. 
			ATmega168
				Arduino NG (Vintage Arduino)
			ATmega2560, ATmega1280
				Arduino Mega
			ATtiny85
				Digispark, Adafruit Trinket (old version), etc 


*/

#ifndef SELECT_TIMER_H
#define SELECT_TIMER_H


#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_RENESAS) || defined(ARDUINO_ARCH_RP2040)\
|| defined(ARDUINO_SAM_DUE) || defined(ARDUINO_NRF52_ADAFRUIT)
#define USING_HARDWARE_TIMER true
#else
#define USING_HARDWARE_TIMER false
#endif



#endif // SELECT_TIMER include guard