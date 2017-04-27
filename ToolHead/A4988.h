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
	float m_destinationCM;
	long m_stepsRemaining;
	uint8_t m_lastToggleType;
	bool m_ForwardDirection;

	bool m_bHoming = false;

	uint8_t m_timer = -1;
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
	void EnalableIRQ(int uSec);
	void DisableIRQ();

public:
	A4988(int dirPin, int stepPin, int enablePin, String axiSName);

	float GetCurrentLocation();
	void Move(float cm, float feed);
	void SetMinLimitPin(uint8_t maxLimit);
	void SetMaxLimitPin(uint8_t maxLimit);
	void Enable();
	void Kill();
	void Disable();
	void SetISRTimer(uint8_t timer);
	void Home();
	void ClearLimitSwitches();
	bool GetIsBusy();
	void Update();
};

#endif

