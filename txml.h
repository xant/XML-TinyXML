/*
 *  tinyxml.h
 *
 *  Created by xant on 2/17/06.
 *
 */

#ifndef __TINYXML_H__
#define __TINYXML_H__

#define XmlErr int
#define XML_NOERR 0
#define XML_GENERIC_ERR -1
#define XML_BADARGS -2
#define XML_UPDATE_ERR -2
#define XML_OPEN_FILE_ERR -3
#define XML_PARSER_GENERIC_ERR -4
#define XML_MEMORY_ERR -5
#define XML_LINKLIST_ERR -6

#include <linklist.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef WIN32
#include <windows.h> // for w32lock/unlock functions
#include <io.h>
/* strings */
#define snprintf _snprintf
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#define strdup _strdup
/* files */
#define stat _stat
/* time */
#define sleep(_duration) (Sleep(_duration * 1000))
#endif // WIN32

/**
    @type XmlNodeAttribute
    @brief One attribute associated to an element 
*/
typedef struct {
    char *name; ///< the attribute name
    char *value; ///< the attribute value
} XmlNodeAttribute;

typedef struct __XmlNode {
    char *path;
    char *name;
    struct __XmlNode *parent;
    char *value;
    LinkedList *children;
    LinkedList *attributes;
#define XML_NODETYPE_SIMPLE 0
#define XML_NODETYPE_COMMENT 1
#define XML_NODETYPE_CDATA 2
    char type;
} XmlNode;

typedef struct {
    XmlNode *cNode;
    LinkedList *rootElements;
    char *head;
} TXml;

/***
    @brief allocates memory for an XmlNode. In case of errors NULL is returned 
    @arg name of the new node
    @arg value associated to the new node (can be NULL and specified later through XmlSetNodeValue function)
    @arg parent of the new node if present, NULL if this will be a root node
    @return the newly created node 
 */
XmlNode *XmlCreateNode(char *name,char *val,XmlNode *parent);
/*** 
    @brief associate a value to XmlNode *node. XML_NOERR is returned if no error occurs 
    @arg the node we want to modify
    @arg the value we want to set for node
    @return XML_NOERR if success , error code otherwise
 */
XmlErr XmlSetNodeValue(XmlNode *node,char *val);
/***
    @brief get value for an XmlNode
    @arg the XmlNode containing the value we want to access.
    @return returns value associated to XmlNode *node 
 */
char *XmlGetNodeValue(XmlNode *node);
/****
    @brief free resources for XmlNode *node and all its subnodes 
    @arg the XmlNode we want to destroy
 */
void XmlDestroyNode(XmlNode *node);
/*** 
    @brief Adds XmlNode *child to the children list of XmlNode *node 
    @arg the parent node
    @arg the new child
    @return return XML_NOERR on success, error code otherwise 
*/
XmlErr XmlAddChildNode(XmlNode *parent,XmlNode *child);
/***
    @brief Makes XmlNode *node a root node in context represented by TXml *xml 
    @arg the xml context pointer
    @arg the new root node
    @return XML_NOERR on success, error code otherwise
 */
XmlErr XmlAddRootNode(TXml *xml,XmlNode *node);
/***
    @brief add an attribute to XmlNode *node 
    @arg the XmlNode that we want to set attributes to 
    @arg the name of the new attribute
    @arg the value of the new attribute
    @return XML_NOERR on success, error code otherwise
 */
XmlErr XmlAddAttribute(XmlNode *node,char *name,char *val);
/***
    @brief substitute an existing branch with a new one
    @arg the xml context pointer
    @arg the index of the branch we want to substitute
    @arg the root of the new branch
    @reurn XML_NOERR on success, error code otherwise
 */
XmlErr XmlSubstBranch(TXml *xml,unsigned long index, XmlNode *newBranch);
/***
    @brief Remove a specific node from the xml structure
    XXX - UNIMPLEMENTED
 */
XmlErr XmlRemoveNode(TXml *xml,char *path);
/***
    XXX - UNIMPLEMENTED
 */
XmlErr XmlRemoveBranch(TXml *xml,unsigned long index);
/***
    @brief Returns the number of root nodes in the xml context 
    @arg the xml context pointer
    @return the number of root nodes found in the xml context
 */
unsigned long XmlCountBranches(TXml *xml);
/***
    @brief Returns the number of children of the given XmlNode
    @arg the node we want to query
    @return the number of children of queried node
 */
unsigned long XmlCountChildren(XmlNode *node);
/***
    @brief Returns the number of attributes of the given XmlNode
    @arg the node we want to query
    @return the number of attributes that are set for queried node
 */
unsigned long XmlCountAttributes(XmlNode *node);
/***
    @brief Returns the XmlNode at specified path
    @arg the xml context pointer
    @arg the path that references requested node. 
        This must be of formatted as a slash '/' separated list
        of node names ( ex. "tag_A/tag_B/tag_C" )
    @return the node at specified path
 */
XmlNode *XmlGetNode(TXml *xml, char *path);
/***
    @brief get the root node at a specific index
    @arg the xml context pointer
    @arg the index of the requested root node
    @return the root node at requested index

 */
XmlNode *XmlGetBranch(TXml *xml,unsigned long index);
/***
    @brief get the child at a specific index inside a node
    @arg the node 
    @arg the index of the child we are interested in
    @return the selected child node 
 */
XmlNode *XmlGetChildNode(XmlNode *node,unsigned long index);
/***
    @brief get the first child of an XmlNode whose name is 'name'
    @arg the parent node
    @arg the name of the desired child node
    @return the requested child node
 */
XmlNode *XmlGetChildNodeByName(XmlNode *node,char *name);

/***
    @brief parse a string buffer containing an xml profile and fills internal structures appropriately
    @arg the null terminated string buffer containing the xml profile
    @return true if buffer is parsed successfully , false otherwise)
*/
XmlErr XmlParseBuffer(TXml *xml,char *buf);

/***
    @brief parse an xml file containing the profile and fills internal structures appropriately
    @arg a null terminating string representing the path to the xml file
    @return an XmlErr error status (XML_NOERR if buffer was parsed successfully)
*/
XmlErr XmlParseFile(TXml *xml,char *path);

char *XmlDumpBranch(TXml *xml,XmlNode *rNode,unsigned int depth);
/***
    @brief dump the entire xml configuration tree that reflects the status of internal structures
    @return a null terminated string containing the xml representation of the configuration tree.
    The memory allocated for the dump-string must be freed by the user when no more needed
*/
char *XmlDump(TXml *xml);

TXml *XmlCreateContext();
void XmlDestroyContext(TXml *xml);


XmlNodeAttribute *XmlGetAttribute(XmlNode *node,unsigned long index);

int XmlRemoveAttribute(XmlNode *node, unsigned long index);
void XmlClearAttributes(XmlNode *node);


/***
    @brief save the configuration stored in the xml file containing the current profile
    the xml file name is obtained appending '.xml' to the category name . The xml file is stored 
    in the repository directory specified during object construction.
    @return an XmlErr error status (XML_NOERR if buffer was parsed successfully)
*/
XmlErr XmlSave(TXml *xml,char *path);

XmlErr XmlFileLock(FILE *file);
XmlErr XmlFileUnlock(FILE *file);

#endif
