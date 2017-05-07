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
	float m_destinationLocation;
	float m_startLocation;
	long m_totalSteps;
	float m_moveLength;
	long m_stepsRemaining;
	uint8_t m_lastToggleType;
	bool m_ForwardDirection;

	bool m_bHoming = false;
	float m_worksetOffset;
	int8_t m_eepromStartAddr;
	int8_t m_timer = -1;
	uint8_t m_dirPin;
	uint8_t m_stepPin;
	uint8_t m_enablePin;	
	int8_t m_maxLimitPin = -1;
	int8_t m_minLimitPin = -1;
	int8_t m_updatesPerCount = 1;
	int8_t m_updatesCount = 0;
	String m_axisName;
	
	bool m_minSwitchTripped = false;
	bool m_maxSwitchTripped = false;
	float m_currentMachineLocation;	

public:
	A4988(int dirPin, int stepPin, int enablePin, String axiSName, int eepromStartAddr);

	bool IsBusy = false;

	float GetCurrentLocation();
	void Move(float cm, float feed);
	void SetMinLimitPin(uint8_t maxLimit);
	void SetMaxLimitPin(uint8_t maxLimit);
	void SetUpdatesPerCount(uint8_t updatesPerStep);
	void Enable();
	void Kill();
	void Disable();
	void Home();
	void ClearLimitSwitches();
	void Update();
	float GetWorkspaceOffset();
	void SetWorkspaceOffset(float value);
};

#endif

