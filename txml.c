/*
 *  tinyxml.c
 *
 *  Created by xant on 2/17/06.
 *
 */

#include "txml.h"
#include "string.h"
#include "stdlib.h"
#include "iconv.h"
#include "errno.h"

#define XML_ELEMENT_NONE 0
#define XML_ELEMENT_START 1
#define XML_ELEMENT_VALUE 2
#define XML_ELEMENT_END 3
#define XML_ELEMENT_UNIQUE 4

int TXML_ALLOW_MULTIPLE_ROOTNODES = 0; // XXX - find a better way
int errno;

static char *
dexmlize(char *string)
{
    int i, p = 0;
    int len = strlen(string);
    char *unescaped = NULL;

    if (string) {
        unescaped = calloc(1, len+1); // inlude null-byte
        for (i = 0; i < len; i++) {
            switch (string[i]) {
                case '&':
                    if (string[i+1] == '#') {
                        char *marker;
                        i+=2;
                        marker = &string[i];
                        if (string[i] >= '0' && string[i] <= '9' &&
                            string[i+1] >= '0' && string[i+1] <= '9')
                        {
                            char chr = 0;
                            i+=2;
                            if (string[i] >= '0' && string[i] <= '9' && string[i+1] == ';')
                                i++;
                            else if (string[i] == ';')
                                ;
                            else
                                return NULL;
                            chr = (char)strtol(marker, NULL, 0);
                            unescaped[p] = chr;
                        }
                    } else if (strncmp(&string[i], "&amp;", 5) == 0) {
                        i+=4;
                        unescaped[p] = '&';
                    } else if (strncmp(&string[i], "&lt;", 4) == 0) {
                        i+=3;
                        unescaped[p] = '<';
                    } else if (strncmp(&string[i], "&gt;", 4) == 0) {
                        i+=3;
                        unescaped[p] = '>';
                    } else if (strncmp(&string[i], "&quot;", 6) == 0) {
                        i+=5;
                        unescaped[p] = '"';
                    } else if (strncmp(&string[i], "&apos;", 6) == 0) {
                        i+=5;
                        unescaped[p] = '\'';
                    } else {
                        return NULL;
                    }
                    p++;
                    break;
                default:
                    unescaped[p] = string[i];
                    p++;
            }
        }
    }
    return unescaped;
}

static char *
xmlize(char *string)
{
    int i, p = 0;
    int len;
    int bufsize;
    char *escaped = NULL;

    len = strlen(string);
    if (string) {
        bufsize = len+1;
        escaped = calloc(1, bufsize); // inlude null-byte
        for (i = 0; i < len; i++) {
            switch (string[i]) {
                case '&':
                    bufsize += 5;
                    escaped = realloc(escaped, bufsize);
                    memset(escaped+p, 0, bufsize-p);
                    strcpy(&escaped[p], "&amp;");
                    p += 5;
                    break;
                case '<':
                    bufsize += 4;
                    escaped = realloc(escaped, bufsize);
                    memset(escaped+p, 0, bufsize-p);
                    strcpy(&escaped[p], "&lt;");
                    p += 4;
                    break;
                case '>':
                    bufsize += 4;
                    escaped = realloc(escaped, bufsize);
                    memset(escaped+p, 0, bufsize-p);
                    strcpy(&escaped[p], "&gt;");
                    p += 4;
                    break;
                case '"':
                    bufsize += 6;
                    escaped = realloc(escaped, bufsize);
                    memset(escaped+p, 0, bufsize-p);
                    strcpy(&escaped[p], "&quot;");
                    p += 6;
                    break;
                case '\'':
                    bufsize += 6;
                    escaped = realloc(escaped, bufsize);
                    memset(escaped+p, 0, bufsize-p);
                    strcpy(&escaped[p], "&apos;");
                    p += 6;
                    break;
/*
                    bufsize += 5;
                    escaped = realloc(escaped, bufsize);
                    memset(escaped+p, 0, bufsize-p);
                    sprintf(&escaped[p], "&#%02d;", string[i]);
                    p += 5;
                    break;
*/
                default:
                    escaped[p] = string[i];
                    p++;
            }
        }
    }
    return escaped;
}

TXml *
XmlCreateContext()
{
    TXml *xml;

    xml = (TXml *)calloc(1, sizeof(TXml));
    xml->cNode = NULL;
    TAILQ_INIT(&xml->rootElements);
    xml->head = NULL;
    // default to UTF-8
    sprintf(xml->outputEncoding, "utf-8");
    sprintf(xml->documentEncoding, "utf-8");
    return xml;
}

void
XmlSetDocumentEncoding(TXml *xml, char *encoding)
{
    strncpy(xml->documentEncoding, encoding, sizeof(xml->documentEncoding)-1);
}

void
XmlSetOutputEncoding(TXml *xml, char *encoding)
{
    strncpy(xml->outputEncoding, encoding, sizeof(xml->outputEncoding)-1);
}

void
XmlDestroyContext(TXml *xml)
{
    XmlNode *rNode, *tmp;
    TAILQ_FOREACH_SAFE(rNode, &xml->rootElements, siblings, tmp) {
        TAILQ_REMOVE(&xml->rootElements, rNode, siblings);
        XmlDestroyNode(rNode);
    }
    if(xml->head)
        free(xml->head);
    free(xml);
}

static void
XmlSetNodePath(XmlNode *node, XmlNode *parent)
{
    unsigned int pathLen;

    if (node->path)
        free(node->path);

    if(parent) {
        if(parent->path) {
            pathLen = (unsigned int)strlen(parent->path)+1+strlen(node->name)+1;
            node->path = (char *)calloc(1, pathLen);
            sprintf(node->path, "%s/%s", parent->path, node->name);
        } else {
            pathLen = (unsigned int)strlen(parent->name)+1+strlen(node->name)+1;
            node->path = (char *)calloc(1, pathLen);
            sprintf(node->path, "%s/%s", parent->name, node->name);
        }
    } else { /* root node */
        node->path = (char *)calloc(1, strlen(node->name)+2);
        sprintf(node->path, "/%s", node->name);
    }

}

XmlNode *
XmlCreateNode(char *name, char *value, XmlNode *parent)
{
    XmlNode *node = NULL;
    node = (XmlNode *)calloc(1, sizeof(XmlNode));
    if(!node || !name)
        return NULL;

    TAILQ_INIT(&node->attributes);
    TAILQ_INIT(&node->children);
    node->parent = parent;
    node->name = strdup(name);

    if (parent)
        XmlAddChildNode(parent, node);
    else
        XmlSetNodePath(node, NULL);

    if(value && strlen(value) > 0)
        node->value = strdup(value);
    else
        node->value = calloc(1, 1);
    return node;
}

void
XmlDestroyNode(XmlNode *node)
{
    XmlNodeAttribute *attr, *attr_tmp;
    XmlNode *child, *child_tmp;

    TAILQ_FOREACH_SAFE(attr, &node->attributes, list, attr_tmp) {
        TAILQ_REMOVE(&node->attributes, attr, list);
        if(attr->name)
            free(attr->name);
        if(attr->value)
            free(attr->value);
        free(attr);
    }

    TAILQ_FOREACH_SAFE(child, &node->children, siblings, child_tmp) {
        TAILQ_REMOVE(&node->children, child, siblings);
        XmlDestroyNode(child);
    }
    if(node->name)
        free(node->name);
    if(node->path)
        free(node->path);
    if(node->value)
        free(node->value);
    free(node);
}

XmlErr
XmlSetNodeValue(XmlNode *node, char *val)
{
    if(!val)
        return XML_BADARGS;

    if(node->value)
        free(node->value);
    node->value = strdup(val);
    return XML_NOERR;
}

/* quite useless */
char *
XmlGetNodeValue(XmlNode *node)
{
    if(!node)
        return NULL;
    return node->value;
}

static void
XmlRemoveChildNode(XmlNode *parent, XmlNode *child)
{
    int i;
    XmlNode *p, *tmp;
    TAILQ_FOREACH_SAFE(p, &parent->children, siblings, tmp) {
        if (p == child) {
            TAILQ_REMOVE(&parent->children, p, siblings);
            p->parent = NULL;
            XmlSetNodePath(p, NULL);
            break;
        }
    }
}

XmlErr
XmlAddChildNode(XmlNode *parent, XmlNode *child)
{
    if(!child)
        return XML_BADARGS;

    if (child->parent)
        XmlRemoveChildNode(child->parent, child);

    TAILQ_INSERT_TAIL(&parent->children, child, siblings);
    child->parent = parent;
    XmlSetNodePath(child, parent);
    return XML_NOERR;
}

XmlNode *
XmlNextSibling(XmlNode *node)
{
    return TAILQ_NEXT(node, siblings);
}

XmlNode *
XmlPrevSibling(XmlNode *node)
{
    return TAILQ_PREV(node, nodelistHead, siblings);
}

XmlErr
XmlAddRootNode(TXml *xml, XmlNode *node)
{
    if(!node)
        return XML_BADARGS;

    if (!TAILQ_EMPTY(&xml->rootElements) && !TXML_ALLOW_MULTIPLE_ROOTNODES) {
        return XML_MROOT_ERR;
    }

    TAILQ_INSERT_TAIL(&xml->rootElements, node, siblings);
    return XML_NOERR;
}

XmlErr
XmlAddAttribute(XmlNode *node, char *name, char *val)
{
    XmlNodeAttribute *attr;

    if(!name || !node)
        return XML_BADARGS;

    attr = calloc(1, sizeof(XmlNodeAttribute));
    attr->name = strdup(name);
    attr->value = val?strdup(val):strdup("");
    attr->node = node;

    TAILQ_INSERT_TAIL(&node->attributes, attr, list);
    return XML_NOERR;
    /*
    free(attr->name);
    free(attr->value);
    free(attr);
    return XML_GENERIC_ERR;
    */
}

int
XmlRemoveAttribute(XmlNode *node, unsigned long index)
{
    XmlNodeAttribute *attr, *tmp;
    int count = 1;

    TAILQ_FOREACH_SAFE(attr, &node->attributes, list, tmp) {
        if (count == index) {
            TAILQ_REMOVE(&node->attributes, attr, list);
            free(attr->name);
            free(attr->value);
            free(attr);
            return XML_NOERR;
        }
        count++;
    }
    return XML_GENERIC_ERR;
}

void
XmlClearAttributes(XmlNode *node)
{
    XmlNodeAttribute *attr, *tmp;
    unsigned int nAttrs = 0;
    int i;

    TAILQ_FOREACH_SAFE(attr, &node->attributes, list, tmp) {
        TAILQ_REMOVE(&node->attributes, attr, list);
        free(attr->name);
        free(attr->value);
        free(attr);
    }
}

XmlNodeAttribute
*XmlGetAttributeByName(XmlNode *node, char *name)
{
    int i;
    XmlNodeAttribute *attr;
    TAILQ_FOREACH(attr, &node->attributes, list) {
        if (strcmp(attr->name, name) == 0)
            return attr;
    }
    return NULL;
}

XmlNodeAttribute
*XmlGetAttribute(XmlNode *node, unsigned long index)
{
    XmlNodeAttribute *attr;
    int count = 1;
    TAILQ_FOREACH(attr, &node->attributes, list) {
        if (count == index)
            return attr;
        count++;
    }
    return NULL;
}

static XmlErr
XmlExtraNodeHandler(TXml *xml, char *content, char type)
{
    XmlNode *newNode = NULL;
    XmlErr res = XML_NOERR;
    char fakeName[256];

    sprintf(fakeName, "_fakenode_%d_", type);
    newNode = XmlCreateNode(fakeName, content, xml->cNode);
    newNode->type = type;
    if(!newNode || !newNode->name) {
        /* XXX - ERROR MESSAGES HERE */
        res = XML_GENERIC_ERR;
        goto _node_done;
    }
    if(xml->cNode) {
        res = XmlAddChildNode(xml->cNode, newNode);
        if(res != XML_NOERR) {
            XmlDestroyNode(newNode);
            goto _node_done;
        }
    } else {
        res = XmlAddRootNode(xml, newNode) ;
        if(res != XML_NOERR) {
            XmlDestroyNode(newNode);
            goto _node_done;
        }
    }
_node_done:
    return res;
}

static XmlErr
XmlStartHandler(TXml *xml, char *element, char **attr_names, char **attr_values)
{
    XmlNode *newNode = NULL;
    unsigned int offset = 0;
    XmlErr res = XML_NOERR;
    char *nodename;

    if(!element || strlen(element) == 0)
        return XML_BADARGS;

    // unescape read element to be used as nodename
    nodename = dexmlize(element);
    if (!nodename)
        return XML_BAD_CHARS;

    newNode = XmlCreateNode(nodename, NULL, xml->cNode);
    free(nodename);
    if(!newNode || !newNode->name)
    {
        /* XXX - ERROR MESSAGES HERE */
        return XML_MEMORY_ERR;
    }
    /* handle attributes if present */
    if(attr_names && attr_values)
    {
        while(attr_names[offset] != NULL)
        {
            res = XmlAddAttribute(newNode, attr_names[offset], attr_values[offset]);
            if(res != XML_NOERR)
            {
                XmlDestroyNode(newNode);
                goto _start_done;
            }
            offset++;
        }
    }
    if(xml->cNode)
    {
        res = XmlAddChildNode(xml->cNode, newNode);
        if(res != XML_NOERR)
        {
            XmlDestroyNode(newNode);
            goto _start_done;
        }
    }
    else
    {
        res = XmlAddRootNode(xml, newNode) ;
        if(res != XML_NOERR)
        {
            XmlDestroyNode(newNode);
            goto _start_done;
        }
    }
    xml->cNode = newNode;

_start_done:
    return res;
}

static XmlErr
XmlEndHandler(TXml *xml, char *element)
{
    XmlNode *parent;
    if(xml->cNode)
    {
        parent = xml->cNode->parent;
        xml->cNode = parent;
        return XML_NOERR;
    }
    return XML_GENERIC_ERR;
}

static XmlErr
XmlValueHandler(TXml *xml, char *text)
{
    char *p;
    if(text) {
        // TODO - make 'skipblanks' optional
        // remove heading blanks
        while((*text == ' ' || *text == '\t' ||
            *text == '\r' || *text == '\n') && *text != 0)
        {
            text++;
        }

        p = text+strlen(text)-1;

        // remove trailing blanks
        while((*p == ' ' || *p == '\t' ||
            *p == '\r' || *p == '\n') && p != text)
        {
            *p = 0;
            p--;
        }

        if(xml->cNode)  {
            char *rtext = dexmlize(text);
            if (!rtext)
                return XML_BAD_CHARS;
            XmlSetNodeValue(xml->cNode, rtext);
            free(rtext);
        } else {
            fprintf(stderr, "cTag == NULL while handling a value!!");
        }
        return XML_NOERR;
    }
    return XML_GENERIC_ERR;
}


XmlErr
XmlParseBuffer(TXml *xml, char *buf)
{
    XmlErr err = XML_NOERR;
    int state = XML_ELEMENT_NONE;
    char *p = buf;
    unsigned int i;
    char *start = NULL;
    char *end = NULL;
    char **attrs = NULL;
    char **values = NULL;
    unsigned int nAttrs = 0;
    char *mark = NULL;
    int quote = 0;

    //unsigned int offset = fileStat.st_size;

#define XML_FREE_ATTRIBUTES \
    if(nAttrs>0) {\
        for(i = 0; i < nAttrs; i++) {\
            if(attrs[i]) free(attrs[i]);\
            if(values[i]) free(values[i]);\
        }\
        free(attrs);\
        attrs = NULL;\
        free(values);\
        values = NULL;\
        nAttrs = 0;\
    }\

#define SKIP_BLANKS(__p) \
    while((*__p == ' ' || *__p == '\t' || *__p == '\r' || *__p == '\n') && *__p != 0) __p++; \
    if(*__p == 0) break;

#define ADVANCE_ELEMENT(__p) \
    while(*__p != '>' && *__p != ' ' && *__p != '\t' && *__p != '\r' && *__p != '\n' && *__p != 0) __p++; \
    if(*__p == 0) break;

#define ADVANCE_TO_ATTR_VALUE(__p) \
    while(*__p != '=' && *__p != ' ' && *__p != '\t' && *__p != '\r' && *__p != '\n' && *__p != 0) __p++;\
    SKIP_BLANKS(__p);

    while(*p != 0) {
        SKIP_BLANKS(p);
        if(*p == '<') {
            p++;
            if(*p == '/') {
                p++;
                SKIP_BLANKS(p);
                mark = p;
                while(*p != '>' && *p != 0)
                    p++;
                if(*p == '>') {
                    end = (char *)malloc(p-mark+1);
                    if(!end)
                    {
                        err = XML_MEMORY_ERR;
                        goto _parser_err;
                    }
                    strncpy(end, mark, p-mark);
                    end[p-mark] = 0;
                    p++;
                    state = XML_ELEMENT_END;
                    err = XmlEndHandler(xml, end);
                    free(end);
                    if(err != XML_NOERR)
                        goto _parser_err;
                }
            } else if(strncmp(p, "!ENTITY", 8) == 0) { // XXX - IGNORING !ENTITY NODES
                p += 8;
                mark = p;
                p = strstr(mark, ">");
                if(!p) {
                    fprintf(stderr, "Can't find where the !ENTITY element ends\n");
                    err = XML_PARSER_GENERIC_ERR;
                    goto _parser_err;
                }
                p++;
            } else if(strncmp(p, "!NOTATION", 9) == 0) { // XXX - IGNORING !NOTATION NODES
                p += 9;
                mark = p;
                p = strstr(mark, ">");
                if(!p) {
                    fprintf(stderr, "Can't find where the !NOTATION element ends\n");
                    err = XML_PARSER_GENERIC_ERR;
                    goto _parser_err;
                }
                p++;
            } else if(strncmp(p, "!ATTLIST", 8) == 0) { // XXX - IGNORING !ATTLIST NODES
                p += 8;
                mark = p;
                p = strstr(mark, ">");
                if(!p) {
                    fprintf(stderr, "Can't find where the !NOTATION element ends\n");
                    err = XML_PARSER_GENERIC_ERR;
                    goto _parser_err;
                }
                p++;
            } else if(strncmp(p, "!--", 3) == 0) { /* comment */
                char *comment = NULL;
                p += 3; /* skip !-- */
                mark = p;
                p = strstr(mark, "-->");
                if(!p) {
                    /* XXX - TODO - This error condition must be handled asap */
                }
                comment = calloc(1, p-mark+1);
                if(!comment) {
                    err = XML_MEMORY_ERR;
                    goto _parser_err;
                }
                strncpy(comment, mark, p-mark);
                err = XmlExtraNodeHandler(xml, comment, XML_NODETYPE_COMMENT);
                free(comment);
                p+=3;
            } else if(strncmp(p, "![", 2) == 0) {
                mark = p;
                p += 2; /* skip ![ */
                SKIP_BLANKS(p);
                //mark = p;
                if(strncmp(p, "CDATA", 5) == 0) {
                    char *cdata = NULL;
                    p+=5;
                    SKIP_BLANKS(p);
                    if(*p != '[') {
                        fprintf(stderr, "Unsupported entity type at \"... -->%15s\"", mark);
                        err = XML_PARSER_GENERIC_ERR;
                        goto _parser_err;
                    }
                    mark = ++p;
                    p = strstr(mark, "]]>");
                    if(!p) {
                        /* XXX - TODO - This error condition must be handled asap */
                    }
                    cdata = calloc(1, p-mark+1);
                    if(!cdata) {
                        err = XML_MEMORY_ERR;
                        goto _parser_err;
                    }
                    strncpy(cdata, mark, p-mark);
                    err = XmlExtraNodeHandler(xml, cdata, XML_NODETYPE_CDATA);
                    free(cdata);
                    p+=3;
                } else {
                    fprintf(stderr, "Unsupported entity type at \"... -->%15s\"", mark);
                    err = XML_PARSER_GENERIC_ERR;
                    goto _parser_err;
                }
            } else if(*p =='?') { /* head */
                char *encoding = NULL;
                p++;
                mark = p;
                p = strstr(mark, "?>");
                if(xml->head)
                    free(xml->head); /* XXX - should notify this behaviour? */
                xml->head = calloc(1, p-mark+1);
                strncpy(xml->head, mark, p-mark);
                encoding = strstr(xml->head, "encoding=");
                if (encoding) {
                    encoding += 9;
                    if (*encoding == '"' || *encoding == '\'') {
                        int encoding_length = 0;
                        quote = *encoding;
                        encoding++;
                        end = (char *)strchr(encoding, quote);
                        if (!end) {
                            fprintf(stderr, "Unquoted encoding string in the <?xml> section");
                            err = XML_PARSER_GENERIC_ERR;
                            goto _parser_err;
                        }
                        encoding_length = end - encoding;
                        if (encoding_length < sizeof(xml->documentEncoding)) {
                            strncpy(xml->documentEncoding, encoding, encoding_length);
                            // ensure to terminate it, if we are reusing a context we 
                            // could have still the old encoding there possibly with a 
                            // longer name (so poisoning the buffer)
                            xml->documentEncoding[encoding_length] = 0; 
                        }
                    }
                } else {
                }
                p+=2;
            } else { /* start tag */
                attrs = NULL;
                values = NULL;
                nAttrs = 0;
                state = XML_ELEMENT_START;
                SKIP_BLANKS(p);
                mark = p;
                ADVANCE_ELEMENT(p);
                start = (char *)malloc(p-mark+2);
                if(start == NULL)
                    return XML_MEMORY_ERR;
                strncpy(start, mark, p-mark);
                if(*p == '>' && *(p-1) == '/') {
                    start[p-mark-1] = 0;
                    state = XML_ELEMENT_UNIQUE;
                }
                else {
                    start[p-mark] = 0;
                }
                SKIP_BLANKS(p);
                while(*p != '>' && *p != 0) {
                    mark = p;
                    ADVANCE_TO_ATTR_VALUE(p);
                    if(*p == '=') {
                        char *tmpAttr = (char *)malloc(p-mark+1);
                        strncpy(tmpAttr, mark, p-mark);
                        tmpAttr[p-mark] = 0;
                        p++;
                        SKIP_BLANKS(p);
                        if(*p == '"' || *p == '\'') {
                            quote = *p;
                            p++;
                            mark = p;
                            while(*p != 0) {
                                if (*p == quote) {
                                    if (*(p+1) != quote) // handle quote escaping
                                        break;
                                    else
                                        p++;
                                }
                                p++;
                            }
                            if(*p == quote) {
                                char *dexmlized;
                                char *tmpVal = (char *)malloc(p-mark+2);
                                strncpy(tmpVal, mark, p-mark);
                                tmpVal[p-mark] = 0;
                                /* add new attribute */
                                nAttrs++;
                                attrs = (char **)realloc(attrs, sizeof(char *)*(nAttrs+1));
                                attrs[nAttrs-1] = tmpAttr;
                                attrs[nAttrs] = NULL;
                                values = (char **)realloc(values, sizeof(char *)*(nAttrs+1));
                                dexmlized = dexmlize(tmpVal);
                                free(tmpVal);
                                values[nAttrs-1] = dexmlized;
                                values[nAttrs] = NULL;
                                p++;
                                SKIP_BLANKS(p);
                            }
                            else {
                                free(tmpAttr);
                            }
                        } /* if(*p == '"' || *p == '\'') */
                        else {
                            free(tmpAttr);
                        }
                    } /* if(*p=='=') */
                    if(*p == '/' && *(p+1) == '>') {
                        p++;
                        state = XML_ELEMENT_UNIQUE;
                    }
                } /* while(*p != '>' && *p != 0) */
                err = XmlStartHandler(xml, start, attrs, values);
                if(err != XML_NOERR) {
                    XML_FREE_ATTRIBUTES
                    free(start);
                    return err;
                }
                if(state == XML_ELEMENT_UNIQUE) {
                    err = XmlEndHandler(xml, start);
                    if(err != XML_NOERR) {
                        XML_FREE_ATTRIBUTES
                        free(start);
                        return err;
                    }
                }
                XML_FREE_ATTRIBUTES
                free(start);
                p++;
            } /* end of start tag */
        } /* if(*p == '<') */
        else if(state == XML_ELEMENT_START) {
            state = XML_ELEMENT_VALUE;
            mark = p;
            while(*p != '<' && *p != 0)
                p++;
            if(*p == '<') {
                char *value = (char *)malloc(p-mark+1);
                strncpy(value, mark, p-mark);
                value[p-mark] = 0;
                err = XmlValueHandler(xml, value);
                if(value)
                    free(value);
                if(err != XML_NOERR)
                    return(err);
                //p++;
            }
        }
        else {
            /* XXX */
            p++;
        }
    } // while(*p != 0)

_parser_err:
    return err;
}


XmlErr
XmlParseFile(TXml *xml, char *path)
{
    FILE *inFile;
    char *buffer;
    XmlErr err;
    struct stat fileStat;
    int rc = 0;

    inFile = NULL;
    err = XML_NOERR;
    if(!path)
        return XML_BADARGS;
    rc = stat(path, &fileStat);
    if (rc != 0)
        return XML_BADARGS;
    xml->cNode = NULL;
    if(fileStat.st_size>0) {
        inFile = fopen(path, "r");
        if(inFile) {
            if(XmlFileLock(inFile) != XML_NOERR) {
                fprintf(stderr, "Can't lock %s for opening ", path);
                return -1;
            }
            buffer = (char *)malloc(fileStat.st_size+1);
            fread(buffer, 1, fileStat.st_size, inFile);
            buffer[fileStat.st_size] = 0;
            err = XmlParseBuffer(xml, buffer);
            free(buffer);
            XmlFileUnlock(inFile);
            fclose(inFile);
        }
        else {
            fprintf(stderr, "Can't open xmlfile %s\n", path);
            return -1;
        }
    }
    else
    {
        fprintf(stderr, "Can't stat xmlfile %s\n", path);
        return -1;
    }
    return XML_NOERR;
}

char *
XmlDumpBranch(TXml *xml, XmlNode *rNode, unsigned int depth)
{
    unsigned int i, n;
    char *out = NULL;
    char *startTag;
    char *endTag;
    char *childDump;
    char *value = NULL;
    int nameLen;
    XmlNodeAttribute *attr;
    XmlNode *child;
    unsigned long nAttrs;


    if (rNode->value) {
        if (rNode->type == XML_NODETYPE_SIMPLE) {
            value = xmlize(rNode->value);
        } else {
            value = strdup(rNode->value);
        }
    }

    if(rNode->name)
        nameLen = (unsigned int)strlen(rNode->name);
    else
        return NULL;

    /* First check if this is a special node (a comment or a CDATA) */
    if(rNode->type == XML_NODETYPE_COMMENT) {
        out = malloc(strlen(value)+depth+9);
        *out = 0;
        for(n = 0; n < depth; n++) strcat(out, "\t");
        strcat(out, "<!--");
        strcat(out, value);
        strcat(out, "-->\n");
        return out;
    } else if(rNode->type == XML_NODETYPE_CDATA) {
        out = malloc(strlen(value)+depth+14);
        *out = 0;
        for(n = 0; n < depth; n++)
            strcat(out, "\t");
        strcat(out, "<![CDATA[");
        strcat(out, value);
        strcat(out, "]]>\n");
        return out;
    }

    childDump = (char *)malloc(1);
    *childDump = 0;

    startTag = (char *)malloc(depth+nameLen+7);
    memset(startTag, 0, depth+nameLen+7);
    endTag = (char *)malloc(depth+nameLen+7);
    memset(endTag, 0, depth+nameLen+7);

    for(n = 0; n < depth; n++)
        strcat(startTag, "\t");
    strcat(startTag, "<");
    strcat(startTag, rNode->name);
    nAttrs = XmlCountAttributes(rNode);
    if(nAttrs>0) {
        for(i = 1; i <= nAttrs; i++) {
            attr = XmlGetAttribute(rNode, i);
            if(attr) {
                char *attr_value = xmlize(attr->value);
                startTag = (char *)realloc(startTag, strlen(startTag)+
                    strlen(attr->name)+strlen(attr_value)+8);
                strcat(startTag, " ");
                strcat(startTag, attr->name);
                strcat(startTag, "=\"");
                strcat(startTag, attr_value);
                strcat(startTag, "\"");
                if (attr_value)
                    free(attr_value);
            }
        }
    }
    if((value && *value) || !TAILQ_EMPTY(&rNode->children)) {
        if(!TAILQ_EMPTY(&rNode->children)) {
            strcat(startTag, ">\n");
            for(n = 0; n < depth; n++)
                strcat(endTag, "\t");
            TAILQ_FOREACH(child, &rNode->children, siblings) {
                char *childBuff = XmlDumpBranch(xml, child, depth+1); /* let's recurse */
                if(childBuff) {
                    childDump = (char *)realloc(childDump, strlen(childDump)+strlen(childBuff)+2);
                    strcat(childDump, childBuff);
                    //strcat(childDump, "\n");
                    free(childBuff);
                }
            }
        } else {
            // TODO - allow to specify a flag to determine if we want white spaces or not
            strcat(startTag, ">");
        }
        strcat(endTag, "</");
        strcat(endTag, rNode->name);
        strcat(endTag, ">\n");
        out = (char *)malloc(depth+strlen(startTag)+strlen(endTag)+
            (value?strlen(value)+1:1)+strlen(childDump)+3);
        strcpy(out, startTag);
        if(value && *value) { // skip also if value is an empty string (not only if it's a null pointer)
            if(!TAILQ_EMPTY(&rNode->children)) {
                for(n = 0; n < depth; n++)
                    strcat(out, "\t");
                strcat(out, value);
                strcat(out, "\n");
            }
            else {
                strcat(out, value);
                //strcat(out, " ");
            }
        }
        strcat(out, childDump);
        strcat(out, endTag);
    }
    else {
        strcat(startTag, "/>\n");
        out = strdup(startTag);
    }
    free(startTag);
    free(endTag);
    free(childDump);
    if (value)
        free(value);
    return out;
}

char *
XmlDump(TXml *xml, int *outlen)
{
    char *dump;
    XmlNode *rNode;
    char *branch;
    unsigned int i;
    int doConversion = 0;
    char head[256]; // should be enough

    memset(head, 0, sizeof(head));
    if (xml->head) {
        int quote;
        char *start, *end, *encoding;
        char *initial = strdup(xml->head);
        start = strstr(initial, "encoding=");
        if (start) {
            *start = 0;
            encoding = start+9;
            if (*encoding == '"' || *encoding == '\'') {
                quote = *encoding;
                encoding++;
                end = (char *)strchr(encoding, quote);
                if (!end) {
                    /* TODO - Error Messages */
                } else if ((end-encoding) >= sizeof(xml->outputEncoding)) {
                    /* TODO - Error Messages */
                }
                *end = 0;
                // check if document encoding matches
                if (strncasecmp(encoding, xml->documentEncoding, end-encoding) != 0) {
                    /* TODO - Error Messages */
                } 
                if (strncasecmp(encoding, xml->outputEncoding, end-encoding) != 0) {
                    snprintf(head, sizeof(head), "%sencoding=\"%s\"%s",
                        initial, xml->outputEncoding, ++end);
                    doConversion = 1;
                } else {
                    snprintf(head, sizeof(head), "%s", xml->head);
                }
            }
        } else {
            snprintf(head, sizeof(head), "xml version=\"1.0\" encoding=\"%s\"", 
                xml->outputEncoding?xml->outputEncoding:"utf-8");
        }
        free(initial);
    } else {
        snprintf(head, sizeof(head), "xml version=\"1.0\" encoding=\"%s\"", 
            xml->outputEncoding?xml->outputEncoding:"utf-8");
    }
    dump = malloc(strlen(head)+6);
    sprintf(dump, "<?%s?>\n", head);
    TAILQ_FOREACH(rNode, &xml->rootElements, siblings) {
        branch = XmlDumpBranch(xml, rNode, 0);
        if(branch) {
            dump = (char *)realloc(dump, strlen(dump)+strlen(branch)+1);
            strcat(dump, branch);
            free(branch);
        }
    }
    if (outlen) // check if we need to report the output size
        *outlen = strlen(dump);
    if (doConversion) {
        iconv_t ich;
        size_t ilen, olen, cb;
        char *out;
        char *iconvIn;
        char *iconvOut;
        ilen = strlen(dump);
        // the most expensive conversion would be from ascii to utf-32/ucs-4
        // ( 4 bytes for each char )
        olen = ilen *4; 
        // we still don't know how big the output buffer is going to be
        // we will update outlen later once iconv tell us the size
        if (outlen) 
            *outlen = olen;
        out = calloc(1, olen);
        ich = iconv_open (xml->outputEncoding, xml->documentEncoding);
        if (ich == (iconv_t)(-1)) {
            free(dump);
            free(out);
            fprintf(stderr, "Can't init iconv: %s\n", strerror(errno));
            return NULL;
        }
        iconvIn = dump;
        iconvOut = out;
        cb = iconv(ich, &iconvIn, &ilen, &iconvOut, &olen);
        if (cb == -1) {
            free(dump);
            free(out);
            fprintf(stderr, "Error from iconv: %s\n", strerror(errno));
            return NULL;
        }
        iconv_close(ich);
        free(dump); // release the old buffer (in the original encoding)
        dump = out;
        if (outlen) // update the outputsize if we have to
            *outlen -= olen;
    }
    return(dump);
}

XmlErr
XmlSave(TXml *xml, char *xmlFile)
{
    struct stat fileStat;
    FILE *saveFile = NULL;
    char *dump = NULL;
    char *backup = NULL;
    char *backupPath = NULL;
    FILE *backupFile = NULL;


    if (stat(xmlFile, &fileStat) == 0) {
        if(fileStat.st_size>0) { /* backup old profiles */
            saveFile = fopen(xmlFile, "r");
            if(!saveFile) {
                fprintf(stderr, "Can't open %s for reading !!", xmlFile);
                return XML_GENERIC_ERR;
            }
            if(XmlFileLock(saveFile) != XML_NOERR) {
                fprintf(stderr, "Can't lock %s for reading ", xmlFile);
                return XML_GENERIC_ERR;
            }
            backup = (char *)malloc(fileStat.st_size+1);
            fread(backup, 1, fileStat.st_size, saveFile);
            backup[fileStat.st_size] = 0;
            XmlFileUnlock(saveFile);
            fclose(saveFile);
            backupPath = (char *)malloc(strlen(xmlFile)+5);
            sprintf(backupPath, "%s.bck", xmlFile);
            backupFile = fopen(backupPath, "w+");
            if(backupFile) {
                if(XmlFileLock(backupFile) != XML_NOERR) {
                    fprintf(stderr, "Can't lock %s for writing ", backupPath);
                    free(backupPath);
                    free(backup);
                    return XML_GENERIC_ERR;
                }
                fwrite(backup, 1, fileStat.st_size, backupFile);
                XmlFileUnlock(backupFile);
                fclose(backupFile);
            }
            else {
                fprintf(stderr, "Can't open backup file (%s) for writing! ", backupPath);
                free(backupPath);
                free(backup);
                return XML_GENERIC_ERR;
            }
            free(backupPath);
            free(backup);
        } /* end of backup */
    }
    dump = XmlDump(xml, NULL);
     if(dump) {
        saveFile = fopen(xmlFile, "w+");
        if(saveFile) {
            if(XmlFileLock(saveFile) != XML_NOERR) {
                fprintf(stderr, "Can't lock %s for writing ", xmlFile);
                free(dump);
                return XML_GENERIC_ERR;
            }
            fwrite(dump, 1, strlen(dump), saveFile);
            free(dump);
            XmlFileUnlock(saveFile);
            fclose(saveFile);
        }
        else {
            fprintf(stderr, "Can't open output file %s", xmlFile);
            if(!saveFile) {
                free(dump);
                return XML_GENERIC_ERR;
            }
        }
    }
    return XML_NOERR;
}

unsigned long
XmlCountAttributes(XmlNode *node)
{
    XmlNodeAttribute *attr;
    int cnt = 0;
    TAILQ_FOREACH(attr, &node->attributes, list) 
        cnt++;
    return cnt;
}

unsigned long
XmlCountChildren(XmlNode *node)
{
    XmlNode *child;
    int cnt = 0; 
    TAILQ_FOREACH(child, &node->children, siblings)
        cnt++;
    return cnt;
}

unsigned long
XmlCountBranches(TXml *xml)
{
    XmlNode *node;
    int cnt = 0;
    TAILQ_FOREACH(node, &xml->rootElements, siblings)
        cnt++;
    return cnt;
}

XmlErr
XmlRemoveNode(TXml *xml, char *path)
{
    /* XXX - UNIMPLEMENTED */
    return XML_GENERIC_ERR;
}

XmlErr
XmlRemoveBranch(TXml *xml, unsigned long index)
{
    int count = 1;
    XmlNode *branch, *tmp;
    TAILQ_FOREACH_SAFE(branch, &xml->rootElements, siblings, tmp) {
        if (count == index) {
            TAILQ_REMOVE(&xml->rootElements, branch, siblings);
            XmlDestroyNode(branch);
            return XML_NOERR;
        }
        count++;
    }
    return XML_GENERIC_ERR;
}

XmlNode
*XmlGetChildNode(XmlNode *node, unsigned long index)
{
    XmlNode *child;
    int count = 1;
    if(!node)
        return NULL;
    TAILQ_FOREACH(child, &node->children, siblings) {
        if (count == index) {
            return child;
            break;
        }
        count++;
    }
    return NULL;
}

/* XXX - if multiple children shares the same name, only the first is returned */
XmlNode
*XmlGetChildNodeByName(XmlNode *node, char *name)
{
    XmlNode *child;
    unsigned int i = 0;
    char *attrName = NULL;
    char *attrVal = NULL;
    char *nodeName = NULL;
    int nameLen = 0;
    char *p;

    if(!node)
        return NULL;

    nodeName = strdup(name); // make a copy to avoid changing the provided buffer
    nameLen = strlen(nodeName);

    if (nodeName[nameLen-1] == ']') {
        p = strchr(nodeName, '[');
        *p = 0;
        p++;
        if (sscanf(p, "%d]", &i) == 1) {
            i--;
        } else if (*p == '@') {
            p++;
            p[strlen(p)-1] = 0;
            attrName = p;
            attrVal = strchr(p, '=');
            if (attrVal) {
                *attrVal = 0;
                attrVal++;
                if (*attrVal == '\'' || *attrVal == '"') {
                    char quote = *attrVal;
                    int n, j;
                    // s = ++attrVal could be unsafe
                    attrVal++;
                    for (n = 0; attrVal[n] != 0; n++) {
                        if (attrVal[n] == quote) {
                            if (attrVal[n-1] == quote) { // quote escaping (XXX - perhaps out of spec)
                                j--;
                            } else {
                                attrVal[n] = 0;
                                break;
                            }
                            if (j != n)
                                attrVal[j] = attrVal[n];
                            j++;
                        }
                    }

                }
            }
        }
    }

    TAILQ_FOREACH(child, &node->children, siblings) {
        if(strcmp(child->name, nodeName) == 0) {
            if (attrName) {
                XmlNodeAttribute *attr = XmlGetAttributeByName(child, attrName);
                if (attr) {
                    if (attrVal) {
                        char *dexmlized = dexmlize(attrVal);
                        if (strcmp(attr->value, dexmlized) != 0) {
                            free(dexmlized);
                            continue; // the attr value doesn't match .. let's skip to next matching node
                        }
                        free(dexmlized);
                    }
                    free(nodeName);
                    return child;
                }
            } else if (i == 0) {
                free(nodeName);
                return child;
            } else {
                i--;
            }
        }
    }
    free(nodeName);
    return NULL;
}

XmlNode *
XmlGetNode(TXml *xml, char *path)
{
    char *buff, *walk;
    char *tag;
    unsigned long i = 0;
    XmlNode *cNode = NULL;
    XmlNode *wNode = NULL;
//#ifndef WIN32
    char *brkb;
//#endif
    if(!path)
        return NULL;

    buff = strdup(path);
    walk = buff;
    /* skip leading slashes '/' */
    while(*walk == '/')
        walk++;

    /* first get root node */
#ifndef WIN32
    tag  = strtok_r(walk, "/", &brkb);
#else
    tag = strtok(walk, "/");
#endif
    if(!tag) {
        free(buff);
        return NULL;
    }

    for(i = 1; i <= XmlCountBranches(xml); i++)
    {
        wNode = XmlGetBranch(xml, i);
        if(strcmp(wNode->name, tag) == 0)
        {
            cNode = wNode;
            break;
        }
    }
    if(!cNode) {
        free(buff);
        return NULL;
    }

    /* now cNode points to the root node ... let's find requested node */
#ifndef WIN32
    tag = strtok_r(NULL, "/", &brkb);
#else
    tag = strtok(NULL, "/");
#endif
    while(tag)
    {
        XmlNode *tmp;
        wNode = XmlGetChildNodeByName(cNode, tag);
        if(!wNode) {
            free(buff);
            return NULL;
        }
        cNode = wNode; // update current node
#ifndef WIN32
        tag = strtok_r(NULL, "/", &brkb);
#else
        tag = strtok(NULL, "/");
#endif
    }

    free(buff);
    return cNode;
}

XmlNode
*XmlGetBranch(TXml *xml, unsigned long index)
{
    XmlNode *node;
    int cnt = 1;
    if(!xml)
        return NULL;
    TAILQ_FOREACH(node, &xml->rootElements, siblings) {
        if (cnt == index)
            return node;
        cnt++;
    }
    return NULL;
}

XmlErr
XmlSubstBranch(TXml *xml, unsigned long index, XmlNode *newBranch)
{
    XmlNode *branch, *tmp;
    int cnt = 1;
    TAILQ_FOREACH_SAFE(branch, &xml->rootElements, siblings, tmp) {
        if (cnt == index) {
            TAILQ_INSERT_BEFORE(branch, newBranch, siblings);
            TAILQ_REMOVE(&xml->rootElements, branch, siblings);
            return XML_NOERR;
        }
        cnt++;
    }
    return XML_LINKLIST_ERR;
}

#ifdef WIN32
//************************************************************************
// BOOL W32LockFile (FILE* filestream)
//
// locks the specific file for exclusive access, nonblocking
//
// returns 0 on success
//************************************************************************
static BOOL
W32LockFile (FILE* filestream)
{
    BOOL res = TRUE;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    unsigned long size = 0;
    int fd = 0;

    // check params
    if (!filestream)
        goto __exit;

    // get handle from stream
    fd = _fileno (filestream);
    hFile = (HANDLE)_get_osfhandle(fd);

    // lock file until access is permitted
    size = GetFileSize(hFile, NULL);
    res = LockFile (hFile, 0, 0, size, 0);
    if (res)
        res = 0;
__exit:
    return res;
}

//************************************************************************
// BOOL W32UnlockFile (FILE* filestream)
//
// unlocks the specific file locked by W32LockFile
//
// returns 0 on success
//************************************************************************
static BOOL
W32UnlockFile (FILE* filestream)
{
    BOOL res = TRUE;
    HANDLE hFile = INVALID_HANDLE_VALUE;
    unsigned long size = 0;
    int tries = 0;
    int fd = 0;

    // check params
    if (!filestream)
        goto __exit;

    // get handle from stream
    fd = _fileno (filestream);
    hFile = (HANDLE)_get_osfhandle(fd);

    // unlock
    size = GetFileSize(hFile, NULL);
    res = UnlockFile (hFile, 0, 0, size, 0);
    if (res)
        res = 0;

__exit:
    return res;
}
#endif // #ifdef WIN32

XmlErr
XmlFileLock(FILE *file)
{
    int tries = 0;
    if(file) {
#ifdef WIN32
        while(W32LockFile(file) != 0) {
#else
        while(ftrylockfile(file) != 0) {
#endif
    // warning("can't obtain a lock on xml file %s... waiting (%d)", xmlFile, tries);
            tries++;
            if(tries>5) {
                fprintf(stderr, "sticky lock on xml file!!!");
                return XML_GENERIC_ERR;
            }
            sleep(1);
        }
        return XML_NOERR;
    }
    return XML_GENERIC_ERR;
}

XmlErr XmlFileUnlock(FILE *file)
{
    if(file) {
#ifdef WIN32
        if(W32UnlockFile(file) == 0)
#else
        funlockfile(file);

#endif
        return XML_NOERR;
    }
    return XML_GENERIC_ERR;
}

