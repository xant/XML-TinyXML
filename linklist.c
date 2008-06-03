/* linked list management library - by xant */
 
#include <stdio.h>
#include "linklist.h"

/* 
 * Create a new LinkedList. Allocates resources and returns 
 * a LinkedList opaque structure for later use 
 */
LinkedList *CreateList() 
{
	LinkedList *list = (LinkedList *)malloc(sizeof(LinkedList));
	if(list)
	{
#ifdef DEBUG
		printf("(linklist.c) Created new LinkedList at address 0x%p \n",list);
#endif
		InitList(list);
		list->free = 1;
	}
	return list;
}

/*
 * Initialize a preallocated LinkedList pointed by list 
 * useful when using static list handlers
 */ 
void InitList(LinkedList *list) 
{
	memset(list,0,sizeof(LinkedList));
#ifdef THREADED
	pthread_mutex_init(&list->lock,NULL);
#endif
#ifdef DEBUG
	printf("(linklist.c) Initialized LinkedList at address 0x%p \n",list);
#endif
}

/*
 * Destroy a LinkedList. Free resources allocated for list
 */
void DestroyList(LinkedList *list) 
{
	if(list) 
	{
		ClearList(list);
#ifdef THREADED
		pthread_mutex_destroy(&list->lock);
#endif
		if(list->free) free(list);
	}
#ifdef DEBUG
	printf("(linklist.c) Destroyed LinkedList at address 0x%p \n",list);
#endif
}

/*
 * Clear a LinkedList. Removes all entries in list
 * Dangerous if used with value-based api ... 
 * if values are associated to entries, resources for those will not be freed.
 * ClearList() can be used safely with entry-based and tagged-based api,
 * otherwise you must really know what you are doing
 */
void ClearList(LinkedList *list) 
{
	ListEntry *e;
	/* Destroy all entries still in list */
	while((e = ShiftEntry(list)) != NULL)
	{
		/* if there is a TaggedValue associated to the entry, 
		* let's free memory also for it */
		if(e->tagged && e->value)
			DestroyTaggedValue(e->value);
	
		DestroyEntry(e);
	}
}

/* Returns actual lenght of LinkedList pointed by l */
unsigned long ListLength(LinkedList *l) 
{
	unsigned long len;
#ifdef THREADED
	pthread_mutex_lock(&l->lock);
#endif
	len = l->length;
#ifdef THREADED
	pthread_mutex_unlock(&l->lock);
#endif
	return len;
}

/* 
 * Create a new ListEntry structure. Allocates resources and returns  
 * a pointer to the just created ListEntry opaque structure
 */
ListEntry *CreateEntry() 
{
	ListEntry *newEntry = (ListEntry *)calloc(1,sizeof(ListEntry));
#ifdef DEBUG
		printf("(linklist.c) Created Entry at address 0x%p \n",newEntry);
#endif
	return newEntry;
}

/* 
 * Free resources allocated for a ListEntry structure. 
 * If the entry is linked in a list this routine will also unlink correctly
 * the entry from the list.
 */
void DestroyEntry(ListEntry *entry) 
{
	unsigned long pos;
	if(entry) 
	{
		if(entry->list) 
		{
			/* entry is linked in a list...let's remove that reference */
			pos = GetEntryPosition(entry);
			if(pos) RemoveEntry(entry->list,pos);
		}
#ifdef DEBUG
		printf("(linklist.c) Destroyed Entry at address 0x%p \n",entry);
#endif
		free(entry);
	}
}

/*
 * Get a pointer to the value associated to entry. 
 */
void *GetEntryValue(ListEntry *entry) 
{
	return entry->value;
}

/* 
 * Associate a value to the ListEntry pointed by entry.
 * 
 */
void SetEntryValue(ListEntry *entry,void *val) 
{
	entry->value = val;
}

/*
 * Pops a ListEntry from the end of the list (or bottom of the stack
 * if you are using the list as a stack)
 */
ListEntry *PopEntry(LinkedList *list) 
{
	ListEntry *entry;
#ifdef THREADED
	pthread_mutex_lock(&list->lock);
#endif
	entry = list->tail;
	if(entry) 
	{
		list->tail = entry->prev;
		if(list->tail)
			list->tail->next = NULL;
		list->length--;
		entry->list = NULL;
	}
	if(list->length == 0)
		list->head = list->tail = NULL;
#ifdef THREADED
	pthread_mutex_unlock(&list->lock);
#endif
	return entry;
}

/*
 * Pushs a ListEntry at the end of a list
 */
int PushEntry(LinkedList *list,ListEntry *entry) 
{
	ListEntry *p;
	if(!entry) return 0;
#ifdef THREADED
	pthread_mutex_lock(&list->lock);
#endif
	if(list->length == 0) 
	{
		list->head = list->tail = entry;
	}
	else 
	{
		p = list->tail;
		p->next = entry;
		entry->prev = p;
		entry->next = NULL;
		list->tail = entry;
	}
	list->length++;
	entry->list = list;
#ifdef THREADED
	pthread_mutex_unlock(&list->lock);
#endif
	return 1;
}
 
/*
 * Retreive a ListEntry from the beginning of a list (or top of the stack
 * if you are using the list as a stack) 
 */
ListEntry *ShiftEntry(LinkedList *list) 
{
	ListEntry *entry;
#ifdef THREADED
	pthread_mutex_lock(&list->lock);
#endif
	entry = list->head;
	if(entry) 
	{
		list->head = entry->next;
		if(list->head) 
			list->head->prev = NULL;
		list->length--;
		entry->list = NULL;
	}
	if(list->length == 0)
		list->head = list->tail = NULL;
#ifdef THREADED
	pthread_mutex_unlock(&list->lock);
#endif
	return entry;
}


/* 
 * Insert a ListEntry at the beginning of a list (or at the top if the stack)
 */
int UnshiftEntry(LinkedList *list,ListEntry *entry) 
{
	ListEntry *p;
	if(!entry) return 0;
#ifdef THREADED
	pthread_mutex_lock(&list->lock);
#endif
	if(list->length == 0) 
	{
		list->head = list->tail = entry;
	} 
	else 
	{
		p = list->head;
		p->prev = entry;
		entry->prev = NULL;
		entry->next = p;
		list->head = entry;
	}
	list->length++;
	entry->list = list;
#ifdef THREADED
	pthread_mutex_unlock(&list->lock);
#endif
	return 1;
}

/*
 * Instert an entry at a specified position in a LinkedList
 */
int InsertEntry(LinkedList *list,ListEntry *entry,unsigned long pos) 
{
	ListEntry *prev,*next;
	if(pos == 1) return UnshiftEntry(list,entry);
	else if(pos == list->length) return PushEntry(list,entry);
	prev = PickEntry(list,pos);
#ifdef THREADED
	pthread_mutex_lock(&list->lock);
#endif
	if(prev) 
	{
		next = prev->next;
		prev->next = entry;
		entry->prev = prev;
		entry->next = next;
		next->prev = entry;
		list->length++;
#ifdef THREADED
		pthread_mutex_unlock(&list->lock);
#endif
		return 1;
	}
#ifdef THREADED
	pthread_mutex_unlock(&list->lock);
#endif
	return 0;
}

/* 
 * Retreive the ListEntry at pos in a LinkedList without removing it from the list
 */
ListEntry *PickEntry(LinkedList *list,unsigned long pos) 
{
	int i;	
	ListEntry *entry;
	if(list->length < pos) return NULL;
#ifdef THREADED
	pthread_mutex_lock(&list->lock);
#endif
	if (pos > list->length/2) 
	{
		entry = list->tail;
		for(i=list->length;i>(int)pos;i--) 
			entry = entry->prev;
	}
	else 
	{
		entry = list->head;
		for(i=1;i<(int)pos;i++) 
			entry = entry->next;
	}
#ifdef THREADED
	pthread_mutex_unlock(&list->lock);
#endif
	return entry;
}

/* Retreive the ListEntry at pos in a LinkedList removing it from the list 
 * XXX - no locking here because this routine is just an accessor to other routines
 * XXX - POSSIBLE RACE CONDITION BETWEEN PickEntry and RemoveEntry
 * Caller MUST destroy returned entry trough DestroyEntry() call
 */
ListEntry *FetchEntry(LinkedList *list,unsigned long pos) 
{
	ListEntry *entry;
	if(pos == 1 ) return ShiftEntry(list);
	else if(pos == list->length) return PopEntry(list);
	entry = PickEntry(list,pos);
	if(RemoveEntry(list,pos) == 0) return entry;
	return NULL;
}

/*
ListEntry *SelectEntry(LinkedList *list,unsigned long pos) 
{
}
*/

int MoveEntry(LinkedList *list,unsigned long srcPos,unsigned long dstPos) 
{
	ListEntry *e;
	
	e = FetchEntry(list,srcPos);
	if(e)
	{
		if(InsertEntry(list,e,dstPos) == 0)
			return 0;
		else 
		{
			if(!InsertEntry(list,e,srcPos) != 0)
			{
#ifdef DEBUG
				printf("(linklist.c) Can't restore entry at index %lu while moving to %lu\n",srcPos,dstPos);
#endif
			}
		}
	}
	/* TODO - Unimplemented */
	return -1;
}

/* XXX - still dangerous ... */
int SwapEntries(LinkedList *list,unsigned long pos1,unsigned long pos2) 
{
	ListEntry *e1;
	ListEntry *e2;
	if(pos2 > pos1)
	{
		e2 = FetchEntry(list,pos2);
		InsertEntry(list,e2,pos1);
		e1 = FetchEntry(list,pos1+1);
		InsertEntry(list,e1,pos2);
	}
	else if(pos1 > pos2)
	{
		e1 = FetchEntry(list,pos1);
		InsertEntry(list,e1,pos2);
		e2 = FetchEntry(list,pos2+1);
		InsertEntry(list,e2,pos1);
	}
	else
		return -1;
	
	/* TODO - Unimplemented */
	return 0;
}

/* return old entry at pos */
ListEntry *SubstEntry(LinkedList *list,unsigned long pos,ListEntry *entry)
{
	ListEntry *old;
	old = FetchEntry(list,pos);
	if(!old)
		return NULL;
	InsertEntry(list,entry,pos);
	/* XXX - NO CHECK ON INSERTION */
	return old;
}

ListEntry *RemoveEntry(LinkedList *list,unsigned long pos) 
{
	ListEntry *next,*prev;
	ListEntry *entry = PickEntry(list,pos);
#ifdef THREADED
	pthread_mutex_lock(&list->lock);
#endif
	if(entry) 
	{
		prev = entry->prev;
		next = entry->next;
		if(prev) 
			prev->next = next;
		if(next)
			next->prev = prev;
		list->length--;
		entry->list = NULL;
#ifdef THREADED
		pthread_mutex_unlock(&list->lock);
#endif
		return entry;
	}
#ifdef THREADED
	pthread_mutex_unlock(&list->lock);
#endif
	return NULL;
}

/* return position of entry if linked in a list.
 * Scans entire list so it can be slow for very long lists */
unsigned long GetEntryPosition(ListEntry *entry) 
{
	int i = 0;
	LinkedList *list;
	ListEntry *p;
	list = entry->list;
	if(list) 
	{
		p  = list->head;
		while(p) 
		{
			i++;
			if(p == entry) return i;
			p = p->next;
		}
	}
	return 0;
}

void *PopValue(LinkedList *list) 
{
	void *val = NULL;
	ListEntry *entry = PopEntry(list);
	if(entry) 
	{
		val = GetEntryValue(entry);
		DestroyEntry(entry);
	}
	return val;
}

int PushValue(LinkedList *list,void *val) 
{
	int res;
	ListEntry *newEntry = CreateEntry();
	if(!newEntry) return -1;
	SetEntryValue(newEntry,val);
	res = PushEntry(list,newEntry);
	if(!res) DestroyEntry(newEntry);
	return res;
}

int UnshiftValue(LinkedList *list,void *val) 
{
	int res;
	ListEntry *newEntry = CreateEntry();
	if(!newEntry) return -1;
	SetEntryValue(newEntry,val);
	res = UnshiftEntry(list,newEntry);
	if(!res) DestroyEntry(newEntry);
	return res;
}

void *ShiftValue(LinkedList *list) 
{
	void *val = NULL;
	ListEntry *entry = ShiftEntry(list);
	if(entry) 
	{
		val = GetEntryValue(entry);
		DestroyEntry(entry);
	}
	return val;
}

int InsertValue(LinkedList *list,void *val,unsigned long pos) 
{
	int res;
	ListEntry *newEntry = CreateEntry();
	if(!newEntry) return -1;
	SetEntryValue(newEntry,val);
	res=InsertEntry(list,newEntry,pos);
	if(!res) DestroyEntry(newEntry);
	return  res;
}

void *PickValue(LinkedList *list,unsigned long pos) 
{
	ListEntry *entry = PickEntry(list,pos);
	if(entry) return GetEntryValue(entry);
	return NULL;
}

void *FetchValue(LinkedList *list,unsigned long pos) 
{
	void *val = NULL;
	ListEntry *entry = FetchEntry(list,pos);
	if(entry) 
	{
		val = GetEntryValue(entry);
		DestroyEntry(entry);
	}
	return val;
}

/* just an accessor to MoveEntry */
int MoveValue(LinkedList *list,unsigned long srcPos,unsigned long dstPos)
{
	return MoveEntry(list,srcPos,dstPos);
}

/* return old value at pos */
void *SubstValue(LinkedList *list,unsigned long pos,void *newVal)
{
	ListEntry *newEntry;
	ListEntry *oldEntry;
	void *oldVal;
	newEntry = CreateEntry();
	if(newEntry)
	{
		SetEntryValue(newEntry,newVal);
		oldEntry = SubstEntry(list,pos,newEntry);
		if(oldEntry)
		{
			oldVal = GetEntryValue(oldEntry);
			DestroyEntry(oldEntry);
			return oldVal;
		}
	}
	return NULL;
}

int SwapValues(LinkedList *list,unsigned long pos1,unsigned long pos2)
{
	return SwapEntries(list,pos1,pos2);
}

void ForEachListValue(LinkedList *list,void (*itemHandler)(void *item,unsigned long idx,void *user),void *user)
{
	unsigned long i;
	unsigned long len = ListLength(list);
	/* TODO - maybe should lock list while iterating? */
	for(i=1;i<=len;i++)
		itemHandler(PickValue(list,i),i,user);
}

/* 
 * Allocates resources for a new TaggedValue initializing both tag and value
 * to what received as argument.
 * if vLen is 0 or negative, then val is assumed to be a string and 
 * strdup is used to copy it.
 * Return a pointer to the new allocated TaggedValue.
 */
TaggedValue *CreateTaggedValue(char *tag,void *val,unsigned long vLen) 
{
	TaggedValue *newVal = (TaggedValue *)calloc(1,sizeof(TaggedValue));
	if(!newVal) return NULL;
	if(tag) newVal->tag = strdup(tag);
	if(val) 
	{
		if(vLen) 
		{
			newVal->value = malloc(vLen+1);
			if(newVal->value) 
			{
				memcpy(newVal->value,val,vLen);
				memset((char *)newVal->value+vLen,0,1);
				newVal->vLen = vLen;
			}
		} 
		else 
		{
			newVal->value = (void *)strdup((char *)val);
			newVal->vLen = (unsigned long)strlen((char *)val);
		}
	}
#ifdef DEBUG
		printf("(linklist.c) Created TaggedValue at address 0x%p \n",newVal);
#endif
	return newVal;
}

/* Release resources for TaggedValue pointed by tVal */
void DestroyTaggedValue(TaggedValue *tVal) 
{
	if(tVal) 
	{
		if(tVal->tag) free(tVal->tag);
		if(tVal->value) free(tVal->value);
		free(tVal);
	}
	#ifdef DEBUG
		printf("(linklist.c) Destroyed TaggedValue at address 0x%p \n",tVal);
	#endif
}

/* Pops a TaggedValue from the list pointed by list */
TaggedValue *PopTaggedValue(LinkedList *list) 
{
	return (TaggedValue *)PopValue(list);
}

/* 
 * Pushes a new TaggedValue into list. user must give a valid TaggedValue pointer 
 * created trough a call to CreateTaggedValue() routine 
 */
int PushTaggedValue(LinkedList *list,TaggedValue *tVal) 
{
	ListEntry *newEntry;
	int res = 0;
	if(tVal) 
	{
		newEntry = CreateEntry();
		if(newEntry) 
		{
			newEntry->tagged = 1;
			newEntry->value = tVal;
			res = PushEntry(list,newEntry);
			if(!res) DestroyEntry(newEntry);
		}
	}
	return res;
}

int UnshiftTaggedValue(LinkedList *list,TaggedValue *tVal) 
{
	int res = 0;
	ListEntry *newEntry;
	if(tVal) 
	{
		newEntry = CreateEntry();
		if(newEntry)
		 {
			newEntry->tagged = 1;
			newEntry->value = tVal;
			res = UnshiftEntry(list,newEntry);
			if(!res) DestroyEntry(newEntry);
		}
	}
	return res;
}
 
TaggedValue *ShiftTaggedValue(LinkedList *list) 
{
	return (TaggedValue *)ShiftValue(list);
}

int InsertTaggedValue(LinkedList *list,TaggedValue *tVal,unsigned long pos) 
{
	int res = 0;
	ListEntry *newEntry;
	if(tVal) 
	{
		newEntry = CreateEntry();
		if(newEntry) 
		{
			newEntry->tagged = 1;
			newEntry->value = tVal;
			res = InsertEntry(list,newEntry,pos);
			if(!res) DestroyEntry(newEntry);
		}
	}
	return res;
}

TaggedValue *PickTaggedValue(LinkedList *list,unsigned long pos) 
{
	return (TaggedValue *)PickValue(list,pos);
}

TaggedValue *FetchTaggedValue(LinkedList *list,unsigned long pos) 
{
	return (TaggedValue *)FetchValue(list,pos);
}

/* 
 * ... without removing it from the list
 */
TaggedValue *GetTaggedValue(LinkedList *list,char *tag) 
{
	int i;
	TaggedValue *tVal;
	for(i = 1;i <= (int)ListLength(list); i++)
	{
		tVal = PickTaggedValue(list,i);
		if(strcmp(tVal->tag,tag) == 0)
			return tVal;
	}
	return NULL;
}

/* 
 * ... without removing it from the list
 * USER MUST NOT FREE MEMORY FOR RETURNED VALUES
 * User MUST create a new list, pass it as 'values'
 * and destroy it when no more needed .... entries 
 * returned inside the 'values' list MUST not be freed, 
 * because they reference directly the real entries inside 'list'.
 */
unsigned long GetTaggedValues(LinkedList *list,char *tag, LinkedList *values) 
{
	int i;
	int ret;
	TaggedValue *tVal;
	ret = 0;
	for(i = 1;i <= (int)ListLength(list); i++)
	{
		tVal = PickTaggedValue(list,i);
		if(strcmp(tVal->tag,tag) == 0)
		{
			PushValue(values,tVal->value);
			ret++;
		}
	}
	return ret;
}

