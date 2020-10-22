//
//    FILE: XMLWriter.h
//  AUTHOR: Rob Tillaart
// VERSION: 0.1.05
//    DATE: 2013-11-06
// PURPOSE: Simple XML writer library
//
// Released to the public domain
//

#ifndef XML_WRITER_H
#define XML_WRITER_H

#include "Arduino.h"
// no pre 1.0 support!


// include pgmspace stuff so we can use strings from flash to save RAM
#ifdef ARDUINO_ARCH_AVR
#include <avr/pgmspace.h>
#endif

#ifdef ESP8266
#include <pgmspace.h>
#endif


// max length of tag stack
#ifndef XMLWRITER_MAXTAGSIZE
#define XMLWRITER_MAXTAGSIZE 31
#endif

#ifndef XMLWRITER_MAXSTACKSIZE
#define XMLWRITER_MAXSTACKSIZE 150
#endif

//#define XMLWRITER_ESCAPE_SUPPORT


class XMLWriter
{
public:
    XMLWriter(Print* stream);

    void reset();

    // to generate tag names from prefix and name stored in PROGMEM
    char* buildTagName(const char* prefix, const char* name);

    // standard XML header
    void header();

    // if multiline == true it does not indent to allow bigger text blocks
    // <!-- text -->
    void comment(const char* text, bool multiLine = false);

    // <tag>
    void tagOpen(const char* tag, bool newline = true);
    // <tag name="name">
    void tagOpen(const char* tag, const char* name, bool newline = true);
    // </tag>
    void tagClose(bool indent = true);

    // <tag
    void tagStart(const char* tag);
    // field="value"
    void tagField(const char* field, const char* value);
    //  />
    void tagEnd(bool newline = true, bool addSlash = true);

    // <tag>value</tag>
    void writeNode(const char* tag, const char* value);

    // typically 0,2,4; default == 2;
    void setIndentSize(uint8_t size);

#ifdef XMLWRITER_ESCAPE_SUPPORT
    // expands the special xml chars
    void escape(const char* str);
#endif

private:
    // outputstream
    Print* _stream;

    // for indentation
    void spaces();
    uint8_t _indent;
    uint8_t _indentStep;

    // for writing tags from PROGMEM
    char _tagName[XMLWRITER_MAXTAGSIZE];

    // current path - used to remember the current tagname to
    // automaticly close tags.
    char _tagStack[XMLWRITER_MAXSTACKSIZE];
    char* _endLastTag;
    void pushTag(const char* tag);
    char* popTag();
};

#endif
// END OF FILE
