/*
 *		  l d u t i l s _ c
 *
 *	This module contains utility routines used within the
 *	LD linker program.
 *
 *	In particular, it contains routines that are in some C
 *	libraries but not others as they are not mandated by
 *	ANSI. Whether they are actually used will be determined
 *	by the settings in the LDCONFIG.H configuration file.
 *
 *	(c) Copyright 1991-1999 David J. Walker
 *		Email:	dave.walker@icl.com
 *
 *	Permission to copy and/or distribute granted under the
 *	following conditions:
 *		1). This notice must remain intact.
 *		2). The author is not responsible for the consequences of use
 *			this software, no matter how awful, even if they arise from
 *			defects in it.
 *		3). Altered version must not be represented as being the
 *			original software.
 *		4). The author retains all rights regarding the use of this
 *			software for use as part of any commercially sold package.
 */


#include "ld.h"
#include <ctype.h>
#include <stdlib.h>



#define MSG(number,text) text,
static char   * message_texts[] =  {
#include "ldmsgs.h"
			};
#undef MSG

/*========================================================== MESSAGE_TEXT */
static
char *	Message_Text (int n)
/*		~~~~~~~~~~~~
 *	This routine will return the address of the error message
 *	corresponding to the supplied error code.	If the error
 *	code is outside the expected range, then a pointer is
 *	returned that points to a generated message quoting the
 *	error number.
 *------------------------------------------------------------------------*/
{
	static	char buf[80];

	DBG(("MESSAGE_TEXT", 0x41, "Enter: n=%d", n));
	if (n < 1 || n > sizeof(message_texts)/sizeof(char *)) 
	{
		(void) sprintf (buf,message_texts[ERR_UNKNOWN],n);
		DBG(("MESSAGE_TEXT",0x41, "Exit: '%s'", buf));
		return (buf);
	}
	DBG(("MESSAGE_TEXT",0x41, "Exit: '%s'", message_texts[n]));
	return message_texts[n];
}


/*=============================================================== MESSAGE */
void	Message (int n, ...)
/*		~~~~~~~
 *	This is the generalised routine for outputting error messages.
 *	It takes as a parameter the number coresponding to the required
 *	text, plus optional extra parameters in printf style.
 *-------------------------------------------------------------------------*/
{
	va_list ap;

	DBG(("MESSAGE", 0x41, "Enter: n=%d", n));
	va_start(ap,n);
	(void) vfprintf (stderr, Message_Text(n), ap);
	(void) fprintf (stderr,"\n");
	va_end(ap);
	DBG(("MESSAGE", 0x41, "Exit"));
	return;
}

/*===================================================================== HALT */
void	Halt(int n,...)
/*		~~~~
 *	This routine is called whenever a fatal error is encountered.
 *---------------------------------------------------------------------------*/
{
	va_list ap;

	DBG(("HALT", 0x11, "Enter: n=%d", n));
	va_start(ap,n);
	(void) fprintf(stderr,"%s: Error:- ",_prog_name);
	(void) vfprintf (stderr, Message_Text(n), ap);
	(void) fprintf (stderr,"\n");
	va_end(ap);
	exit(-1);
}


/*==================================================== NAMESPACE_USE */
PUBLIC
char *	NameSpace_Use (char * name)
/*		~~~~~~~~~~~~~
 *	This is a simple wrapper around the LIST_NameSpace_Use
 *	library routine that adds a check on failure so that it
 *	does not have to be repeated each time in the main code.
 *-------------------------------------------------------------------*/
{
	char * reply;

	reply = LIST_NameSpace_Use (name);
	if (reply == NULL)
	{
		Halt (ERR_NOMEMORY);
	}
	return reply;
}


/*================================================ NAMELIST_INITNODE */
PRIVATE
int 	NameList_InitNode (node_t node, va_list ap)
/*		~~~~~~~~~~~~~~~~
 *	Routine called when a NAMELIST node is created
 *-------------------------------------------------------------------*/
{
	char * name;
	int    type;

	name = va_arg (ap, char *);
	type = va_arg (ap, int);

#if 0
	DBG(("NAMELIST_INITNODE", 0x101, "Enter: node at $%p, name='%s', type=%d", node, name, type));
#endif

	((NAMELIST *)node)->name = NameSpace_Use (name);
	((NAMELIST *)node)->type = type;

#if 0
	DBG(("NAMELIST_INITNODE", 0x101, "Exit LIST_ERROR_NONE: name=$%p('%s'), type=$%p", \
										((NAMELIST *)node)->name, \
										((NAMELIST *)node)->name, \
										((NAMELIST *)node)->type));
#endif
	return LIST_ERROR_NONE;
}


/*================================================ NAMELIST_KILLNODE */
PRIVATE
int 	NameList_KillNode (node_t node)
/*		~~~~~~~~~~~~~~~~~
 *	Routine called when a NAMELIST node is destroyed
 *-------------------------------------------------------------------*/
{
#if 0
	DBG(("NAMELIST_KILLNODE", 0x1001, "Enter: node at $%p", node));
#endif

	(void)LIST_NameSpace_Free (&((NAMELIST *)node)->name);

#if 0
	DBG(("NAMELIST_KILLNODE", 0x1001, "Exit LIST_ERROR_NONE"));
#endif

	return LIST_ERROR_NONE;
}


/*================================================= NAMELIST_APPEND */
PUBLIC
NAMELIST *	NameList_Append (list_t * plist, char * name, int flags)
/*			~~~~~~~~~~~~~~~
 *	Append an entry to a list of NAMELIST nodes.
 *	If the list is not yet intialised, then create it.
 *	If an out of memory error occurs, stop the program.
 *------------------------------------------------------------------*/
{
	NAMELIST *	reply;

	DBG(("NAMELIST_APPEND", 0x11, "Enter: plist=$%p($%p), name='%s', flags=%d",\
						plist, *plist, name,flags));

	if (*plist == NULL)
	{
		DBG(("NAMELIST_APPEND", 0x01, "Create new node"));
		*plist	= LIST_Create (LIST_CLASS_SINGLE,
							   sizeof(NAMELIST),
							   NameList_InitNode,
							   NameList_KillNode,
							   NULL);
		if (*plist == NULL)
		{
			Halt (ERR_NOMEMORY);
			return NULL;
		}
	}
	reply = LIST_NewAppend (*plist, name, flags);
	if (reply == NULL)
	{
		Halt (ERR_NOMEMORY);
		return NULL;
	}
	DBG(("NAMELIST_APPEND", 0x11, "Exit: reply=$%p (name=%s')", reply, reply->name));
	return reply;
}



/*============================================================= READ_B */
int 	read_b(void)
/*		~~~~~~
 *	Get a character from a buffer or from disk.
 *	Implements simple buffering to improve performance.
 *---------------------------------------------------------------------*/
{
#if 0
	if (buf_end - inp_buf < BLEN)
   {
	  return (EOF);
   }
#endif
   buf_end = inp_buf + read(inp_hnd,inp_buf,BLEN);
   buf_ptr = inp_buf;
   return(buf_ptr < buf_end
			? (int)*buf_ptr++
			: EOF);
}


/*=========================================================== GET_DRCT */
int 	get_drct (void)
/*		~~~~~~~~
 *---------------------------------------------------------------------*/
{
   return buf_ptr < buf_end
			? (int)*buf_ptr++
			: read_b();
}


/*========================================================== GET_BYTE */
int 	get_byte (void)
/*		~~~~~~~~
 *	This routine will get data either from a cashed
 *	module or from the current input file when there
 *	is no cashed input file.
 *--------------------------------------------------------------------*/
{
   return module_ptr < module_end
			? (int)module_buffer[module_ptr++]
			: get_drct();
}

/*========================================================== GET_LONG */
long	get_long (void)
/*		~~~~~~~~
 *	Note.	Cannot put the get_byte() calls into
 *			the return statement as the C standard
 *			does not guarantee the evaluation order
 *			of the expression (recently got bitten
 *			by the latest version of the Metrowerks
 *			compiler on the Mac!).
 *--------------------------------------------------------------------*/
{
	register unsigned int  c1, c2, c3, c4;

	c1 = get_byte();
	c2 = get_byte();
	c3 = get_byte();
	c4 = get_byte();
	return (long)((c1 << 24) + (c2 << 16) + (c3 << 8) + c4);
}

/*========================================================== GET_SHORT */
short	get_short (void)
/*		~~~~~~~~~
 *	Note.	Cannot put the get_byte() calls into
 *			the return statement as the C standard
 *			does not guarantee the evaluation order
 *			of the expression (recently got bitten
 *			by the latest version of the Metrowerks
 *			compiler on the Mac!).
 *---------------------------------------------------------------------*/
{
	register unsigned int c1, c2;

	c1 = get_byte();
	c2 = get_byte();
	return (short)((c1<<8) + c2);
}


/*============================================================ WRITE_DATA */
void	Write_Data (void * buffer, size_t length)
/*		~~~~~~~~~~
 *	Routine to write data to the program file
 *	(assuming that we are writing a program file).
 *	Any errors cause an appropriate error message
 *	to be output and the program to fail.
 *------------------------------------------------------------------------*/
{
	DBG(("WRITE_DATA",0x1001,"Enter: buffer=$%p, length=%d",buffer,length));
	if (prgm_flag)
	{
		if (fwrite(buffer,length,1,outp_fp) != 1)
		{
#if 0
			Write_Error();
#endif 
			Halt (ERR_WRITEFILE);
		}
	}
	DBG(("WRITE_DATA",0x1001,"Exit"));
}



#if  (! HAS_GETOPT)
static int getopt_err  (char *, char *, int );
int 	opterr = 1; 	/* error => print message */
int 	optind = 1; 	/* next argv[] index */
char *	optarg = NULL;	/* option parameter if any */

/*============================================================ GETOPT */
int 	getopt( argc, argv, optstring )
/*		~~~~~~
 *	Routine to help with parsing a command line
  returns letter, '?', EOF
 *---------------------------------------------------------------------*/
 int  argc; 			/* argument count from main */
 char *argv[];			/* argument vector from main */
 char *optstring;		/* allowed args, e.g. "ab:c" */
{
	static int sp = 1;	/* position within argument */
	int osp;			/* saved 'sp' for param test */
	int c;				/* option letter */
	char *cp;			/* -> option in 'optstring' */

	optarg = NULL;

	if ( sp == 1 )			/* fresh argument */
	{
		if ( optind >= argc 	/* no more arguments */
		|| argv[optind][0] != '-'	/* no more options */
		|| argv[optind][1] == '\0'	/* not option; stdin */
		 )
		{
			return EOF;
		}
		else
		{
			if ( strcmp( argv[optind], "--" ) == 0 )
			{
				++optind;	/* skip over "--" */
				return EOF; /* "--" marks end of options */
			}
		}
	}

	c = argv[optind][sp];	/* option letter */
	osp = sp++; 			/* get ready for next letter */

	if ( argv[optind][sp] == '\0' ) {
		/* end of argument */
		++optind;		/* get ready for next try */
		sp = 1; 		/* beginning of next argument */
	}

	if ( c == ':'			/* optstring syntax conflict */
	  || (cp = strchr( optstring, c )) == NULL	/* not found */
	   )
		return getopt_err( argv[0], "illegal option", c );

	if ( cp[1] == ':' ) {
		/* option takes parameter */
		if ( optind >= argc )
			return getopt_err( argv[0],"option requires an argument",c);

#if (STRICT != 0)
		if ( osp != 1 )
			return Err( argv[0],"option must not be clustered",c);

		if ( sp != 1 )		/* reset by end of argument */
			return getopt_err( argv[0],"option must be followed by white space",c);
		optarg = (char *)argv[optind];	/* make parameter available */
#else
		optarg = (char *)argv[optind] + (sp-(sp==1));  /* make parameter available */
#endif /* STRICT */
		sp = 1;
		++optind;						/* skip over parameter */
	}

	return c;
}

/*
 *	Local function that optionally prints a message, and returns '?'
 */
static int getopt_err( name, mess, c ) /* returns '?' */
char *name; 	/* program name argv[0] */
char *mess; 	/* specific message */
int c;			/* defective option letter */
{
	if ( opterr )
		(void) fprintf( stderr, "%s: %s -- %c\n", name, mess, c );

	return '?'; 		/* erroneous-option marker */
}
#endif /* ! HAS_GETOPT */


#if (! HAS_STRICMP)
/*============================================================ STRICMP */
int 	stricmp (scan1, scan2)
/*		~~~~~~~
 *	Case independent version of strcmp()
 *---------------------------------------------------------------------*/
 const char * scan1;
 const char * scan2;
{
	char c1, c2;

	do {
		c1 = (char)tolower(*scan1);
		c2 = (char)tolower(*scan2);
		scan1++; scan2++;
	} while (c1 && (c1 == c2));

	/*
	 * The following case analysis is necessary so that characters
	 * which look negative collate low against normal characters but
	 * high against the end-of-string NULL.
	 */
	if (c1 == c2)
		return(0);
	else if (c1 == '\0')
		return(-1);
	else if (c2 == '\0')
		return(1);
/*
	else
*/
		return(c1 - c2);
}
#endif	/* HAS_STRICMP */


#if  (! HAS_STRNICMP)
/*============================================================ STRNICMP */
int 	strnicmp (str1, str2, len)
/*		~~~~~~~~
 *	Case independent version of strncmp()
 *---------------------------------------------------------------------*/
 const char *str1;
 const char *str2;
 size_t len;
{
	unsigned char c1, c2;
	long limit = len;

	do {
		c1 = (char)tolower(*str1);
		c2 = (char)tolower(*str2);
		str1++; str2++;
	} while((c1 == c2) && --limit);

	if (limit < 0) {
		return(0);
	}
	return(c1 - c2);
}
#endif	/* HAS_STRNICMP */


#if  (! HAS_STRUPR)
/*=========================================================== STRUPR */
char * strupr (string)
/*	   ~~~~~~
 * Convert a string to upper case
 *-------------------------------------------------------------------*/
 char * string;
{
	char *ptr;

	for (ptr = string ; *ptr ; ptr++) {
		*ptr = toupper (*ptr);
	}
	return (string);
}
#endif /* HAS_STRUPR */


#if  (! HAS_STRDUP)
/*============================================================== STRDUP */
char *	strdup (s)
/*		~~~~~~
 *	Duplicate a string in local string space
 *----------------------------------------------------------------------*/
const char * s;
{
	char *dup;

	if ((dup = (char *)malloc(strlen(s)+1)) != NULL) {
		(void) strcpy(dup, s);
	}
	return dup;
}
#endif /* HAS_STRDUP */


#if (! HAS_STRFND)
/*================================================================ STRFND */
PUBLIC
int 	strfnd (const char *tofind,
				const char *tosearch,
				int   casematch)
/*
 *	Find a string.	In this simple implementation we ignore
 *	the flag that provides case independent matching
 *------------------------------------------------------------------------*/
{
	char *p;

	(void) casematch;		/* Dummy to suppress Compiler warnings */
	p = strstr(tosearch, tofind);
	if(p == NULL)
	{
		return -1;
	}
	else
	{
		return (p - tosearch);
	}
}

#endif /* ! HAS_STRFND */

#if (! HAS_STRMFE)
/*================================================================ STRMFE */
void	 strmfe (char * out,
				 const char *in,
				 const char *ext)
/*
 *------------------------------------------------------------------------*/
{
	int i = strfnd( ext, in, 0);
	int extlen = strlen(ext);
	int inlen = strlen(in);

	(void) strcpy( out, in);
	if(i == -1 || (i + extlen != inlen)) {
		(void) strcat( out, ext);
	}

	return;
}
#endif /* HAS_STRMFE */


#ifdef GST_COMPATIBILITY
/*============================================================ MAKE_EXT */
void	make_ext (char * make_name,
				  char * name,
				  char * ext)
/*
 *----------------------------------------------------------------------*/
{
	char  oldtext[FMSIZE];

	stcgfe(oldtext,name);
	if (! *oldtext)
	{
		strmfe(make_name,name,ext);
	}
	else
	{
		(void) strcpy(make_name,name);
	}
}
#endif /* GST_COMPATIBILITY */
