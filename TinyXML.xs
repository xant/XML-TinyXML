#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <txml.h>

#include "const-c.inc"

extern int TXML_ALLOW_MULTIPLE_ROOTNODES;

MODULE = XML::TinyXML        PACKAGE = XML::TinyXML        

INCLUDE: const-xs.inc

int 
TXML_ALLOW_MULTIPLE_ROOTNODES(__value = NO_INIT)
    UV __value
    PROTOTYPE: $
    CODE:
    RETVAL = TXML_ALLOW_MULTIPLE_ROOTNODES;
    if (items > 0) {
        TXML_ALLOW_MULTIPLE_ROOTNODES = __value;
    }
    OUTPUT:
    RETVAL


XmlNamespace *
XmlCreateNamespace(nsName, nsUri)
    char *nsName
    char *nsUri

void
XmlDestroyNamespace(ns)
    XmlNamespace *ns

XmlNamespace *
XmlGetNamespaceByName(node, nsName)
    XmlNode *node
    char *nsName

XmlNamespace *
XmlGetNamespaceByUri(node, nsUri)
    XmlNode *node
    char *nsUri

XmlNamespace *
XmlAddNamespace(node, nsName, nsUri)
    XmlNode *node
    char *nsName
    char *nsUri

XmlNamespace *
XmlGetNodeNamespace(node)
    XmlNode *node

XmlErr
XmlSetNodeNamespace(node, ns)
    XmlNode *node
    XmlNamespace *ns

XmlErr
XmlSetNodeCNamespace(node, ns)
    XmlNode *node
    XmlNamespace *ns

void
XmlSetOutputEncoding(xml, encoding)
    TXml *xml
    char *encoding

int
XmlAddAttribute(node, name, val)
    XmlNode *node
    char *name
    char *val


XmlNodeAttribute *
XmlGetAttributeByName(node, name)
    XmlNode *node
    char *name

XmlNodeAttribute *
XmlGetAttribute(node, index)
    XmlNode *node
    unsigned long index

int
XmlRemoveAttribute(node, index)
    XmlNode *node
    unsigned long index

void XmlClearAttributes(node)
    XmlNode *node

int
XmlAddChildNode(parent, child)
    XmlNode *parent
    XmlNode *child

XmlNode *
XmlNextSibling(node)
    XmlNode *node

XmlNode *
XmlPrevSibling(node)
    XmlNode *node

int
XmlAddRootNode(xml, node)
    TXml *xml
    XmlNode *node

unsigned long
XmlCountAttributes(node)
    XmlNode *node

unsigned long
XmlCountBranches(xml)
    TXml *xml

unsigned long
XmlCountChildren(node)
    XmlNode *node

TXml *
XmlCreateContext()

void
XmlResetContext(xml)
    TXml *xml

void
XmlDestroyContext(xml)
    TXml *xml

XmlNode *
XmlCreateNode(name, val, parent = NULL)
    char *name
    char *val
    XmlNode *parent

void
XmlDestroyNode(node)
    XmlNode *node

SV *
XmlDump(xml)
    TXml *xml
    PREINIT:
    char *dump;
    int outlen;
    SV   *sv = &PL_sv_undef;
    CODE:
    dump = XmlDump(xml, &outlen);
    if (dump) {
        sv = newSVpv(dump, outlen);
        free(dump);
    }
    RETVAL = sv;
    OUTPUT:
    RETVAL

char *
XmlDumpBranch(xml, rNode, depth)
    TXml *xml
    XmlNode *rNode
    unsigned int    depth

XmlNode *
XmlGetBranch(xml, index)
    TXml *xml
    unsigned long    index

XmlNode *
XmlGetChildNode(node, index)
    XmlNode *node
    unsigned long    index

XmlNode *
XmlGetChildNodeByName(node, name)
    XmlNode *node
    char *name

XmlNode *
XmlGetNode(xml, path)
    TXml *xml
    char *path

char *
XmlGetNodeValue(node)
    XmlNode *node

int
XmlParseBuffer(xml, buf)
    TXml *xml
    char *buf

int
XmlParseFile(xml, path)
    TXml *xml
    char *path

int
XmlRemoveBranch(xml, index)
    TXml *xml
    unsigned long    index

int
XmlRemoveNode(xml, path)
    TXml *xml
    char *path

int
XmlSave(xml, path)
    TXml *xml
    char *path

int
XmlSetNodeValue(node, val)
    XmlNode *node
    char *val

int
XmlSubstBranch(xml, index, newBranch)
    TXml *xml
    unsigned long    index
    XmlNode *newBranch

MODULE = XML::TinyXML        PACKAGE = XmlNamespace

XmlNamespace *
_to_ptr(THIS)
    XmlNamespace THIS = NO_INIT
    PROTOTYPE: $
    CODE:
    if (sv_derived_from(ST(0), "XmlNamespace")) {
        STRLEN len;
        char *s = SvPV((SV*)SvRV(ST(0)), len);
        if (len != sizeof(THIS))
        croak("Size %d of packed data != expected %d",
            len, sizeof(THIS));
        RETVAL = (XmlNamespace *)s;
    }
    else
        croak("THIS is not of type XmlNamespace");
    OUTPUT:
    RETVAL

XmlNamespace
new(CLASS)
    char *CLASS = NO_INIT
    PROTOTYPE: $
    CODE:
    Zero((void*)&RETVAL, sizeof(RETVAL), char);
    OUTPUT:
    RETVAL

MODULE = XML::TinyXML        PACKAGE = XmlNamespacePtr        

char *
name(THIS, __value = NO_INIT)
    XmlNamespace *THIS
    char *__value
    PROTOTYPE: $;$
    CODE:
    if (items > 1) {
        if(THIS->name)
        free(THIS->name);
        THIS->name = __value;
    }
    RETVAL = THIS->name;
    OUTPUT:
    RETVAL

char *
uri(THIS, __value = NO_INIT)
    XmlNamespace *THIS
    char *__value
    PROTOTYPE: $;$
    CODE:
    if (items > 1) {
        if(THIS->uri)
        free(THIS->uri);
        THIS->uri = __value;
    }
    RETVAL = THIS->uri;
    OUTPUT:
    RETVAL


MODULE = XML::TinyXML        PACKAGE = XmlNodeAttribute        

XmlNodeAttribute *
_to_ptr(THIS)
    XmlNodeAttribute THIS = NO_INIT
    PROTOTYPE: $
    CODE:
    if (sv_derived_from(ST(0), "XmlNodeAttribute")) {
        STRLEN len;
        char *s = SvPV((SV*)SvRV(ST(0)), len);
        if (len != sizeof(THIS))
        croak("Size %d of packed data != expected %d",
            len, sizeof(THIS));
        RETVAL = (XmlNodeAttribute *)s;
    }
    else
        croak("THIS is not of type XmlNodeAttribute");
    OUTPUT:
    RETVAL

XmlNodeAttribute
new(CLASS)
    char *CLASS = NO_INIT
    PROTOTYPE: $
    CODE:
    Zero((void*)&RETVAL, sizeof(RETVAL), char);
    OUTPUT:
    RETVAL

MODULE = XML::TinyXML        PACKAGE = XmlNodeAttributePtr        

char *
name(THIS, __value = NO_INIT)
    XmlNodeAttribute *THIS
    char *__value
    PROTOTYPE: $;$
    CODE:
    if (items > 1) {
        if(THIS->name)
        free(THIS->name);
        THIS->name = __value;
    }
    RETVAL = THIS->name;
    OUTPUT:
    RETVAL

char *
value(THIS, __value = NO_INIT)
    XmlNodeAttribute *THIS
    char *__value
    PROTOTYPE: $;$
    CODE:
    if (items > 1) {
        if(THIS->value)
        free(THIS->value);
        THIS->value = __value;
    }
    RETVAL = THIS->value;
    OUTPUT:
    RETVAL

XmlNode *
node(THIS)
    XmlNodeAttribute *THIS
    PROTOTYPE: $
    CODE:
    RETVAL = THIS->node;
    OUTPUT:
    RETVAL

MODULE = XML::TinyXML        PACKAGE = XmlNode        

XmlNode *
_to_ptr(THIS)
    XmlNode THIS = NO_INIT
    PROTOTYPE: $
    CODE:
    if (sv_derived_from(ST(0), "XmlNode")) {
        STRLEN len;
        char *s = SvPV((SV*)SvRV(ST(0)), len);
        if (len != sizeof(THIS))
        croak("Size %d of packed data != expected %d",
            len, sizeof(THIS));
        RETVAL = (XmlNode *)s;
    }
    else
        croak("THIS is not of type XmlNode");
    OUTPUT:
    RETVAL

XmlNode
new(CLASS)
    char *CLASS = NO_INIT
    PROTOTYPE: $
    CODE:
    Zero((void*)&RETVAL, sizeof(RETVAL), char);
    OUTPUT:
    RETVAL

MODULE = XML::TinyXML        PACKAGE = XmlNodePtr        

char *
path(THIS)
    XmlNode *THIS
    PROTOTYPE: $;$
    CODE:
    /*if (items > 1)
        THIS->path = __value; */
    RETVAL = THIS->path;
    OUTPUT:
    RETVAL

char *
name(THIS, __value = NO_INIT)
    XmlNode *THIS
    char *__value
    PROTOTYPE: $;$
    CODE:
    if (items > 1) {
        if(THIS->name)
        free(THIS->name);
        THIS->name = __value;
    }
    RETVAL = THIS->name;
    OUTPUT:
    RETVAL

XmlNode *
parent(THIS, __value = NO_INIT)
    XmlNode *THIS
    struct __XmlNode *__value
    PROTOTYPE: $;$
    CODE:
    if (items > 1)
        THIS->parent = __value;
    RETVAL = THIS->parent;
    OUTPUT:
    RETVAL

char *
value(THIS, __value = NO_INIT)
    XmlNode *THIS
    char *__value
    PROTOTYPE: $;$
    CODE:
    if (items > 1)
        XmlSetNodeValue(THIS, __value);
    RETVAL = THIS->value;
    OUTPUT:
    RETVAL

int
type(THIS, __value = NO_INIT)
    XmlNode *THIS
    int __value
    PROTOTYPE: $;$
    CODE:
    if (items > 1)
        THIS->type = __value;
    RETVAL = THIS->type;
    OUTPUT:
    RETVAL

AV *
knownNamespaces(THIS)
    XmlNode *THIS
    PROTOTYPE: $
    PREINIT:
    XmlNamespaceSet *item;
    AV *namespaces;
    CODE:
    namespaces = newAV();
    TAILQ_FOREACH(item, &THIS->knownNamespaces, next) {
        SV *ns = newRV_noinc(newSViv(item->ns));
        HV* st = gv_stashpv("XmlNamespacePtr", 0);
        av_push(namespaces, sv_bless(ns, st));
    }
    RETVAL = namespaces;
    OUTPUT:
    RETVAL

MODULE = XML::TinyXML        PACKAGE = TXml        

TXml *
_to_ptr(THIS)
    TXml THIS = NO_INIT
    PROTOTYPE: $
    CODE:
    if (sv_derived_from(ST(0), "TXml")) {
        STRLEN len;
        char *s = SvPV((SV*)SvRV(ST(0)), len);
        if (len != sizeof(THIS))
        croak("Size %d of packed data != expected %d",
            len, sizeof(THIS));
        RETVAL = (TXml *)s;
    }
    else
        croak("THIS is not of type TXml");
    OUTPUT:
    RETVAL

TXml
new(CLASS)
    char *CLASS = NO_INIT
    PROTOTYPE: $
    CODE:
    Zero((void*)&RETVAL, sizeof(RETVAL), char);
    OUTPUT:
    RETVAL

MODULE = XML::TinyXML        PACKAGE = TXmlPtr        

XmlNode *
cNode(THIS, __value = NO_INIT)
    TXml *THIS
    XmlNode *__value
    PROTOTYPE: $;$
    CODE:
    RETVAL = THIS->cNode;
    if (items > 1)
        THIS->cNode = __value;
    OUTPUT:
    RETVAL

char *
head(THIS, __value = NO_INIT)
    TXml *THIS
    char *__value
    PROTOTYPE: $;$
    CODE:
    if (items > 1)
        THIS->head = __value;
    RETVAL = THIS->head;
    OUTPUT:
    RETVAL

