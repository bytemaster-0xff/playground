
#include <EEPROM.h>
#include "A4988.h"
#define DELAY_MICRO_SECONDS 5000


#define XMAX 2
#define XMIN 3
#define YMIN 14
#define YMAX 15
#define SOLDERPASTEMAX 18
#define PNPHEADMAX 19

#define XENABLE 38
#define YENABLE A2
#define ZENABLE A8
#define E1ENABLE 30
#define E0ENABLE 24

#define XSTEP A0
#define XDIR A1
#define YSTEP A6
#define YDIR A7

#define ZSTEP 46
#define ZDIR 48


#define E0STEP 26
#define E0DIR 28
#define E1STEP 36
#define E1DIR 34

#define UPPER_LIGHT 9
#define LOWER_LIGHT 8
#define SOLENOID 10

#define COMMAND_BUFFER_LENGTH 20
#define GCODE_LINE_BUFFER_LENGTH 50
#define WORD_BUFFER_LENGTH 10

#include "A4988.h"

byte bufferIndex;
/* A full GCode Commands see GetCommand for understanding on buffer overflow */
byte buffer[GCODE_LINE_BUFFER_LENGTH];

byte wordBufferIndex;
/* For each value that could get set, store that in a string prior to parsing*/
char wordBuffer[WORD_BUFFER_LENGTH];

byte parserIndex;

enum ParserState
{
	ReadingWord, /* Reading content, any characters 0-9 and .*/
	ExpectingWordType, /* Reading G, M, F, Z */
	PendingNextWord /* Read a Space [ ], will wait for the next character to set the state */
};

enum WordTypes
{
	ExpectingGCode,
	ExpectingMCode,
	ExpectingTCode,
	ExpectingHome,
	ExpectingFeed,
	ExpectingXLocation,
	ExpectingYLocation,
	ExpectingZLocation,
	ExpectingCLocation,
	ExpectingILocation,
	ExpectingJLocation,
	ExpectingSParameter,
	ExpectingPParameter,
	NotSpecified,
};

enum CommandTypes
{
	GCode,
	TCode,
	MCode,
	Unknown
};

int _currentTool = 0;
int _hasPart = 0;

ParserState _parserState;
WordTypes _wordType;
double _word;
int _intCmd;
byte _commandBufferHead;
byte _commandBufferTail;

typedef struct GCodeCommand
{
	CommandTypes CommandType;
	byte Command;
	float XLocation;

	bool HasXParameter;
	float YLocation;
	bool HasYParameter;
	float ZLocation;
	bool HasZParameter;
	float CLocation;
	bool HasCParameter;
	float ELocatoin;
	bool HasELocation;

	short SParameter;
	bool HasSParameter;

	short PParameter;
	bool HasPParameter;

	float IParameter;
	bool HasIParameter;

	float JParameter;
	bool HasJParameter;

	float Feed;
} GCodeCommand_t;


GCodeCommand_t CommandBuffer[COMMAND_BUFFER_LENGTH];

A4988 XAxis(XDIR, XSTEP, XENABLE, "X", 100);
A4988 YAxis(YDIR, YSTEP, YENABLE, "Y", 132);
A4988 CAxis(ZDIR, ZSTEP, ZENABLE, "C", 164);
A4988 ZPlace(E0DIR, E0STEP, E0ENABLE, "PLACE", 196);
A4988 ZPaste(E1DIR, E1STEP, E1ENABLE, "SOLDER", 228);

void ResetWord(int idx) {
	CommandBuffer[idx].Command = -1;
	CommandBuffer[idx].CommandType = Unknown;
	CommandBuffer[idx].Feed = 5000;
	CommandBuffer[idx].XLocation = 0;
	CommandBuffer[idx].HasXParameter = false;
	CommandBuffer[idx].YLocation = 0;
	CommandBuffer[idx].HasYParameter = false;
	CommandBuffer[idx].ZLocation = 0;
	CommandBuffer[idx].HasZParameter = false;
	CommandBuffer[idx].CLocation = 0;
	CommandBuffer[idx].HasCParameter = false;
	CommandBuffer[idx].IParameter = 0;
	CommandBuffer[idx].HasIParameter = false;
	CommandBuffer[idx].JParameter = 0;
	CommandBuffer[idx].HasJParameter = false;
	CommandBuffer[idx].PParameter = 0;
	CommandBuffer[idx].HasPParameter = false;
	CommandBuffer[idx].SParameter = 0;
	CommandBuffer[idx].HasSParameter = false;
}


void initCommandBuffer()
{
	for (byte idx = 0; idx < COMMAND_BUFFER_LENGTH; ++idx)
	{
		ResetWord(idx);
	}
}

void EnableTimer2() {
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
}

void setup() {
	Serial.begin(115200);
	Serial.write("online");

	XAxis.SetMaxLimitPin(XMAX);
	XAxis.SetMinLimitPin(XMIN);
	XAxis.SetISRTimer(1);
	YAxis.SetMaxLimitPin(YMAX);
	YAxis.SetMinLimitPin(YMIN);
	YAxis.SetISRTimer(2);

	ZPlace.SetMinLimitPin(PNPHEADMAX);
	ZPaste.SetMinLimitPin(SOLDERPASTEMAX);

	// put your setup code here, to run once:
	pinMode(UPPER_LIGHT, OUTPUT);
	pinMode(LOWER_LIGHT, OUTPUT);
	pinMode(SOLENOID, OUTPUT);

	pinMode(XMAX, INPUT);
	pinMode(XMIN, INPUT);
	pinMode(YMAX, INPUT);
	pinMode(YMIN, INPUT);
	pinMode(SOLDERPASTEMAX, INPUT);
	pinMode(PNPHEADMAX, INPUT);

	EnableTimer2();

	initCommandBuffer();

	parserIndex = 0;
	bufferIndex = 0;
	wordBufferIndex = 0;
	memset(buffer, 0, GCODE_LINE_BUFFER_LENGTH);
	memset(wordBuffer, 0, WORD_BUFFER_LENGTH);
}

ISR(TIMER2_COMPA_vect) {
	XAxis.Update();
	YAxis.Update();
	ZPaste.Update();
	ZPlace.Update();
	CAxis.Update();
}

void ParseWord()
{
	if (_parserState == NotSpecified)
	{
		Serial.println("Unkown Word Type");
		return;
	}

	if (_parserState == ExpectingWordType)
	{
		Serial.print("Expecting Word Type");
		return;
	}

	switch (_wordType)
	{
	case ExpectingMCode:
		CommandBuffer[_commandBufferHead].CommandType = MCode;
		CommandBuffer[_commandBufferHead].Command = atoi(wordBuffer);
		break;
	case ExpectingGCode:
		CommandBuffer[_commandBufferHead].CommandType = GCode;
		CommandBuffer[_commandBufferHead].Command = atoi(wordBuffer);
		break;
	case ExpectingTCode:
		CommandBuffer[_commandBufferHead].CommandType = TCode;
		CommandBuffer[_commandBufferHead].Command = atoi(wordBuffer);
		break;
	case ExpectingFeed:
		CommandBuffer[_commandBufferHead].Feed = atof(wordBuffer);
		break;
	case ExpectingXLocation:
		CommandBuffer[_commandBufferHead].XLocation = atof(wordBuffer);
		break;
	case ExpectingYLocation:
		CommandBuffer[_commandBufferHead].YLocation = atof(wordBuffer);
		break;
	case ExpectingZLocation:
		CommandBuffer[_commandBufferHead].ZLocation = atof(wordBuffer);
		break;
	case ExpectingCLocation:
		CommandBuffer[_commandBufferHead].CLocation = atof(wordBuffer);
		break;
	case ExpectingPParameter:
		CommandBuffer[_commandBufferHead].PParameter = atoi(wordBuffer);
		CommandBuffer[_commandBufferHead].HasPParameter = true;
		break;
	case ExpectingSParameter:
		CommandBuffer[_commandBufferHead].SParameter = atoi(wordBuffer);
		CommandBuffer[_commandBufferHead].HasSParameter = true;
		break;
	case ExpectingILocation:
		CommandBuffer[_commandBufferHead].IParameter = atoi(wordBuffer);
		CommandBuffer[_commandBufferHead].HasIParameter = true;
		break;
	case ExpectingJLocation:
		CommandBuffer[_commandBufferHead].JParameter = atoi(wordBuffer);
		CommandBuffer[_commandBufferHead].HasSParameter = true;
		break;
	}

	_parserState = ExpectingWordType;
	_wordType = NotSpecified;
	wordBufferIndex = 0;
	memset(wordBuffer, 0, WORD_BUFFER_LENGTH);
}


bool IsCurrentCommandMovement() {
	return(CommandBuffer[_commandBufferHead].CommandType == GCode &&
		(CommandBuffer[_commandBufferHead].Command == 0 ||
			CommandBuffer[_commandBufferHead].Command == 1 ||
			CommandBuffer[_commandBufferHead].Command == 2 ||
			CommandBuffer[_commandBufferHead].Command == 3));

}


void GetCommand()
{
	int ch = Serial.read();
	if (ch == -1)
	{
		return;
	}

	//	Serial.print("IN LOOP: ");
		//Serial.println((char) ch);

	if (ch == '?')
	{
		Serial.print("<Idle,");
		Serial.print("MPos:");
		Serial.print(XAxis.GetCurrentLocation(), 4);
		Serial.print(",");
		Serial.print(YAxis.GetCurrentLocation(), 4);
		Serial.print(",");
		Serial.print(ZPlace.GetCurrentLocation(), 4);
		Serial.print(",");
		Serial.print(ZPaste.GetCurrentLocation(), 4);
		Serial.print(",");
		Serial.print(CAxis.GetCurrentLocation(), 4);
		Serial.print(",");
		Serial.print("WPos:");
		Serial.print(XAxis.GetWorkspaceOffset(), 4);
		Serial.print(",");
		Serial.print(YAxis.GetWorkspaceOffset(), 4);
		Serial.print(",");
		Serial.print(ZPlace.GetWorkspaceOffset(), 4);
		Serial.print(",");
		Serial.print(ZPaste.GetWorkspaceOffset(), 4);
		Serial.print(",");
		Serial.print(CAxis.GetWorkspaceOffset(), 4);
		Serial.print(",");
		Serial.print("T:");
		Serial.print(_currentTool);
		Serial.print(",");
		Serial.print("P:");
		Serial.print(_hasPart);
		Serial.println(">");
		return;
	}
	if (ch == '!' || ch == 0x18) {
		XAxis.Kill();
		YAxis.Kill();
		ZPlace.Kill();
		ZPaste.Kill();
		CAxis.Kill();
		return;
	}

	/* If we aren't parsing a message and we receive a '\n' then bail*/
	if (ch == '\n' && bufferIndex == 0) {
		return;
	}


	buffer[bufferIndex++] = ch;
	//Read in exacly one line starting with a G or an M and ending with the final character of the GCode Line
	while (ch != '\n' && ch != '?' && ch != -1)
	{
		ch = Serial.read();
		if (ch != -1)
		{
			if (ch == 'G' || ch == 'M')
			{
				bufferIndex = 0;
			}

			buffer[bufferIndex++] = ch;
			if (bufferIndex == GCODE_LINE_BUFFER_LENGTH)
			{
				bufferIndex = 0;
			}
		}
	}

	/* Set the Feed and Z Location to the prior GCode Command */

	bool completedCommand = false;
	bool invalidCommand = false;
	/* Go through the entire list and parse out values...likely not that strong*/
	while (parserIndex < bufferIndex)
	{
		switch (buffer[parserIndex])
		{
		case '\n':
			ParseWord();
			completedCommand = true;
			break;
		case ' ':
			ParseWord();
			_parserState = PendingNextWord;
			break;

		case '-': case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': case '.':
			wordBuffer[wordBufferIndex++] = buffer[parserIndex];
			break;
		case 'P':
			ParseWord();
			_wordType = ExpectingPParameter;
			_parserState = PendingNextWord;
			break;
		case 'S':
			ParseWord();
			_wordType = ExpectingSParameter;
			_parserState = PendingNextWord;
			break;
		case 'I':
			ParseWord();
			_wordType = ExpectingILocation;
			_parserState = PendingNextWord;
			break;
		case 'J':
			ParseWord();
			_wordType = ExpectingJLocation;
			_parserState = PendingNextWord;
			break;
		case 'X':
			ParseWord();
			CommandBuffer[_commandBufferHead].HasXParameter = true;
			if (IsCurrentCommandMovement()) {
				_wordType = ExpectingXLocation;
			}

			_parserState = PendingNextWord;
			break;
		case 'L':
			/* TODO This likely needs some updates */
			_parserState = PendingNextWord;
			break;
		case 'Y':
			ParseWord();
			CommandBuffer[_commandBufferHead].HasYParameter = true;
			if (IsCurrentCommandMovement()) {
				_wordType = ExpectingYLocation;
			}

			_parserState = PendingNextWord;
			break;
		case 'C':
			ParseWord();
			CommandBuffer[_commandBufferHead].HasCParameter = true;
			if (IsCurrentCommandMovement()) {
				_wordType = ExpectingCLocation;				
			}

			_parserState = PendingNextWord;
			break;
		case 'Z':
			ParseWord();
			CommandBuffer[_commandBufferHead].HasZParameter = true;
			if (IsCurrentCommandMovement()) {
				_wordType = ExpectingZLocation;				
			}

			_parserState = PendingNextWord;
			break;
		case 'F':
			ParseWord();
			_wordType = ExpectingFeed;
			_parserState = PendingNextWord;
			break;
		case 'G':
			_wordType = ExpectingGCode;
			_parserState = PendingNextWord;
			break;
		case 'M':
			_wordType = ExpectingMCode;
			_parserState = PendingNextWord;
			break;
		case 'T':
			_wordType = ExpectingTCode;
			_parserState = PendingNextWord;
			break;

		}
		parserIndex++;
		if (parserIndex == GCODE_LINE_BUFFER_LENGTH)
		{
			Serial.println("<error:002>");
			return;
		}
	}

	if (invalidCommand)
	{
		Serial.println("<error:001>");
	}
	else if (completedCommand)
	{
		if (CommandBuffer[_commandBufferHead].CommandType == Unknown) {
			Serial.println("<error:004>");
		}
		else if (CommandBuffer[_commandBufferHead].Command == -1) {
			Serial.println("<error:005>");
		}
		else {
			_commandBufferHead++;
			if (_commandBufferHead == COMMAND_BUFFER_LENGTH)
			{
				_commandBufferHead = 0;
			}

			if (_commandBufferHead == _commandBufferTail)
			{
				Serial.println("<error:003>");
			}


			/* Set Defaults for Next Command Buffer */
			ResetWord(_commandBufferHead);
			CommandBuffer[_commandBufferHead].Feed = CommandBuffer[_commandBufferHead == 0 ? COMMAND_BUFFER_LENGTH - 1 : _commandBufferHead - 1].Feed;

			parserIndex = 0;
			bufferIndex = 0;
			wordBufferIndex = 0;
			memset(buffer, 0, GCODE_LINE_BUFFER_LENGTH);
		}
	}
}

void EnableMotors() {
	GCodeCommand current = CommandBuffer[_commandBufferTail];

	if (current.HasXParameter || current.HasYParameter || current.HasZParameter || current.HasCParameter)
	{
		if (current.HasXParameter) XAxis.Enable();
		if (current.HasYParameter) YAxis.Enable();
		if (current.HasCParameter) CAxis.Enable();
		if (current.HasZParameter) {
			switch (_currentTool) {
			case 0: ZPlace.Enable(); break;
			case 1: ZPaste.Enable(); break;
			}

		}
	}
	else {
		XAxis.Enable();
		YAxis.Enable();
		CAxis.Enable();
		ZPlace.Enable();
		ZPaste.Enable();
	}
}

void DisableMotors() {
	GCodeCommand current = CommandBuffer[_commandBufferTail];

	if (current.HasXParameter || current.HasYParameter || current.HasZParameter || current.HasCParameter)
	{
		if (current.HasXParameter) XAxis.Disable();
		if (current.HasYParameter) YAxis.Disable();
		if (current.HasCParameter) CAxis.Disable();
		if (current.HasZParameter) {
			switch (_currentTool) {
			case 0: ZPlace.Enable(); break;
			case 1: ZPaste.Enable(); break;
			}
		}
	}
	else {
		XAxis.Disable();
		YAxis.Disable();
		CAxis.Disable();
		ZPlace.Disable();
		ZPaste.Disable();
	}
}

void Home() {
	GCodeCommand current = CommandBuffer[_commandBufferTail];

	if (current.HasXParameter || current.HasYParameter || current.HasZParameter)
	{
		if (current.HasXParameter) {
			XAxis.Home();
		}
		if (current.HasYParameter) {
			YAxis.Home();
		}
		if (current.HasZParameter) {
			if (_currentTool == 0) {
				ZPlace.Home();
			}
			else {
				ZPaste.Home();
			}
		}
	}
	else
	{
		XAxis.Home();
		YAxis.Home();
		ZPlace.Home();
		ZPaste.Home();
	}
}

void LinearMove() {
	GCodeCommand current = CommandBuffer[_commandBufferTail];

	if (current.HasXParameter) {
		XAxis.Move(current.XLocation, current.Feed);
	}

	if (current.HasYParameter) {
		YAxis.Move(current.YLocation, current.Feed);
	}

	if (current.HasZParameter) {
		if (_currentTool == 0) {
			ZPlace.Move(current.ZLocation, current.Feed);
		}
		else {
			ZPaste.Move(current.ZLocation, current.Feed);
		}
	}

	if (current.HasCParameter) {
		CAxis.Move(current.CLocation, current.Feed);
	}

	/* Axis get configurated in IRQ, should still do SerialData input check. */
	while (XAxis.IsBusy || YAxis.IsBusy || CAxis.IsBusy || ZPaste.IsBusy || ZPlace.IsBusy) {
		GetCommand();
	}
}

void SetWorkspaceLocation() {
	GCodeCommand current = CommandBuffer[_commandBufferTail];

	if (current.HasXParameter) {
		XAxis.SetWorkspaceOffset(current.XLocation);
	}

	if (current.HasYParameter) {
		YAxis.SetWorkspaceOffset(current.YLocation);
	}

	if (current.HasCParameter){
		CAxis.SetWorkspaceOffset(current.CLocation);
	}

	if (current.HasZParameter) {
		if (_currentTool == 0) {
			ZPlace.SetWorkspaceOffset(current.CLocation);						
		}
		else if(_currentTool == 1) {
			ZPaste.SetWorkspaceOffset(current.CLocation);
		}
	}
}

void ArcMove() {
	GCodeCommand current = CommandBuffer[_commandBufferTail];
}

void Dwell() {
	GCodeCommand current = CommandBuffer[_commandBufferTail];
}

void CheckEndStops() {
	Serial.print("x-min: ");
	Serial.println(digitalRead(XMIN) == LOW ? "hit" : "not hit");
	Serial.print("x-max: ");
	Serial.println(digitalRead(XMAX) == LOW ? "hit" : "not hit");
	Serial.print("y-min: ");
	Serial.println(digitalRead(YMIN) == LOW ? "hit" : "not hit");
	Serial.print("y-max: ");
	Serial.println(digitalRead(YMAX) == LOW ? "hit" : "not hit");
	Serial.print("placez-min: ");
	Serial.println(digitalRead(SOLDERPASTEMAX) == LOW ? "hit" : "not hit");
	Serial.print("solderz-min: ");
	Serial.println(digitalRead(PNPHEADMAX) == LOW ? "hit" : "not hit");
}

void ProcessCommand()
{
	GCodeCommand current = CommandBuffer[_commandBufferTail];
	switch (current.CommandType)
	{
	case GCode:
		switch (current.Command)
		{
		case 0:
		case 1: LinearMove();
			break;
		case 2:
		case 3: ArcMove();
			break;
		case 4: Dwell(); break;
		case 10: SetWorkspaceLocation(); break;
		case 28: Home(); break;
		}
		break;

	case TCode:
		switch (CommandBuffer[_commandBufferTail].Command)
		{
		case 0: _currentTool = 0; break;
		case 1: _currentTool = 1; break;
		}
		break;

	case MCode:
		switch (CommandBuffer[_commandBufferTail].Command)
		{
		case 17: EnableMotors(); break;
		case 18: DisableMotors(); break;
		case 60: { digitalWrite(UPPER_LIGHT, CommandBuffer[_commandBufferTail].PParameter == 0 ? LOW : HIGH); } break;
		case 61: { digitalWrite(LOWER_LIGHT, CommandBuffer[_commandBufferTail].PParameter == 0 ? LOW : HIGH); } break;
		case 62: { digitalWrite(SOLENOID, CommandBuffer[_commandBufferTail].PParameter == 0 ? LOW : HIGH); } break;
		case 119: CheckEndStops(); break;
		}
		break;

	}

	Serial.println("ok");

	_commandBufferTail++;

	if (_commandBufferTail == COMMAND_BUFFER_LENGTH)
	{
		_commandBufferTail = 0;
	}

}


void loop() {
	GetCommand();

	if (_commandBufferTail != _commandBufferHead)
	{
		ProcessCommand();
	}
	/*  // put your main code here, to run repeatedly:
	  */
}
