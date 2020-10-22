
#ifndef OSH_NOMETADATA

#include "OSHClient.h"

namespace osh
{


#ifndef OSH_NOXML

void Device::toXML(Print& out)
{
    XMLWriter w(&out);
    w.header();
    writeXML(w, false);
}

void Device::writeNamespaces(XMLWriter& w)
{
#ifndef OSH_NOXMLNS
    char nsUri[40];

    buildUrl(OGC_NS_PREFIX, SML_NS, nsUri);
    char tagName[6];
    strcpy_P(tagName, XMLNS_PREFIX);
    w.tagField(tagName, nsUri);

    buildUrl(OGC_NS_PREFIX, SWE_NS, nsUri);
    w.tagField(w.buildTagName(XMLNS_PREFIX, SWE_PREFIX), nsUri);

    buildUrl(OGC_NS_PREFIX, GML_NS, nsUri);
    w.tagField(w.buildTagName(XMLNS_PREFIX, GML_PREFIX), nsUri);

    strcpy_P(nsUri, XLINK_NS);
    w.tagField(w.buildTagName(XMLNS_PREFIX, XLINK_PREFIX), nsUri);
#endif
}

#endif // OSH_NOXML



#ifndef OSH_NOJSON

void Device::toJSON(Print& out)
{
    // TODO
}

#endif // OSH_NOJSON


} // namespace osh

#endif // OSH_NOMETADATA
