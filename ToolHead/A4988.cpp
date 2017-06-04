// 
// 
// 

#include "A4988.h"
#include <EEPROM.h>

extern void SetAlarmMode();


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

/*
 *  ISR Rate               : 50 uSec
 *  Effectice Step Rate    : 100 uSec (two IRQs per step)
 *  ISR's Per Second       : 10000 (10K)
 *  Steps per MM           : 80
 *  Steps Per Second (max) : 125 = 10,000 / 80
 *  Max/Base Fed Rate      : 125
 */

#define STATE_IDLE 0
#define STATE_ACCELERATING 1
#define STATE_MOVING 2
#define STATE_DECELERATING 3


#define DEFAULT_MM_PER_SECOND 125
#define STEPS_PER_MM 80
 //#define STEPS_AT_ACCEL_DECL 10

#define STEPS_AT_ACCEL_DECL 2

 /* We will start accelerating from this IRQs per step and decellerate back down to it as well*/
/* Start out at 100mm / min, then decement / increment by one each delta */
#define START_ENDING_FEED_RATE 15

/* Feed arrives in mm/minute*/
void A4988::Move(float cm, float feedMMMinute) {
	Enable();

	float relativePosition = (m_currentMachineLocation - m_worksetOffset);

	float delta =  m_absoluteMove ? cm - relativePosition : cm;

	m_stepsRemaining = abs(delta * STEPS_PER_MM);
	if (m_stepsRemaining == 0)
	{
		return;
	}

	m_totalSteps = m_stepsRemaining;

	m_requestedFeedRate_mmSeconds = feedMMMinute / 60.0;
	m_IRQs_PerStep = ((int)DEFAULT_MM_PER_SECOND / m_requestedFeedRate_mmSeconds);
	m_ForwardDirection = delta > 0;

	if (m_IRQs_PerStep >= 75)
	{
		/* For relatively slow moves, don't accel/decel */
		m_rampDelta = 0;
		m_IRQ_CurrentCountDown = m_IRQs_PerStep;
	}
	else
	{
		/* Starting rate for sending pulses*/
		m_IRQs_AtAccelDecel = START_ENDING_FEED_RATE;

		/* Calculate the number of steps in acceleration */
		m_rampDelta = ((m_IRQs_AtAccelDecel - m_IRQs_PerStep) * STEPS_AT_ACCEL_DECL);

		/* Used as starting point for moving*/
		m_IRQ_CurrentCountDown = START_ENDING_FEED_RATE;

		/* Count Down to when we accelerate/decelerate next*/
		m_IRQs_AccelDecelCountDown = STEPS_AT_ACCEL_DECL;
	}

	digitalWrite(m_dirPin, m_ForwardDirection ? HIGH : LOW);
	m_destinationLocation = cm + m_worksetOffset;
	m_startLocation = m_currentMachineLocation;
	m_state = STATE_IDLE;

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

void A4988::Home() {
	bool endStopHit = false;
	Enable();

	digitalWrite(m_dirPin, LOW);
	m_bHoming = true;

	while (endStopHit == false)
	{
		digitalWrite(m_stepPin, HIGH);
		delayMicroseconds(100);

		digitalWrite(m_stepPin, LOW);
		delayMicroseconds(100);

		endStopHit = digitalRead(m_minLimitPin) == LOW;
	}

	m_bHoming = false;

	m_currentMachineLocation = 0;
}

void A4988::ResetHomeLocation() {
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

void A4988::SetRelativeCoordMove()
{
	m_absoluteMove = false;
}

void A4988::SetAbsoluteCoordMove()
{
	m_absoluteMove = true;
}

int A4988::GetIRQs_PerStep()
{

	switch (m_state)
	{
		case STATE_IDLE:
			m_state = STATE_ACCELERATING;
			m_rampDelta = 0;
			break;
		case STATE_ACCELERATING:
			m_rampDelta++;

			while (m_IRQs_AccelDecelCountDown-- > 0) {
				return m_IRQs_AtAccelDecel;
			}

			m_IRQs_AccelDecelCountDown = 5;

			m_IRQs_AtAccelDecel--;
			if (m_IRQs_AtAccelDecel == m_IRQs_PerStep)
			{				
				m_state = STATE_MOVING;
			}				

			return m_IRQs_AtAccelDecel;
		case STATE_MOVING:
			if (m_stepsRemaining == m_rampDelta)
			{
				m_IRQs_AccelDecelCountDown = 5;
				m_state = STATE_DECELERATING;
			}
			
			return m_IRQs_PerStep;
		case STATE_DECELERATING:
			while (m_IRQs_AccelDecelCountDown-- > 0) {
				return m_IRQs_AtAccelDecel;
			}

			m_IRQs_AccelDecelCountDown = 5;


			m_IRQs_AtAccelDecel++;
			if (m_stepsRemaining == 0)
			{
				m_state = STATE_IDLE;
			}

			m_IRQs_AtAccelDecel = min(m_IRQs_AtAccelDecel, START_ENDING_FEED_RATE);

			return 	m_IRQs_AtAccelDecel;
	}

	return m_IRQs_PerStep;

}

void A4988::SetUpdatesPerCount(uint8_t updatesPerStep) {
	m_IRQs_PerStep = updatesPerStep;
}

void A4988::ClearLimitSwitches() {
	m_minSwitchTripped = false;
	m_maxSwitchTripped = false;
}

#define ISR_INTERVAL_uSEC 50 //50 uSeconds
#define BASE_STEP_INTERVAL ISR_INTERVAL_uSEC * 2

void A4988::Update() {
	if (!IsBusy) {
		return;
	}

	//Spin until we should toggle.
	if (m_IRQ_CurrentCountDown-- > 1) {
		return;
	}
	else {
		m_IRQ_CurrentCountDown = GetIRQs_PerStep();
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
			Serial.println(String("<EndStop:" + m_axisName + ",min>"));
			m_minSwitchTripped = true;
			SetAlarmMode();
		}
		else {
			m_bHoming = false;
			m_currentMachineLocation = 0;
		}

		IsBusy = false;
	}

	if (m_ForwardDirection && m_maxLimitPin != -1 && digitalRead(m_maxLimitPin) == LOW)
	{
		Serial.println("<EndStop:" + m_axisName + ",max>");
		IsBusy = false;
		SetAlarmMode();
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