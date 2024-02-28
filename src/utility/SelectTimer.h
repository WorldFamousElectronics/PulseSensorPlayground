/*
	We will use pre-processor directives to determine the target hardware architecture
	and based on that we will define USE_HARDWARE_TIMER true or false accordingly.
	
	In the event that hardware timer interrupts are supported, 
	USE_HARDWARE_TIMER will be defined true, and the compilation process will proceed without warnings.

	In the event that hardware timer interrupts are not supported,
	USE_HARDWARE_TIMER will be defined false, and a warning saying this will be printed during compiliation.

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

		ARDUINO_ARCH_RENESAS
			RA4M1 Core-M4
				Arduino UNO R4 Minima
				Arduino UNO R4 Wifi

		ARDUINO_SAM_DUE
			AT91SAM3X8E Core-M3
				Ardino DUE

		ARDUINO_ARCH_RP2040
			RP2040
				Raspi Pico
				Adafruit Feather RP2040
				Any board with RP2040

		ARDUINO_NRF52_ADAFRUIT
			nRF52840 Bluetooth Family
				Adafruit and Seeed nRF52 boards (non EMBED boards yet!)

		ARDUINO_ARCH_ESP32
			ESP32 Family
				Adafruit Feather ESP32-S2
				Needs testing on other family members

		ARDUINO_ARCH_SAMD
			ATSAMD21 Core-M0
				Adafruit Feather M0
			ATSAMD51 Core-M4
				Adafruit Meather M4

		ARDUINO_ARCH_ESP8266
			ESP8266 Family
			Adafruit Feather Huzzah


*/

#ifndef SELECT_TIMER_H
#define SELECT_TIMER_H

#if defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_RENESAS) || defined(ARDUINO_ARCH_RP2040)\
|| defined(ARDUINO_SAM_DUE) || defined(ARDUINO_NRF52_ADAFRUIT) || defined(ARDUINO_ARCH_ESP32)\
|| defined(ARDUINO_SAMD_ZERO) || defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_ESP8266)
#define USE_HARDWARE_TIMER true
			// #warning "Using hardware timer to sample PulseSensor."
#else
#define USE_HARDWARE_TIMER false
			#warning "Using software timer to sample PulseSensor."
#endif

// Macros to link to interrupt disable/enable only if they exist
#if defined ARDUINO_ARCH_AVR
#define DISABLE_PULSE_SENSOR_INTERRUPTS cli()
#define ENABLE_PULSE_SENSOR_INTERRUPTS sei()
#else
#define DISABLE_PULSE_SENSOR_INTERRUPTS
#define ENABLE_PULSE_SENSOR_INTERRUPTS
#endif

#endif // SELECT_TIMER include guard