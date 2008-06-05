/*
 *  tinyxml.c
 *
 *  Created by xant on 2/17/06.
 *
 */

#include "txml.h"

#define XML_ELEMENT_NONE 0
#define XML_ELEMENT_START 1
#define XML_ELEMENT_VALUE 2
#define XML_ELEMENT_END 3
#define XML_ELEMENT_UNIQUE 4

TXml *XmlCreateContext()
{
    TXml *xml;
    
    xml = (TXml *)calloc(1, sizeof(TXml));
    xml->cNode = NULL;
    xml->rootElements = CreateList();
    xml->head = NULL;
    return xml;
}

void XmlDestroyContext(TXml *xml)
{
    XmlNode *rNode;
    if(xml->rootElements)
    {
        while(rNode = ShiftValue(xml->rootElements))
            XmlDestroyNode(rNode);
        DestroyList(xml->rootElements);
    }
    if(xml->head)
        free(xml->head);
    free(xml);
}

XmlNode *XmlCreateNode(char *name, char *value, XmlNode *parent)
{
    XmlNode *node = NULL;
    unsigned int pathLen;
    node = (XmlNode *)calloc(1, sizeof(XmlNode));
    if(!node || !name)
        return NULL;
    
    node->attributes = CreateList();
    node->children = CreateList();
    node->parent = parent;
    node->name = strdup(name);

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
        node->path = strdup(node->name);
    }
    if(value && strlen(value) > 0)
        node->value = strdup(value);
    else
        node->value = calloc(1, 1);
    return node;
}

void XmlDestroyNode(XmlNode *node)
{
    XmlNode *child;
    XmlNodeAttribute *attr;
    
    if(node->attributes) {
        while(attr = ShiftValue(node->attributes)) {
            if(attr->name)
                free(attr->name);
            if(attr->value)
                free(attr->value);
            free(attr);
        }
        DestroyList(node->attributes);
    }
    if(node->children) {
        while(child = ShiftValue(node->children))
            XmlDestroyNode(child);
        DestroyList(node->children);
    }
    if(node->name) 
        free(node->name);
    if(node->path)
        free(node->path);
    if(node->value)
        free(node->value);
    free(node);
}

XmlErr XmlSetNodeValue(XmlNode *node, char *val)
{
    if(!val) 
        return XML_BADARGS;
        
    if(node->value)
        free(node->value);
    node->value = strdup(val);
    return XML_NOERR;
}

/* quite useless */
char *XmlGetNodeValue(XmlNode *node)
{
    if(!node)
        return NULL;
    return node->value;
}

XmlErr XmlAddChildNode(XmlNode *parent, XmlNode *child)
{
    if(!child)
        return XML_BADARGS;
    
    if(PushValue(parent->children, child)) {
        child->parent = parent;
        return XML_NOERR;
    }
        
    return XML_GENERIC_ERR;
}

XmlErr XmlAddRootNode(TXml *xml, XmlNode *node)
{
    if(!node)
        return XML_BADARGS;
    
    if(PushValue(xml->rootElements, node))
        return XML_NOERR;
        
    return XML_GENERIC_ERR;
}

XmlErr XmlAddAttribute(XmlNode *node, char *name, char *val)
{
    XmlNodeAttribute *attr;
    
    if(!name || !node)
        return XML_BADARGS;
        
    attr = calloc(1, sizeof(XmlNodeAttribute));
    attr->name = strdup(name);
    attr->value = val?strdup(val):strdup("");
    
    if(PushValue(node->attributes, attr))
        return XML_NOERR;
    free(attr->name);
    free(attr->value);
    free(attr);
    return XML_GENERIC_ERR;
}

int XmlRemoveAttribute(XmlNode *node, unsigned long index)
{
    XmlNodeAttribute *attr = FetchValue(node->attributes, index);
    if(attr) {
        free(attr->name);
        free(attr->value);
        free(attr);
        return XML_NOERR;
    }
    return XML_GENERIC_ERR;
}

void XmlClearAttributes(XmlNode *node)
{
    unsigned int nAttrs = 0;
    int i;
    XmlNodeAttribute *attr = NULL;

    nAttrs = XmlCountAttributes(node);
    if(nAttrs>0) {
        for(i=nAttrs;i>0;i--) {
            XmlRemoveAttribute(node, i); // TODO - check return code
        }
    }

}

XmlNodeAttribute *XmlGetAttribute(XmlNode *node, unsigned long index)
{
    return PickValue(node->attributes, index);
}

static XmlErr XmlExtraNodeHandler(TXml *xml, char *content, char type)
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

static XmlErr XmlStartHandler(TXml *xml, char *element, char **attr_names, char **attr_values)
{
    XmlNode *newNode = NULL;
    unsigned int offset = 0;
    XmlErr res = XML_NOERR;
    
    if(!element || strlen(element) == 0)
    {
        res = XML_BADARGS;
        goto _start_done;
    }
    
    newNode = XmlCreateNode(element, NULL, xml->cNode);
    if(!newNode || !newNode->name)
    {
        /* XXX - ERROR MESSAGES HERE */
        res = 1;
        goto _start_done;
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

static XmlErr XmlEndHandler(TXml *xml, char *element)
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

static XmlErr XmlValueHandler(TXml *xml, char *text)
{
    char *p;
    if(text) {
        while((*text == ' ' || *text == '\t' ||
            *text == '\r' || *text == '\n') && *text != 0) text++;
        p = text+strlen(text)-1;
        while((*p == ' ' || *p == '\t' ||
            *p == '\r' || *p == '\n') && p != text) 
        {
            *p=0;
            p--;
        }
        if(text) {
            if(xml->cNode) 
                XmlSetNodeValue(xml->cNode, text);
            else 
            {
                fprintf(stderr, "cTag == NULL while handling a value!!");
            }
        }
        return XML_NOERR;
    }
    return XML_GENERIC_ERR;
}


XmlErr XmlParseBuffer(TXml *xml, char *buf)
{
    XmlErr err = XML_NOERR;
    int state=XML_ELEMENT_NONE;
    char *p=buf;
    unsigned int i;
    char *start = NULL;
    char *end = NULL;
    char **attrs = NULL;
    char **values = NULL;
    unsigned int nAttrs = 0;
    char *mark = NULL;
                
    //unsigned int offset=fileStat.st_size;

#define XML_FREE_ATTRIBUTES \
    if(nAttrs>0) {\
        for(i=0;i<nAttrs;i++) {\
            if(attrs[i]) free(attrs[i]);\
            if(values[i]) free(values[i]);\
        }\
        free(attrs);\
        attrs=NULL;\
        free(values);\
        values=NULL;\
        nAttrs=0;\
    }\

#define SKIP_BLANKS(__p) \
    while((*__p==' ' || *__p=='\t' || *__p=='\r' || *__p == '\n') && *__p!=0) __p++; \
    if(*__p==0) break;
        
#define ADVANCE_ELEMENT(__p) \
    while(*__p!='>' && *__p!=' ' && *__p!='\t' && *__p!='\r' && *__p != '\n' && *__p!=0) __p++; \
    if(*__p==0) break;
        
#define ADVANCE_TO_ATTR_VALUE(__p) \
    while(*__p!='=' && *__p!=' ' && *__p!='\t' && *__p!='\r' && *__p != '\n' && *__p!=0) __p++;\
    SKIP_BLANKS(__p);

    while(*p != 0) {
        SKIP_BLANKS(p);
        if(*p == '<') {
            p++;
            if(*p == '/') {
                p++;
                SKIP_BLANKS(p);
                mark = p;
                while(*p != '>' && *p != 0) p++;
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
                    if(err!=XML_NOERR)
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
                p++;
                mark = p;
                p = strstr(mark, "?>");
                if(xml->head)
                    free(xml->head); /* XXX - should notify this behaviour? */
                xml->head = calloc(1, p-mark+1);
                strncpy(xml->head, mark, p-mark);
                p+=2;
            } else { /* start tag */
                attrs = NULL;
                values = NULL;
                nAttrs = 0;
                state=XML_ELEMENT_START;
                SKIP_BLANKS(p);
                mark=p;
                ADVANCE_ELEMENT(p);
                start = (char *)malloc(p-mark+2);
                if(start == NULL) 
                    return XML_MEMORY_ERR;
                strncpy(start, mark, p-mark);
                if(*p=='>' && *(p-1)=='/') {
                    start[p-mark-1]=0;
                    state=XML_ELEMENT_UNIQUE;
                }
                else {
                    start[p-mark]=0;
                }
                SKIP_BLANKS(p);
                while(*p!='>' && *p!=0) {
                    mark=p;
                    ADVANCE_TO_ATTR_VALUE(p);
                    if(*p=='=') {
                        char *tmpAttr=(char *)malloc(p-mark+1);
                        strncpy(tmpAttr, mark, p-mark);
                        tmpAttr[p-mark]=0;
                        p++;
                        SKIP_BLANKS(p);
                        if(*p == '"' || *p == '\'') {
                            int quote = *p;
                            p++;
                            mark=p;
                            while(*p!=quote && *p!=0) p++;
                            if(*p==quote) {
                                char *tmpVal = (char *)malloc(p-mark+2);
                                strncpy(tmpVal, mark, p-mark);
                                tmpVal[p-mark]=0;
                                /* add new attribute */
                                nAttrs++;
                                attrs=(char **)realloc(attrs, sizeof(char *)*(nAttrs+1));
                                attrs[nAttrs-1]=tmpAttr;
                                attrs[nAttrs]=NULL;
                                values=(char **)realloc(values, sizeof(char *)*(nAttrs+1));
                                values[nAttrs-1]=tmpVal;
                                values[nAttrs]=NULL;
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
                        state=XML_ELEMENT_UNIQUE;
                    }
                } /* while(*p!='>' && *p!=0) */
                err = XmlStartHandler(xml, start, attrs, values);
                if(err!=XML_NOERR) {
                    XML_FREE_ATTRIBUTES
                    free(start);
                    return err;
                }
                if(state==XML_ELEMENT_UNIQUE) {
                    err = XmlEndHandler(xml, start);
                    if(err!=XML_NOERR) {
                        XML_FREE_ATTRIBUTES
                        free(start);
                        return err;
                    }
                }
                XML_FREE_ATTRIBUTES
                free(start);
                p++;
            } /* end of start tag */
        } /* if(*p=='<') */
        else if(state == XML_ELEMENT_START) {
            state=XML_ELEMENT_VALUE;
            mark=p;
            while(*p != '<' && *p != 0) p++;
            if(*p == '<') {
                char *value = (char *)malloc(p-mark+1);
                strncpy(value, mark, p-mark);
                value[p-mark]=0;
                err=XmlValueHandler(xml, value);
                if(value)
                    free(value);
                if(err!=XML_NOERR)
                    return(err);
                //p++;
            }
        }
        else {
            /* XXX */
            p++;
        }
    } // while(*p!=0)

_parser_err:
    return err;
}


XmlErr XmlParseFile(TXml *xml, char *path)
{
    FILE *inFile;
    char *buffer;
    XmlErr err;
    struct stat fileStat;
        
    inFile = NULL;
    err = XML_NOERR;
    if(!path)
        return XML_BADARGS;
    stat(path, &fileStat);
    xml->cNode=NULL;
    if(fileStat.st_size>0) {
        inFile=fopen(path, "r");
        if(inFile) {
            if(XmlFileLock(inFile) != XML_NOERR) {
                fprintf(stderr, "Can't lock %s for opening ", path);
                return -1;
            }
            buffer=(char *)malloc(fileStat.st_size+1);
            fread(buffer, 1, fileStat.st_size, inFile);
            buffer[fileStat.st_size]=0;
            err=XmlParseBuffer(xml, buffer);
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

char *XmlDumpBranch(TXml *xml, XmlNode *rNode, unsigned int depth)
{
    unsigned int i, n;
    char *out = NULL;
    char *startTag;
    char *endTag;    
    char *childDump;
    char *value; 
    char *name;
    int nameLen;
    XmlNodeAttribute *attr;
    XmlNode *child;
    unsigned long nAttrs;
    
    value = rNode->value;

    name = rNode->name;
    if(name)
        nameLen=(unsigned int)strlen(name);
    else
        return NULL;

    /* First check if this is a special node (a comment or a CDATA) */
    if(rNode->type == XML_NODETYPE_COMMENT) {
        out = malloc(strlen(value)+depth+9);
        *out = 0;
        for(n=0;n<depth;n++) strcat(out, "\t");
        strcat(out, "<!--");
        strcat(out, value);
        strcat(out, "-->\n");
        return out;
    } else if(rNode->type == XML_NODETYPE_CDATA) {
        out = malloc(strlen(value)+depth+14);
        *out = 0;
        for(n=0;n<depth;n++)
            strcat(out, "\t");
        strcat(out, "<![CDATA[");
        strcat(out, value);
        strcat(out, "]]>\n");
        return out;
    }

    childDump = (char *)malloc(1);
    *childDump=0;
    
    startTag=(char *)malloc(depth+nameLen+7);
    memset(startTag, 0, depth+nameLen+7);
    endTag=(char *)malloc(depth+nameLen+7);
    memset(endTag, 0, depth+nameLen+7);
    
    for(n = 0; n < depth; n++)
        strcat(startTag, "\t");
    strcat(startTag, "<");
    strcat(startTag, name);
    nAttrs = XmlCountAttributes(rNode);
    if(nAttrs>0) {
        for(i=1;i<=nAttrs;i++) {
            attr = XmlGetAttribute(rNode, i);
            if(attr) {
                startTag = (char *)realloc(startTag, strlen(startTag)+
                    strlen(attr->name)+strlen(attr->value)+8);
                strcat(startTag, " ");
                strcat(startTag, attr->name);
                strcat(startTag, "=\"");
                strcat(startTag, attr->value); /* XXX - should escape '"' char */
                if(i < nAttrs)
                    strcat(startTag, "\" ");
                else
                    strcat(startTag, "\"");
            }
        }
    }
    if(value || XmlCountChildren(rNode)) {
        if(XmlCountChildren(rNode) > 0) {
            strcat(startTag, ">\n");
            for(n = 0; n < depth; n++)
                strcat(endTag, "\t");
            for(i = 1; i <= XmlCountChildren(rNode); i++) {
                child = XmlGetChildNode(rNode, i);
                if(child) {
                    char *childBuff = XmlDumpBranch(xml, child, depth+1); /* let's recurse */
                    if(childBuff) {
                        childDump = (char *)realloc(childDump, strlen(childDump)+strlen(childBuff)+2);
                        strcat(childDump, childBuff);
                        //strcat(childDump, "\n");
                        free(childBuff);
                    }
                }
            }
        }
        else {
            // TODO - allow to specify a flag to determine if we want white spaces or not
            //strcat(startTag, "> ");
            strcat(startTag, ">"); 
        }
        strcat(endTag, "</");
        strcat(endTag, rNode->name);
        strcat(endTag, ">\n");
        out = (char *)malloc(depth+strlen(startTag)+strlen(endTag)+
            (value?strlen(value)+1:1)+strlen(childDump)+3);
        strcpy(out, startTag);
        if(value && *value) { // skip also if value is an empty string (not only if it's a null pointer)
            if(XmlCountChildren(rNode)) {
                for(n=0;n<depth;n++)
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
        out=strdup(startTag);
    }
    free(startTag);
    free(endTag);
    free(childDump);
    return out;
}

char *XmlDump(TXml *xml)
{
    char *dump; 
    XmlNode *rNode;
    char *branch;
    unsigned int i;
    char *head;
    
    head = xml->head?xml->head:"xml version=\"1.0\"";
    dump = malloc(strlen(head)+6);
    sprintf(dump, "<?%s?>\n", head);
    for (i=1;i<=ListLength(xml->rootElements);i++) {
        rNode = (XmlNode *)PickValue(xml->rootElements, i);
        if(rNode) {
            branch = XmlDumpBranch(xml, rNode, 0);
            if(branch) {
                dump = (char *)realloc(dump, strlen(dump)+strlen(branch)+1);
                strcat(dump, branch);
                free(branch);
            }
        }
    }
    return(dump);
}

XmlErr XmlSave(TXml *xml, char *xmlFile)
{
    struct stat fileStat;
    FILE *saveFile=NULL;
    char *dump = NULL;
    char *backup = NULL;
    char *backupPath = NULL;
    FILE *backupFile = NULL;
    
    stat(xmlFile, &fileStat);
    
    if(fileStat.st_size>0) { /* backup old profiles */
        saveFile=fopen(xmlFile, "r");
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
        backup[fileStat.st_size]=0;
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
    dump = XmlDump(xml);
     if(dump) {
        saveFile=fopen(xmlFile, "w+");
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

unsigned long XmlCountAttributes(XmlNode *node)
{
    return ListLength(node->attributes);
}

unsigned long XmlCountChildren(XmlNode *node)
{
    return ListLength(node->children);
}

unsigned long XmlCountBranches(TXml *xml)
{
    return ListLength(xml->rootElements);
}

XmlErr XmlRemoveNode(TXml *xml, char *path)
{
    /* XXX - UNIMPLEMENTED */
    return XML_GENERIC_ERR;
}

XmlErr XmlRemoveBranch(TXml *xml, unsigned long index)
{
    /* XXX - UNIMPLEMENTED */
    return XML_GENERIC_ERR;
}

XmlNode *XmlGetChildNode(XmlNode *node, unsigned long index) 
{
    if(!node) 
        return NULL;
    return PickValue(node->children, index);
}

/* XXX - if multiple children shares the same name, only the first is returned */
XmlNode *XmlGetChildNodeByName(XmlNode *node, char *name) 
{
    XmlNode *child;
    unsigned int i;
    if(!node) 
        return NULL;
    
    for(i=1; i <= XmlCountChildren(node); i++)
    {
        child = XmlGetChildNode(node, i);
        if(strcmp(child->name, name) == 0)
            return child;
        
    }
    return NULL;
}

XmlNode *XmlGetNode(TXml *xml, char *path)
{
    char *walk;
    char *tag;
    unsigned long i = 0;
    XmlNode *cNode = NULL;
    XmlNode *wNode = NULL;
//#ifndef WIN32
    char *brkb;
//#endif
    if(!path) 
        return NULL;
        
    walk = strdup(path);
    /* skip leading slashes '/' */
    while(*walk == '/')
        walk++;
        
    /* first get root node */
//#ifndef WIN32
    tag  = strtok_r(walk, "/", &brkb);
//#else
//    tag = strtok(walk, "/");
//#endif
    if(!tag)
        return NULL;

    for(i=1; i<=XmlCountBranches(xml); i++)
    {
        wNode = XmlGetBranch(xml, i);
        if(strcmp(wNode->name, tag) == 0)
        {
            cNode = wNode;
            break;
        }
    }
    if(!cNode)
        return NULL;
    
    /* now cNode points to the root node ... let's find requested node */
//#ifndef WIN32
    tag = strtok_r(NULL, "/", &brkb);
//#else
//    tag = strtok(NULL, "/");
//#endif
    while(tag)
    {
        for(i=1; i<=XmlCountChildren(cNode); i++)
        {
            wNode = XmlGetChildNode(cNode, i);
            if(strcmp(wNode->name, tag) == 0)
            {
                cNode = wNode;
                break;
            }
            wNode = NULL;
        }
        if(!wNode)
            return NULL;
//#ifndef WIN32
        tag = strtok_r(NULL, "/", &brkb);
//#else
//        tag = strtok(NULL, "/");
//#endif
    }
    
    return cNode;
}

XmlNode *XmlGetBranch(TXml *xml, unsigned long index)
{
    if(!xml) 
        return NULL;
    return PickValue(xml->rootElements, index);
}

XmlErr XmlSubstBranch(TXml *xml, unsigned long index, XmlNode *newBranch)
{
    XmlNode *oldBranch = (XmlNode *)SubstValue(xml->rootElements, index, newBranch);
    if(oldBranch)
        return XML_NOERR;
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
static BOOL W32LockFile (FILE* filestream)
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
static BOOL W32UnlockFile (FILE* filestream)
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

XmlErr XmlFileLock(FILE *file)
{
    int tries=0;
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
        if(W32UnlockFile(file)==0) 
#else
        funlockfile(file);

#endif
        return XML_NOERR;
    }
    return XML_GENERIC_ERR;
}

