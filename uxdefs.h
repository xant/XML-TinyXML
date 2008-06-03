#ifndef __uxdefs_h__
#define __uxdefs_h__

/// missing unix stuff on win32 (at least...to be able to build this little our stuff)
#ifdef WIN32
/*
 *	strings
 *
 */
#define snprintf _snprintf
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#define strdup _strdup

/*
 *	files
 *
 */
#define stat _stat

/*
 *	time
 *
 */
#define sleep(_duration) (Sleep(_duration * 1000))

#endif // #ifdef WIN32
#endif // #ifndef __uxdefs_h__
