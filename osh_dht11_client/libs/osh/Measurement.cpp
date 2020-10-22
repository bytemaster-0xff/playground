
#ifndef OSH_NOMETADATA

#include "OSHClient.h"

namespace osh
{

Measurement::Measurement()
{
    this->Type = NULL;
    this->Name = NULL;
    this->Definition = NULL;
    this->Label = NULL;
    this->Uom = NULL;
    this->RefFrame = NULL;
    this->AxisID = NULL;
}


void Measurement::writeXML(XMLWriter& w)
{
    char defUri[80];

    // Component name
    w.tagStart(w.buildTagName(SWE_PREFIX, this->Type));

    // definition
    if (this->Definition != NULL)
    {
        buildDefUrl(this->Definition, defUri);
        w.tagField(w.buildTagName(NULL, ATT_DEFINITION), defUri);
    }

    // axis ID
    if (this->AxisID != NULL)
        w.tagField(w.buildTagName(NULL, ATT_AXISID), this->AxisID);

    // ref frame
    if (this->RefFrame != NULL)
    {
        buildDefUrl(this->RefFrame, defUri);
        w.tagField(w.buildTagName(NULL, ATT_REFRAME), defUri);
    }
    w.tagEnd(true, false);

    // label
    if (this->Label != NULL)
        w.writeNode(w.buildTagName(SWE_PREFIX, ELT_LABEL), this->Label);

    // uom code or URI
    if (this->Uom != NULL)
    {
        w.tagStart(w.buildTagName(SWE_PREFIX, ELT_UOM));
        if (this->Uom == DEF_ISO8601)
        {
            buildDefUrl(this->Uom, defUri);
            w.tagField(w.buildTagName(XLINK_PREFIX, ATT_HREF), defUri);
        }
        else
            w.tagField(w.buildTagName(NULL, ATT_CODE), this->Uom);
        w.tagEnd(true, true);
    }

    w.tagClose();
}


void VectorMeas::addCoordinate(const char* axisID, const char* uom, const char* label, const char* type)
{
    Measurement* m = new Measurement();
    m->setName(axisID);
    m->setAxisID(axisID);
    m->setType((type != NULL) ? type : QUANTITY);
    m->setLabel(label);
    m->setUom(uom);
    this->coords[numCoords++] = m;
}


void VectorMeas::writeXML(XMLWriter& w)
{
    // Vector
    w.tagStart(w.buildTagName(SWE_PREFIX, ELT_VECTOR));

    // definition attribute
    char defUri[80];
    if (this->Definition != NULL)
    {
        buildDefUrl(this->Definition, defUri);
        w.tagField(w.buildTagName(NULL, ATT_DEFINITION), defUri);
    }

    // ref frame attribute
    if (this->RefFrame != NULL)
    {
        buildDefUrl(this->RefFrame, defUri);
        w.tagField(w.buildTagName(NULL, ATT_REFRAME), defUri);
    }

    w.tagEnd(true, false);

    // label
    if (this->Label != NULL)
        w.writeNode(w.buildTagName(SWE_PREFIX, ELT_LABEL), this->Label);

    // coordinates
    for (int i = 0; i < numCoords; i++)
    {
        Measurement* m = coords[i];
        w.tagStart(w.buildTagName(SWE_PREFIX, ELT_COORDINATE));
        w.tagField(w.buildTagName(NULL, ATT_NAME), m->getName());
        w.tagEnd(true, false);
        m->writeXML(w);
        w.tagClose();
    }

    w.tagClose();
}


VectorMeas::~VectorMeas()
{
    for (int i = 0; i < numCoords; i++)
        delete coords[i];
}

} // namespace osh

#endif // OSH_NOMETADATA


