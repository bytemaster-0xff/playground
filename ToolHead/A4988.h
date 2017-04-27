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
	long m_stepsRemaining;
	uint8_t m_lastToggleType;

	uint8_t m_dirPin;
	uint8_t m_stepPin;
	uint8_t m_enablePin;	
	uint8_t m_maxLimitPin = -1;
	uint8_t m_minLimitPin = -1;
	String m_axisName;
	bool m_isBusy = false;
	bool m_minSwitchTripped = false;
	bool m_maxSwitchTripped = false;
	float m_currentLocation;
	void Update();


public:
	A4988(int dirPin, int stepPin, int enablePin, String axiSName);

	float GetCurrentLocation();
	void Move(float cm, float feed);
	void SetMinLimitPin(uint8_t maxLimit);
	void SetMaxLimitPin(uint8_t maxLimit);
	void Enable();
	void Disable();
	void Home();
	void ClearLimitSwitches();
	bool GetIsBusy();
};

#endif

