/* linked list management library - by xant 
 *
 */
#ifndef __LINKLIST_H__
#define __LINKLIST_H__

#include <stdlib.h>
#include <sys/types.h>
#ifdef THREADED
#ifdef WIN32
#include <w32_pthread.h>
#else
#include <pthread.h>
#endif
#endif
#include <string.h> // for memset

/* this structure represent a TaggedValue and is the main datatype 
 * you will have to handle when workin with the tagged-based api. 
 * If user extract such structure from the list (removing it from the list)
 * then he MUST release its resources trough a call to DestroyTaggedValue
 * when finished using it.
 * If a new TaggedValue must be created and inserted in a list, then 
 * CreateTaggedValue() should be used to allocate resources and obtain 
 * a pointer to a TaggedValue structure.
 */ 
typedef struct __TaggedValue {
	char *tag;
	void *value;
	unsigned long vLen;
} TaggedValue;

/* opaque structures. User should never access their members directly */
typedef struct __ListEntry {
	struct __LinkedList *list;
	struct __ListEntry *prev;
	struct __ListEntry *next;
	void *value;
	int tagged;
} ListEntry;

typedef struct __LinkedList {
	ListEntry *head;
	ListEntry *tail;
	unsigned long length;
#ifdef THREADED
	pthread_mutex_t lock;
#endif
	int free;
} LinkedList;

/********************************************************************
 * Common API 
 ********************************************************************/

/* List creation and destruction routines */
LinkedList *CreateList();
void InitList(LinkedList *list);
void DestroyList(LinkedList *list);
void ClearList(LinkedList *list);

unsigned long ListLength(LinkedList *list);

/********************************************************************
 * Tag-based API 
 ********************************************************************/

/* List creation and destruction routines */

/* Tagged List access routines (same of previous but with tag support */
TaggedValue *CreateTaggedValue(char *tag,void *val,unsigned long vLen);
void DestroyTaggedValue(TaggedValue *tVal);
TaggedValue *PopTaggedValue(LinkedList *list);
int PushTaggedValue(LinkedList *list,TaggedValue *tVal);
int UnshiftTaggedValue(LinkedList *list,TaggedValue *tVal);
TaggedValue *ShiftTaggedValue(LinkedList *list);
int InsertTaggedValue(LinkedList *list,TaggedValue *tVal,unsigned long pos);
TaggedValue *PickTaggedValue(LinkedList *list,unsigned long pos);
TaggedValue *FetchTaggedValue(LinkedList *list,unsigned long pos);
TaggedValue *GetTaggedValue(LinkedList *list,char *tag);
unsigned long GetTaggedValues(LinkedList *list,char *tag,LinkedList *values);

/********************************************************************
 * Value-based API 
 ********************************************************************/

/* List access routines */
void *PopValue(LinkedList *list);
int PushValue(LinkedList *list,void *val);
int UnshiftValue(LinkedList *list,void *val);
void *ShiftValue(LinkedList *list);
int InsertValue(LinkedList *list,void *val,unsigned long pos);
void *PickValue(LinkedList *list,unsigned long pos);
void *FetchValue(LinkedList *list,unsigned long pos);
int MoveValue(LinkedList *list,unsigned long srcPos,unsigned long dstPos);
void *SubstValue(LinkedList *list,unsigned long pos,void *newVal);
int SwapValues(LinkedList *list,unsigned long pos1,unsigned long pos2);
/* list iterator. This iterator can be used for both Tag-based and Value-based lists.
 * If tagged, items can simply be casted to a TaggedValue pointer.
 */
void ForEachListValue(LinkedList *list,void (*itemHandler)(void *item,unsigned long idx,void *user),void *user);

/********************************************************************
 * Entry-based API   
 * XXX - Internal use only ... maybe shouldn't be exported?
 ********************************************************************/

/* Entry creation and destruction routines */
ListEntry *CreateEntry();
void DestroyEntry(ListEntry *entry);
void *GetEntryValue(ListEntry *entry);
void SetEntryValue(ListEntry *entry,void *val);

/* List and ListEntry manipulation routines */
ListEntry *PopEntry(LinkedList *list);
int PushEntry(LinkedList *list,ListEntry *entry);
int UnshiftEntry(LinkedList *list,ListEntry *entry);
ListEntry *ShiftEntry(LinkedList *list);
int InsertEntry(LinkedList *list,ListEntry *entry,unsigned long pos);
ListEntry *PickEntry(LinkedList *list,unsigned long pos);
ListEntry *FetchEntry(LinkedList *list,unsigned long pos);
//ListEntry *SelectEntry(LinkedList *list,unsigned long pos);
ListEntry *RemoveEntry(LinkedList *list,unsigned long pos);
unsigned long GetEntryPosition(ListEntry *entry);
int MoveEntry(LinkedList *list,unsigned long srcPos,unsigned long dstPos);
ListEntry *SubstEntry(LinkedList *list,unsigned long pos,ListEntry *entry);
int SwapEntries(LinkedList *list,unsigned long pos1,unsigned long pos2);

#endif
