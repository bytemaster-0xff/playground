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
	float m_currentFeedRate;
	long m_totalSteps;
	int m_rampSteps;
	int m_state;
	int m_temp1;
	int m_rampDelta;
	int m_absoluteMove = true;
	float m_moveLength;
	float m_requestedFeedRate_mmSeconds;
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

	// Count down for number of steps until we accell/decel
	uint16_t m_IRQs_AccelDecelCountDown = 0;

	// Current number of steps to pause at the point in our accel/decel ramp
	uint16_t m_IRQs_AtAccelDecel = 0;	
	uint8_t m_IRQs_PerStep = 1;
	uint8_t m_IRQ_CurrentCountDown = 0;


	String m_axisName;
	
	bool m_minSwitchTripped = false;
	bool m_maxSwitchTripped = false;
	float m_currentMachineLocation;	
	int GetIRQs_PerStep();

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
	void ResetHomeLocation();
	void ClearLimitSwitches();
	void Update();
	float GetWorkspaceOffset();
	void SetWorkspaceOffset(float value);
	void SetAbsoluteCoordMove();
	void SetRelativeCoordMove();
};

#endif

