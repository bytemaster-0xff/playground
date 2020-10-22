
#ifndef OSH_NOMETADATA

#include "OSHClient.h"

namespace osh
{

void System::addSensor(Sensor* sensor)
{
    this->sensors[numSensors++] = sensor;
}


#ifndef OSH_NOXML

void System::writeXML(XMLWriter& w, bool nested)
{
    // PhysicalSystem
    w.tagStart(w.buildTagName(GML_PREFIX, ELT_SYSTEM));

    // id
    w.tagField(w.buildTagName(GML_PREFIX, ATT_ID), "SYS01");

    // namespace decl
    if (!nested)
        writeNamespaces(w);
    w.tagEnd(true, false);

    // description
    if (this->Description != NULL)
        w.writeNode(w.buildTagName(GML_PREFIX, ELT_DESCRIPTION), this->Description);

    // identifier
    if (this->UniqueID != NULL)
        w.writeNode(w.buildTagName(GML_PREFIX, ELT_IDENTIFIER), this->UniqueID);

    // name
    if (this->Name != NULL)
        w.writeNode(w.buildTagName(GML_PREFIX, ATT_NAME), this->Name);

    // output description
    w.tagOpen(w.buildTagName(SML_PREFIX, ELT_OUTPUTS));
    w.tagOpen(w.buildTagName(SML_PREFIX, ELT_OUTPUTLIST));

    for (int i = 0; i < numSensors; i++)
    {
        Sensor* s = sensors[i];
        char name[10];
        sprintf(name, "out%d", i+1);

        w.tagStart(w.buildTagName(SML_PREFIX, ELT_OUTPUT));
        w.tagField(w.buildTagName(NULL, ATT_NAME), name);
        w.tagEnd(true, false);
        s->writeOutput(w);
        w.tagClose();
    }

    w.tagClose();
    w.tagClose();

    w.tagClose(); // PhysicalSystem
}

#endif // end OSH_NOXML


} // namespace osh

#endif // OSH_NOMETADATA
