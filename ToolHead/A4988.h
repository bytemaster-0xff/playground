// A4988.h

#ifndef _A4988_h
#define _A4988_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

class A4988 {
private:
	uint8_t m_dirPin;
	uint8_t m_stepPin;
	uint8_t m_enablePin;
	uint8_t m_currentLocation;
public:
	A4988(int dirPin, int stepPin, int enablePin);

	float GetCurrentLocation();
	void Move(float cm, float feed);
	void Enable();
	void Disable();
};

#endif

