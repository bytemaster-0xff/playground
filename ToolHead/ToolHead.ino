

#define DELAY_MICRO_SECONDS 5000


#define XMAX 2
#define XMIN 3
#define YMIN 14
#define YMAX 15
#define ZMIN 18
#define ZMAX 19


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
#define SOLINOID 10


#define COMMAND_BUFFER_LENGTH 20
#define GCODE_LINE_BUFFER_LENGTH 50
#define WORD_BUFFER_LENGTH 10

#include <Stepper.h>

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
	ExpectingHome,
	ExpectingFeed,
	ExpectingZLocation,
	NotSpecified,
};

enum CommandTypes
{
	GCode,
	MCode,
	Unknown
};


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
	float ZLocation;
	float Feed;
} GCodeCommand_t;


GCodeCommand_t CommandBuffer[COMMAND_BUFFER_LENGTH];

float _zLocation;


void initCommandBuffer()
{
	for (byte idx = 0; idx < COMMAND_BUFFER_LENGTH; ++idx)
	{
		CommandBuffer[idx].Command = -1;
		CommandBuffer[idx].CommandType = Unknown;
		CommandBuffer[idx].Feed = 5000;
		CommandBuffer[idx].ZLocation = 0;
	}
}



void setup() {
	Serial.begin(115200);
	Serial.write("online");

	// put your setup code here, to run once:
	pinMode(XENABLE, OUTPUT);
	pinMode(YENABLE, OUTPUT);
	pinMode(ZENABLE, OUTPUT);
	pinMode(E1ENABLE, OUTPUT);
	pinMode(E0ENABLE, OUTPUT);

	pinMode(XSTEP, OUTPUT);
	pinMode(XDIR, OUTPUT);

	pinMode(YSTEP, OUTPUT);
	pinMode(YDIR, OUTPUT);

	pinMode(ZSTEP, OUTPUT);
	pinMode(ZDIR, OUTPUT);

	pinMode(E0STEP, OUTPUT);
	pinMode(E0DIR, OUTPUT);

	pinMode(E1STEP, OUTPUT);
	pinMode(E1DIR, OUTPUT);

	pinMode(UPPER_LIGHT, OUTPUT);
	pinMode(LOWER_LIGHT, OUTPUT);
	pinMode(SOLINOID, OUTPUT);


	pinMode(XMAX, INPUT);
	pinMode(XMIN, INPUT);
	pinMode(YMAX, INPUT);
	pinMode(YMIN, INPUT);
	pinMode(ZMAX, INPUT);
	pinMode(ZMIN, INPUT);


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
	case ExpectingFeed:
		CommandBuffer[_commandBufferHead].Feed = atof(wordBuffer);
		break;
	case ExpectingZLocation:
		CommandBuffer[_commandBufferHead].ZLocation = atof(wordBuffer);
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
		Serial.print("<XLOC:");
		Serial.print(_zLocation, 4);
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
			CommandBuffer[_commandBufferHead].ZLocation = CommandBuffer[_commandBufferHead == 0 ? COMMAND_BUFFER_LENGTH - 1 : _commandBufferHead - 1].ZLocation;

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
			float delta = CommandBuffer[_commandBufferTail].ZLocation - _zLocation;

			int stepCount = abs(delta * STEP_PER_CM);
			Serial.println(CommandBuffer[_commandBufferTail].ZLocation);
			Serial.println(delta);
			Serial.println(stepCount);

			if (delta > 0) {
				while (stepCount-- > 0) {
					//Down(CommandBuffer[_commandBufferTail].Feed);
				}
			}
			else {
				while (stepCount-- > 0) {
					//Up(CommandBuffer[_commandBufferTail].Feed);
				}
			};


			_zLocation = CommandBuffer[_commandBufferTail].ZLocation;

			break;
		}
		case 28:
			//while (digitalRead(MIN_LIMIT) != LOW) {
//					Up(15000);					
	//			}

			_zLocation = 0;
			break;
		}
	}
	break;

	case MCode:
	{
		switch (CommandBuffer[_commandBufferTail].Command)
		{

		case 1: {
			digitalWrite(UPPER_LIGHT, 0);
			break;
		}
		case 2: {
			digitalWrite(UPPER_LIGHT, 1);
			break;
		}
		case 101: {

		}
		case 102: {

		}
		case 119: {
			Serial.print("z-min: ");
			Serial.println(digitalRead(ZMIN) == LOW ? "hit" : "not hit");
			Serial.print("z-max: ");
			Serial.println(digitalRead(ZMAX) == LOW ? "hit" : "not hit");
			break;
		}

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
