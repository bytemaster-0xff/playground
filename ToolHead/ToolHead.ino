

#include "A4988.h"
#define DELAY_MICRO_SECONDS 5000


#define XMAX 2
#define XMIN 3
#define YMIN 14
#define YMAX 15
#define SOLDERPASTEMAX 19
#define PNPHEADMAX 18


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
	ExpectingPowerStatus,
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
	bool HasXLocation;
	float YLocation;
	bool HasYLocation;
	float ZLocation;
	bool HasZLocation;
	float CLocation;
	bool HasCLocation;
	float Feed;
	bool PowerStatus;
	bool HasPowerStatus;
} GCodeCommand_t;


GCodeCommand_t CommandBuffer[COMMAND_BUFFER_LENGTH];

A4988 XAxis(XDIR, XSTEP, XENABLE);
A4988 YAxis(YDIR, YSTEP, YENABLE);
A4988 CAxis(ZDIR, ZSTEP, ZENABLE);
A4988 ZPlace(ZDIR, ZSTEP, ZENABLE);
A4988 ZSolder(ZDIR, ZSTEP, ZENABLE);


void initCommandBuffer()
{
	for (byte idx = 0; idx < COMMAND_BUFFER_LENGTH; ++idx)
	{
		CommandBuffer[idx].Command = -1;
		CommandBuffer[idx].CommandType = Unknown;
		CommandBuffer[idx].Feed = 5000;
		CommandBuffer[idx].XLocation = 0;
		CommandBuffer[idx].HasXLocation = 0;
		CommandBuffer[idx].YLocation = 0;
		CommandBuffer[idx].HasYLocation = 0;
		CommandBuffer[idx].ZLocation = 0;
		CommandBuffer[idx].HasZLocation = 0;
		CommandBuffer[idx].CLocation = 0;
		CommandBuffer[idx].HasCLocation = 0;
	}
}


void setup() {
	Serial.begin(115200);
	Serial.write("online");

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


	initCommandBuffer();

	parserIndex = 0;
	bufferIndex = 0;
	wordBufferIndex = 0;
	memset(buffer, 0, GCODE_LINE_BUFFER_LENGTH);
	memset(wordBuffer, 0, WORD_BUFFER_LENGTH);
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
		CommandBuffer[_commandBufferHead].HasXLocation = true;
		break;
	case ExpectingYLocation:
		CommandBuffer[_commandBufferHead].YLocation = atof(wordBuffer);
		CommandBuffer[_commandBufferHead].HasYLocation = true;
		break;
	case ExpectingZLocation:
		CommandBuffer[_commandBufferHead].ZLocation = atof(wordBuffer);
		CommandBuffer[_commandBufferHead].HasZLocation = true;
		break;
	case ExpectingCLocation:
		CommandBuffer[_commandBufferHead].CLocation = atof(wordBuffer);
		CommandBuffer[_commandBufferHead].HasCLocation = true;
		break;
	case ExpectingPowerStatus:
		CommandBuffer[_commandBufferHead].PowerStatus = atoi(wordBuffer) != 0;
		CommandBuffer[_commandBufferHead].HasPowerStatus = true;
		break;
	}

	_parserState = ExpectingWordType;
	_wordType = NotSpecified;
	wordBufferIndex = 0;
	memset(wordBuffer, 0, WORD_BUFFER_LENGTH);
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
		Serial.print("<XPOS:");
		Serial.print(XAxis.GetCurrentLocation(), 4);
		Serial.print(",YPOS:");
		Serial.print(YAxis.GetCurrentLocation(), 4);
		Serial.print(",CPOS:");
		Serial.print(CAxis.GetCurrentLocation(), 4);
		Serial.print(",PPOS:");
		Serial.print(ZPlace.GetCurrentLocation(), 4);
		Serial.print(",SPOS:");
		Serial.print(ZSolder.GetCurrentLocation(), 4);
		Serial.println(">");
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
		case 'X':
			ParseWord();
			_wordType = ExpectingXLocation;
			_parserState = PendingNextWord;
			break;
		case 'P':
			ParseWord();
			_wordType = ExpectingPowerStatus;
			_parserState = PendingNextWord;
			break;
		case 'Y':
			ParseWord();
			_wordType = ExpectingYLocation;
			_parserState = PendingNextWord;
			break;
		case 'C':
			ParseWord();
			_wordType = ExpectingCLocation;
			_parserState = PendingNextWord;
			break;
		case 'Z':
			ParseWord();
			_wordType = ExpectingZLocation;
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
			CommandBuffer[_commandBufferHead].Command = -1;
			CommandBuffer[_commandBufferHead].CommandType = Unknown;
			CommandBuffer[_commandBufferHead].Feed = CommandBuffer[_commandBufferHead == 0 ? COMMAND_BUFFER_LENGTH - 1 : _commandBufferHead - 1].Feed;
			CommandBuffer[_commandBufferHead].HasXLocation = false;
			CommandBuffer[_commandBufferHead].XLocation = 0.0f;
			CommandBuffer[_commandBufferHead].HasYLocation = false;
			CommandBuffer[_commandBufferHead].YLocation = 0.0f;
			CommandBuffer[_commandBufferHead].HasZLocation = false;
			CommandBuffer[_commandBufferHead].ZLocation = 0.0f;
			CommandBuffer[_commandBufferHead].HasCLocation = false;
			CommandBuffer[_commandBufferHead].CLocation = 0.0f;

			parserIndex = 0;
			bufferIndex = 0;
			wordBufferIndex = 0;
			memset(buffer, 0, GCODE_LINE_BUFFER_LENGTH);
		}
	}
}

#define STEP_PER_CM 10

void ProcessCommand()
{
	switch (CommandBuffer[_commandBufferTail].CommandType)
	{
		case GCode:
		{
			switch (CommandBuffer[_commandBufferTail].Command)
			{
			case 0:
			case 1: {				
				if (CommandBuffer[_commandBufferTail].HasXLocation) {
					XAxis.Move(CommandBuffer[_commandBufferTail].XLocation, CommandBuffer[_commandBufferTail].Feed);
				}

				if (CommandBuffer[_commandBufferTail].HasYLocation) {
					YAxis.Move(CommandBuffer[_commandBufferTail].YLocation, CommandBuffer[_commandBufferTail].Feed);
				}

				if (CommandBuffer[_commandBufferTail].HasZLocation) {
					if (_currentTool == 0) {
						ZPlace.Move(CommandBuffer[_commandBufferTail].ZLocation, CommandBuffer[_commandBufferTail].Feed);
					}
					else {
						ZSolder.Move(CommandBuffer[_commandBufferTail].ZLocation, CommandBuffer[_commandBufferTail].Feed);
					}
				}

				if (CommandBuffer[_commandBufferTail].HasCLocation) {
					CAxis.Move(CommandBuffer[_commandBufferTail].CLocation, CommandBuffer[_commandBufferTail].Feed);
				}

				break;
			}
			case 28:
				//while (digitalRead(MIN_LIMIT) != LOW) {
	//					Up(15000);					
		//			}

				
				break;
			}
		}
		break;
		case TCode:
			switch (CommandBuffer[_commandBufferTail].Command)
			{
			case 0: Serial.println("Setting Tool 0"); break;
			case 1: Serial.println("Setting Tool 1"); break;
			}
			break;

		case MCode:
		{
			switch (CommandBuffer[_commandBufferTail].Command)
			{
			case 60: { digitalWrite(UPPER_LIGHT, CommandBuffer[_commandBufferTail].PowerStatus ? 1 : 0); } break;
			case 61: { digitalWrite(LOWER_LIGHT, CommandBuffer[_commandBufferTail].PowerStatus ? 1 : 0); } break;
			case 62: { digitalWrite(SOLENOID, CommandBuffer[_commandBufferTail].PowerStatus ? 1 : 0); }break;
			case 119: {
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

				break;
			}

			}
			break;

		}
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
