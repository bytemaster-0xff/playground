// 
// 
// 

#include "A4988.h"


A4988::A4988(int dirPin, int stepPin, int enablePin, String axisName) {
	pinMode(dirPin, OUTPUT);
	pinMode(stepPin, OUTPUT);
	pinMode(enablePin, OUTPUT);

	m_stepPin = stepPin;
	m_dirPin = dirPin;
	m_enablePin = enablePin;
	m_axisName = axisName;

	Disable();
}

// FROM: http://www.instructables.com/id/Arduino-Timer-Interrupts/
void A4988::EnalableIRQ(int uSec) {
	if (m_timer == 1) {
		TCCR1A = 0;// set entire TCCR1A register to 0
		TCCR1B = 0;// same for TCCR1B
		TCNT1 = 0;//initialize counter value to 0
				  // set compare match register for 1hz increments
		OCR1A = 249;// = (16*10^6) / (8000*4) - 1 (must be <256) /* 124 = 16
					// turn on CTC mode
		TCCR1B |= (1 << WGM12);
		// Set CS12 and CS10 bits for 1024 prescaler
		TCCR1B |= (1 << CS21);
		// enable timer compare interrupt
		TIMSK1 |= (1 << OCIE1A);
		m_isBusy = true;
		Serial.println("START Timer1");
	}
	else if (m_timer == 2) {
		TCCR2A = 0;// set entire TCCR2A register to 0
		TCCR2B = 0;// same for TCCR2B
		TCNT2 = 0;//initialize counter value to 0
				  // set compare match register for 8khz increments
		OCR2A = 249;// = (16*10^6) / (8000*8) - 1 (must be <256)
					// turn on CTC mode
		TCCR2A |= (1 << WGM21);
		// Set CS21 bit for 8 prescaler
		TCCR2B |= (1 << CS21);
		// enable timer compare interrupt
		TIMSK2 |= (1 << OCIE2A);
		m_isBusy = true;
		Serial.println("START Timer2");
	}
}

void A4988::DisableIRQ() {
	if (m_timer == 1) {
		TIMSK1 &= ~(1 << OCIE1A);
		m_isBusy = false;
		m_currentLocation = m_destinationCM;
	}
	else if (m_timer == 2) {
		TIMSK2 &= ~(1 << OCIE2A);
		m_isBusy = false;
		m_currentLocation = m_destinationCM;
	}

}

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
	DisableIRQ();
}

void A4988::Enable() {
	digitalWrite(m_enablePin, LOW);
}

void A4988::Disable() {
	digitalWrite(m_enablePin, HIGH);
}

bool A4988::GetIsBusy()
{
	return m_isBusy;
}

void A4988::Move(float cm, float feed) {
	Enable();

	float delta = cm - m_currentLocation;

	Serial.print(m_axisName);
	Serial.print(" ");
	Serial.print(cm);
	Serial.print(" ");
	Serial.println(delta);

	m_ForwardDirection = delta > 0;

	digitalWrite(m_dirPin, m_ForwardDirection ? HIGH : LOW);

	m_stepsRemaining = abs(delta * 80);

	Serial.print(m_axisName);
	Serial.print(" ");
	Serial.print(cm);
	Serial.print(" ");
	Serial.println(m_stepsRemaining);

	if (m_timer == -1) {
		while (m_stepsRemaining > 0) {
			digitalWrite(m_stepPin, HIGH);
			delayMicroseconds(150);
			digitalWrite(m_stepPin, LOW);
			delayMicroseconds(150);

			if (!m_ForwardDirection && m_minLimitPin != -1 && digitalRead(m_minLimitPin) == LOW)
			{
				m_stepsRemaining = 0;
				Serial.println(m_axisName);
				Serial.println(" ");
				Serial.println("MIN-LIMIT-HIT");
				m_minSwitchTripped = true;
			}

			if (m_ForwardDirection && m_maxLimitPin != -1 && digitalRead(m_maxLimitPin) == LOW)
			{
				m_stepsRemaining = 0;
				Serial.println(m_axisName);
				Serial.println(" ");
				Serial.println("MAX-LIMIT-HIT");
				m_maxSwitchTripped = true;
			}
		}

		m_currentLocation = cm;
	}
	else {
		m_destinationCM = cm;
		EnalableIRQ(150);
	}


}

void A4988::Home()
{
	bool endStopHit = false;
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

	m_currentLocation = 0;
}

void A4988::ClearLimitSwitches() {
	m_minSwitchTripped = false;
	m_maxSwitchTripped = false;
}

void A4988::Update() {
	if (m_lastToggleType == LOW) {
		digitalWrite(m_stepPin, HIGH);
		m_lastToggleType = HIGH;
		if (m_stepsRemaining == 0) {
			DisableIRQ();
		}
	}
	else if (m_lastToggleType == HIGH) {
		digitalWrite(m_stepPin, LOW);
		m_stepsRemaining--;
		m_lastToggleType = LOW;
		if (m_stepsRemaining == 0) {
			DisableIRQ();
		}
	}
	if (!m_ForwardDirection && m_minLimitPin != -1 && digitalRead(m_minLimitPin) == LOW)
	{
		Serial.println(m_axisName);
		Serial.println(" ");
		Serial.println("MIN-LIMIT-HIT");
		m_minSwitchTripped = true;
	}

	if (m_ForwardDirection && m_maxLimitPin != -1 && digitalRead(m_maxLimitPin) == LOW)
	{
		Serial.println(m_axisName);
		Serial.println(" ");
		Serial.println("MAX-LIMIT-HIT");
		m_maxSwitchTripped = true;
	}
}

float A4988::GetCurrentLocation() {
	return m_currentLocation;
}