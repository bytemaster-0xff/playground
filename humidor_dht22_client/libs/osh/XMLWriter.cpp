//
//    FILE: XMLWriter.cpp
//  AUTHOR: Rob Tillaart
// VERSION: 0.1.05
//    DATE: 2013-11-06
// PURPOSE: Simple XML library
//
// HISTORY:
// 0.1.00 - 2013-11-06 initial version
// 0.1.01 - 2013-11-07 rework interfaces
// 0.1.02 - 2013-11-07 +setIndentSize(), corrected history, +escape support
// 0.1.03 - 2015-03-07 refactored - footprint + interface
// 0.1.04 - 2015-05-21 refactored - reduce RAM -> used F() macro etc.
// 0.1.05 - 2015-05-23 added XMLWRITER_MAXTAGSIZE 15 (to support KML coordinates tag)
// 0.1.06 - 2016-02-20 added method to build tag names from PROGMEM
//          2016-02-22 mod to use a zero separated, flat tag stack for better memory efficiency
//
// Released to the public domain
//

#include "XMLWriter.h"

XMLWriter::XMLWriter(Print* stream)
{
    _stream = stream;
    reset();
}

void XMLWriter::reset()
{
    _indent = 0;
    _indentStep = 2;
    _tagStack[0] = 0;
    _endLastTag = _tagStack+1;
}

char* XMLWriter::buildTagName(const char* prefix, const char* name)
{
#ifndef OSH_NOXMLNS
    if (prefix != NULL && strlen_P(prefix) > 0)
    {
        strcpy_P(_tagName, prefix);
        strcat(_tagName, ":");
        strcat_P(_tagName, name);
    }
    else
        strcpy_P(_tagName, name);
#else
    strcpy_P(_tagName, name);
#endif
    return _tagName;
};

inline void XMLWriter::pushTag(const char* tag)
{
    strcpy(_endLastTag, tag);
    _endLastTag += strlen(tag)+1;
}


inline char* XMLWriter::popTag()
{
    _endLastTag -= 2;
    while (*_endLastTag != 0)
        _endLastTag--;
    return ++_endLastTag;
}

void XMLWriter::header()
{
    _stream->println(F("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"));
}

void XMLWriter::comment(const char* text, bool multiLine)
{
    _stream->println();
    if (!multiLine)
        spaces();
    _stream->print(F("<!-- "));
    if (multiLine)
        _stream->println();
    _stream->print(text);
    if (multiLine)
        _stream->println();
    _stream->println(F(" -->"));
}

void XMLWriter::tagOpen(const char* tag, bool newline)
{
    tagOpen(tag, "", newline);
}

void XMLWriter::tagOpen(const char* tag, const char* name, bool newline)
{
    // TODO STACK GUARD
    tagStart(tag);
    if (name[0] != 0)
        tagField("name", name);
    tagEnd(newline, false);
}

void XMLWriter::tagClose(bool indent)
{
    _indent -= _indentStep;
    if (indent)
        spaces();
    _stream->print(F("</"));
    _stream->print(popTag());
    _stream->println(F(">"));
}

void XMLWriter::tagStart(const char *tag)
{
    pushTag(tag);
    spaces();
    _stream->print('<');
    _stream->print(tag);
    _indent += _indentStep;
}

void XMLWriter::tagField(const char *field, const char* str)
{
    _stream->print(' ');
    _stream->print(field);
    _stream->print(F("=\""));
#ifdef XMLWRITER_ESCAPE_SUPPORT
    escape(str);
#else
    _stream->print(str);
#endif
    _stream->print('"');
}

void XMLWriter::tagEnd(bool newline, bool addSlash)
{
    if (addSlash)
    {
        _stream->print('/');
        _indent -= _indentStep;
        popTag();
    }

    _stream->print('>');
    if (newline)
        _stream->println();
}

void XMLWriter::writeNode(const char* tag, const char* str)
{
    tagOpen(tag, "", false);
#ifdef XMLWRITER_ESCAPE_SUPPORT
    escape(str);
#else
    _stream->print(str);
#endif
    tagClose(false);
}

void XMLWriter::setIndentSize(uint8_t size)
{
    _indentStep = size;
}

////////////////////////////////////////////////////////////////////

void XMLWriter::spaces()
{
    for (uint8_t i = _indent; i > 0; i--)
        _stream->print(' ');
}

#ifdef XMLWRITER_ESCAPE_SUPPORT
char c[6] = "\"\'<>&";
char expanded[][7] =
{   "&quot;", "&apos;","&lt;","&gt;","&amp;"}; // todo in flash

void XMLWriter::escape(const char* str)
{
    char* p = str;
    while(*p != 0)
    {
        char* q = strchr(c, *p);
        if (q == NULL) _stream->print(*p);
        else _stream->print(expanded[q - c]); // uint8_t idx = q-c;
        p++;
    }
}
#endif

// END OF FILE
