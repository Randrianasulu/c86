/*		l d p r o t o _ h
 *
 * (c) Copyright 1991-1999 David J. Walker
 *	   Email:  dave.walker@icl.com
 *
 *	   This is the prototype header file for the LD linker
 *
 *	   Permission to copy and/or distribute granted under the
 *	   following conditions:
 *
 *	   1). This notice must remain intact.
 *	   2). The author is not responsible for the consequences of use
 *		   this software, no matter how awful, even if they arise
 *		   from defects in it.
 *	   3). Altered versions must not be represented as being the
 *		   original software.
 *
 *	AMENDMENT HISTORY
 *	~~~~~~~~~~~~~~~~~
 *	January 94	DJW   - First version - based on LD v1.xx code
 *
 *	March	96	DJW   - Changes to incorporate XTC68 support
 */

/*
 *	  'ldlist' module
 */
void	eprintf 			(const char *, ...);
void	lprintf 			(const char *, ...);
void	Init_Listing		(void);
void	List_Module_Details (void);
int 	Listings			(void);


/*
 *	'ldmain' module
 */


/*
 *	'ldparams' module
 */
void Command_Line			(int   argc, char  **argv);

/*
 *	'ldrll' module
 */
void	RLL_Header			(void);
void	RLL_Header_Fix		(void);
void	BSS_Area			(void);
void	Link_RLL			(node_t);

/*
 *	'ldsectn' module
 */
void	  Section_Init		  (void);
void	  Section_Add_Code	  (SECTION *, char *, long);
void	  Section_Add_Long	  (SECTION *, long);
void	  Section_Add_Short   (SECTION *, short);
void	  Section_Add_Byte	  (SECTION *, char);
void	  Section_AllXrefs	  (void);
SECTION * Section_Check 	  (char * name);
SECTION * Section_Define	  (char *);
void	  Section_End		  (void);
SECTION * Section_First 	  (void);
SECTION * Section_Next		  (SECTION *);
void	  Section_SetStarts   (void);
void	  Section_Start 	  (void);
void	  Section_WriteAll	  (void);
/*
 *	'ldsymbol' module
 */
void	  Symbol_Init		  (void);
long	  Symbol_Calc		  (XSYMBOL *);
void	  Symbol_Case		  (int);
XSYMBOL * Symbol_Check		  (char * name);
long	  Symbol_Count		  (void);
int 	  Symbol_AllDefined   (void);
long	  Symbol_Enumerate	  (long (*)(node_t, va_list));
XSYMBOL * Symbol_Find		  (char * name);
/*
 *	'ldutils' module
 */
void	Halt				(int, ...);
void	Message 			(int, ...);
char *	NameSpace_Use		(char *);
NAMELIST * NameList_Append	(list_t * plist, char * name, int flags);
void	Write_Data			(void *, size_t);
int 	read_b				(void);
int 	get_drct			(void);
int 	get_byte			(void);
long	get_long			(void);
short	get_short			(void);

#if (! HAS_GETOPT)
extern int	opterr;
extern int	optind;
extern char * optarg;
int 	getopt			(int, char **, char *);
#endif

#ifndef HAS_STRICMP
int 	stricmp 		(const char *, const char *);
#endif

#ifndef HAS_STRNICMP
int 	strnicmp		(const char *, const char *, size_t);
#endif

#ifndef HAS_STRUPR
char *	strupr			(char *);
#endif

#ifndef HAS_STRDUP
char *	strdup			(const char *);
#endif

#if (! HAS_STRFND)
int 	strfnd			(const char *, const char *, int);
#endif

#if (! HAS_STRMFE)
void	strmfe			(char *, const char *, const char *);
#endif

#ifdef GST_COMPATIBILITY
void	make_ext		(char *, char *, char *);
#endif
