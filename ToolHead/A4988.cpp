// 
// 
// 

#include "A4988.h"
#include <EEPROM.h>


#define EEPROM_WORKSPACEOFFSET 0 

A4988::A4988(int dirPin, int stepPin, int enablePin, String axisName, int eepromStartAddr) {
	pinMode(dirPin, OUTPUT);
	pinMode(stepPin, OUTPUT);
	pinMode(enablePin, OUTPUT);

	m_stepPin = stepPin;
	m_dirPin = dirPin;
	m_enablePin = enablePin;
	m_axisName = axisName;

	EEPROM.get(eepromStartAddr + EEPROM_WORKSPACEOFFSET, m_worksetOffset);
	if (isnan(m_worksetOffset)) {
		m_worksetOffset = 0.0f;
	}
	m_eepromStartAddr = eepromStartAddr;

	Disable();
}

// FROM: http://www.instructables.com/id/Arduino-Timer-Interrupts/


void A4988::SetMinLimitPin(uint8_t minlimitPin)
{
	m_minLimitPin = minlimitPin;
}

void A4988::SetMaxLimitPin(uint8_t maxLimitPin)
{
	m_maxLimitPin = maxLimitPin;
}

void A4988::SetISRTimer(uint8_t timer) {
	m_timer = timer;
}

void A4988::Kill() {
	m_stepsRemaining = 0;
	IsBusy = false;
}

void A4988::Enable() {
	digitalWrite(m_enablePin, LOW);
}

void A4988::Disable() {
	digitalWrite(m_enablePin, HIGH);
}

//#define LOG_MOVE


void A4988::Move(float cm, float feed) {
	Enable();

	float relativePosition = (m_currentMachineLocation - m_worksetOffset);

	float delta = cm - relativePosition;

	if (delta == 0)
	{
		return;
	}

	m_ForwardDirection = delta > 0;

	digitalWrite(m_dirPin, m_ForwardDirection ? HIGH : LOW);
	m_destinationLocation = cm + m_worksetOffset;
	m_startLocation = m_currentMachineLocation;


	m_stepsRemaining = abs(delta * 80);
	m_totalSteps = m_stepsRemaining;
	IsBusy = true;
#ifdef LOG_MOVE
	Serial.print(String("MOVE:"));
	Serial.print(cm);
	Serial.print(",");
	Serial.print(relativePosition);
	Serial.print(",");
	Serial.print(m_worksetOffset);
	Serial.print(",");
	Serial.print(delta);
	Serial.print(",");
	Serial.println(m_stepsRemaining);
#endif
}

void A4988::Home(){
	bool endStopHit = false;
	Enable();

	digitalWrite(m_dirPin, LOW);
	m_bHoming = true;

	while (endStopHit == false)
	{
		digitalWrite(m_stepPin, HIGH);
		delayMicroseconds(250);

		digitalWrite(m_stepPin, LOW);
		delayMicroseconds(250);

		endStopHit = digitalRead(m_minLimitPin) == LOW;
	}

	m_bHoming = false;

	m_currentMachineLocation = 0;
}

float A4988::GetWorkspaceOffset() {
	return m_worksetOffset;
}

void A4988::SetWorkspaceOffset(float value)
{
	m_worksetOffset = GetCurrentLocation() - value;
	EEPROM.put(m_eepromStartAddr + EEPROM_WORKSPACEOFFSET, m_worksetOffset);
}


void A4988::ClearLimitSwitches() {
	m_minSwitchTripped = false;
	m_maxSwitchTripped = false;
}

void A4988::Update() {
	if (!IsBusy) {
		return;
	}

	if (m_totalSteps > 0) {
		float percentComplete = (m_totalSteps - m_stepsRemaining) / m_totalSteps;
		float distanceMoved = m_moveLength * percentComplete;
		if (m_ForwardDirection) {
			m_currentMachineLocation += distanceMoved;
		}
		else {
			m_currentMachineLocation -= distanceMoved;
		}
	}

	if (!m_ForwardDirection && m_minLimitPin != -1 && digitalRead(m_minLimitPin) == LOW)
	{
		if (!m_bHoming) {
			Serial.println(String("<EndStop:min," + m_axisName + ">"));
			m_minSwitchTripped = true;
		}
		else {
			m_bHoming = false;
			m_currentMachineLocation = 0;
		}

		IsBusy = false;
	}

	if (m_ForwardDirection && m_maxLimitPin != -1 && digitalRead(m_maxLimitPin) == LOW)
	{
		Serial.println("<EndStop:max," + m_axisName + ">");
		IsBusy = false;
	}

	if (m_lastToggleType == LOW) {
		digitalWrite(m_stepPin, HIGH);
		m_lastToggleType = HIGH;
	}
	else if (m_lastToggleType == HIGH) {
		digitalWrite(m_stepPin, LOW);
		if (!m_bHoming) {
			m_stepsRemaining--;
		}

		m_lastToggleType = LOW;	
	}

	if (m_stepsRemaining == 0) {
		IsBusy = false;
		m_currentMachineLocation = m_destinationLocation;
	}	
}

float A4988::GetCurrentLocation() {
	return m_currentMachineLocation;
}