#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include <linklist.h>
#include <txml.h>

#include "const-c.inc"

MODULE = XML::TinyXML        PACKAGE = XML::TinyXML        

INCLUDE: const-xs.inc

void
ClearList(list)
    LinkedList *list

ListEntry *
CreateEntry()

LinkedList *
CreateList()

TaggedValue *
CreateTaggedValue(tag, val, vLen)
    char *tag
    void *val
    unsigned long    vLen

void
DestroyEntry(entry)
    ListEntry *entry

void
DestroyList(list)
    LinkedList *list

void
DestroyTaggedValue(tVal)
    TaggedValue *tVal

ListEntry *
FetchEntry(list, pos)
    LinkedList *list
    unsigned long    pos

TaggedValue *
FetchTaggedValue(list, pos)
    LinkedList *list
    unsigned long    pos

void *
FetchValue(list, pos)
    LinkedList *list
    unsigned long    pos

unsigned long
GetEntryPosition(entry)
    ListEntry *entry

void *
GetEntryValue(entry)
    ListEntry *entry

TaggedValue *
GetTaggedValue(list, tag)
    LinkedList *list
    char *tag

unsigned long
GetTaggedValues(list, tag, values)
    LinkedList *list
    char *tag
    LinkedList *values

void
InitList(list)
    LinkedList *list

int
InsertEntry(list, entry, pos)
    LinkedList *list
    ListEntry *entry
    unsigned long    pos

int
InsertTaggedValue(list, tVal, pos)
    LinkedList *list
    TaggedValue *tVal
    unsigned long    pos

int
InsertValue(list, val, pos)
    LinkedList *list
    void *val
    unsigned long    pos

unsigned long
ListLength(list)
    LinkedList *list

int
MoveEntry(list, srcPos, dstPos)
    LinkedList *list
    unsigned long    srcPos
    unsigned long    dstPos

int
MoveValue(list, srcPos, dstPos)
    LinkedList *list
    unsigned long    srcPos
    unsigned long    dstPos

ListEntry *
PickEntry(list, pos)
    LinkedList *list
    unsigned long    pos

TaggedValue *
PickTaggedValue(list, pos)
    LinkedList *list
    unsigned long    pos

void *
PickValue(list, pos)
    LinkedList *list
    unsigned long    pos

ListEntry *
PopEntry(list)
    LinkedList *list

TaggedValue *
PopTaggedValue(list)
    LinkedList *list

void *
PopValue(list)
    LinkedList *list

int
PushEntry(list, entry)
    LinkedList *list
    ListEntry *entry

int
PushTaggedValue(list, tVal)
    LinkedList *list
    TaggedValue *tVal

int
PushValue(list, val)
    LinkedList *list
    void *val

ListEntry *
RemoveEntry(list, pos)
    LinkedList *list
    unsigned long    pos

void
SetEntryValue(entry, val)
    ListEntry *entry
    void *val

ListEntry *
ShiftEntry(list)
    LinkedList *list

TaggedValue *
ShiftTaggedValue(list)
    LinkedList *list

void *
ShiftValue(list)
    LinkedList *list

ListEntry *
SubstEntry(list, pos, entry)
    LinkedList *list
    unsigned long    pos
    ListEntry *entry

void *
SubstValue(list, pos, newVal)
    LinkedList *list
    unsigned long    pos
    void *newVal

int
SwapEntries(list, pos1, pos2)
    LinkedList *list
    unsigned long    pos1
    unsigned long    pos2

int
SwapValues(list, pos1, pos2)
    LinkedList *list
    unsigned long    pos1
    unsigned long    pos2

int
UnshiftEntry(list, entry)
    LinkedList *list
    ListEntry *entry

int
UnshiftTaggedValue(list, tVal)
    LinkedList *list
    TaggedValue *tVal

int
UnshiftValue(list, val)
    LinkedList *list
    void * val

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

XmlNode *
XmlCreateNode(name, val, parent = NULL)
    char *name
    char *val
    XmlNode *parent

void
XmlDestroyContext(xml)
    TXml *xml

void
XmlDestroyNode(node)
    XmlNode *node

char *
XmlDump(xml)
    TXml *xml

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


MODULE = XML::TinyXML        PACKAGE = LinkedList        

LinkedList *
_to_ptr(THIS)
    LinkedList THIS = NO_INIT
    PROTOTYPE: $
    CODE:
    if (sv_derived_from(ST(0), "LinkedList")) {
        STRLEN len;
        char *s = SvPV((SV*)SvRV(ST(0)), len);
        if (len != sizeof(THIS))
        croak("Size %d of packed data != expected %d",
            len, sizeof(THIS));
        RETVAL = (LinkedList *)s;
    }
    else
        croak("THIS is not of type LinkedList");
    OUTPUT:
    RETVAL

LinkedList
new(CLASS)
    char *CLASS = NO_INIT
    PROTOTYPE: $
    CODE:
    Zero((void*)&RETVAL, sizeof(RETVAL), char);
    OUTPUT:
    RETVAL

MODULE = XML::TinyXML        PACKAGE = LinkedListPtr        

ListEntry *
head(THIS, __value = NO_INIT)
    LinkedList *THIS
    ListEntry *__value
    PROTOTYPE: $;$
    CODE:
    if (items > 1)
        THIS->head = __value;
    RETVAL = THIS->head;
    OUTPUT:
    RETVAL

ListEntry *
tail(THIS, __value = NO_INIT)
    LinkedList *THIS
    ListEntry *__value
    PROTOTYPE: $;$
    CODE:
    if (items > 1)
        THIS->tail = __value;
    RETVAL = THIS->tail;
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

LinkedList *
children(THIS, __value = NO_INIT)
    XmlNode *THIS
    LinkedList *__value
    PROTOTYPE: $;$
    CODE:
    if (items > 1)
        THIS->children = __value;
    RETVAL = THIS->children;
    OUTPUT:
    RETVAL

LinkedList *
attributes(THIS, __value = NO_INIT)
    XmlNode *THIS
    LinkedList *__value
    PROTOTYPE: $;$
    CODE:
    if (items > 1)
        THIS->attributes = __value;
    RETVAL = THIS->attributes;
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


MODULE = XML::TinyXML        PACKAGE = TaggedValue        

TaggedValue *
_to_ptr(THIS)
    TaggedValue THIS = NO_INIT
    PROTOTYPE: $
    CODE:
    if (sv_derived_from(ST(0), "TaggedValue")) {
        STRLEN len;
        char *s = SvPV((SV*)SvRV(ST(0)), len);
        if (len != sizeof(THIS))
        croak("Size %d of packed data != expected %d",
            len, sizeof(THIS));
        RETVAL = (TaggedValue *)s;
    }
    else
        croak("THIS is not of type TaggedValue");
    OUTPUT:
    RETVAL

TaggedValue
new(CLASS)
    char *CLASS = NO_INIT
    PROTOTYPE: $
    CODE:
    Zero((void*)&RETVAL, sizeof(RETVAL), char);
    OUTPUT:
    RETVAL

MODULE = XML::TinyXML        PACKAGE = TaggedValuePtr        

char *
tag(THIS, __value = NO_INIT)
    TaggedValue *THIS
    char *__value
    PROTOTYPE: $;$
    CODE:
    if (items > 1)
        THIS->tag = __value;
    RETVAL = THIS->tag;
    OUTPUT:
    RETVAL

void *
value(THIS, __value = NO_INIT)
    TaggedValue *THIS
    void *__value
    PROTOTYPE: $;$
    CODE:
    if (items > 1)
        THIS->value = __value;
    RETVAL = THIS->value;
    OUTPUT:
    RETVAL

unsigned long
vLen(THIS, __value = NO_INIT)
    TaggedValue *THIS
    unsigned long __value
    PROTOTYPE: $;$
    CODE:
    if (items > 1)
        THIS->vLen = __value;
    RETVAL = THIS->vLen;
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
    if (items > 1)
        THIS->cNode = __value;
    RETVAL = THIS->cNode;
    OUTPUT:
    RETVAL

LinkedList *
rootElements(THIS, __value = NO_INIT)
    TXml *THIS
    LinkedList *__value
    PROTOTYPE: $;$
    CODE:
    if (items > 1)
        THIS->rootElements = __value;
    RETVAL = THIS->rootElements;
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


MODULE = XML::TinyXML        PACKAGE = ListEntry        

ListEntry *
_to_ptr(THIS)
    ListEntry THIS = NO_INIT
    PROTOTYPE: $
    CODE:
    if (sv_derived_from(ST(0), "ListEntry")) {
        STRLEN len;
        char *s = SvPV((SV*)SvRV(ST(0)), len);
        if (len != sizeof(THIS))
        croak("Size %d of packed data != expected %d",
            len, sizeof(THIS));
        RETVAL = (ListEntry *)s;
    }
    else
        croak("THIS is not of type ListEntry");
    OUTPUT:
    RETVAL

ListEntry
new(CLASS)
    char *CLASS = NO_INIT
    PROTOTYPE: $
    CODE:
    Zero((void*)&RETVAL, sizeof(RETVAL), char);
    OUTPUT:
    RETVAL

MODULE = XML::TinyXML        PACKAGE = ListEntryPtr        

struct __LinkedList *
list(THIS, __value = NO_INIT)
    ListEntry *THIS
    struct __LinkedList *__value
    PROTOTYPE: $;$
    CODE:
    if (items > 1)
        THIS->list = __value;
    RETVAL = THIS->list;
    OUTPUT:
    RETVAL

struct __ListEntry *
prev(THIS, __value = NO_INIT)
    ListEntry *THIS
    struct __ListEntry *__value
    PROTOTYPE: $;$
    CODE:
    if (items > 1)
        THIS->prev = __value;
    RETVAL = THIS->prev;
    OUTPUT:
    RETVAL

struct __ListEntry *
next(THIS, __value = NO_INIT)
    ListEntry *THIS
    struct __ListEntry *__value
    PROTOTYPE: $;$
    CODE:
    if (items > 1)
        THIS->next = __value;
    RETVAL = THIS->next;
    OUTPUT:
    RETVAL

void *
value(THIS, __value = NO_INIT)
    ListEntry *THIS
    void *__value
    PROTOTYPE: $;$
    CODE:
    if (items > 1)
        THIS->value = __value;
    RETVAL = THIS->value;
    OUTPUT:
    RETVAL

int
tagged(THIS, __value = NO_INIT)
    ListEntry *THIS
    int __value
    PROTOTYPE: $;$
    CODE:
    if (items > 1)
        THIS->tagged = __value;
    RETVAL = THIS->tagged;
    OUTPUT:
    RETVAL

