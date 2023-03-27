/*
 *		l d c o n f i g _ h
 *
 * (c) Copyright 1991-1999 David J. Walker
 *	   Email:  dave.walker@icl.com
 *
 *	   This is the configuration header file for the LD linker
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
 */

#ifndef LDCONFIG_H
#define LDCONFIG_H
/*
 *	Parameter Compatibility Mode
 *	~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *	These two options determine what sort of compatibility you want for
 *	providing the parameters to LD.   If you set neither option then
 *	UNIX compatibility is assumed.	 If both options are set, then the
 *	version of LD generated will first try UNIX compatibility mode, and
 *	if that fails it will try GST compatibility mode.
 *
 *	N.B.  The GST_COMPATIBILITY option is currently only experimental
 */
#define UNIX_COMPATIBILITY	1	/* UNIX style parameter options */
/*#define GST_COMPATIBLITY	1	/* GST style parameter options */

/*
 *	Run-time Link Library Support
 *	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *	This option should be set if the code for Runtime Dynamic Link Libraries
 *	is to be included.
 *
 *	N.B.  This options is still expreimental
 */
#define RLL_SUPPORT 1		/* Include code for RLL support */

/*
 *	Relocation Table Format
 *	~~~~~~~~~~~~~~~~~~~~~~~
 *	These two options determine what sort of relocation table format
 *	you want.	The C68_RELOC_FORMAT is the default one.  It is not
 *	permissible to specify both options.
 *
 *	N.B.  The GST_RELOC_FORMAT is currently only experimental
 */
#define C68_RELOC_FORMAT 1		/*The C68 relocation table format */
/*#define GST_RELOC_FORMAT 1	/* The original GST relocation format */

/*
 *	Default Names
 *	~~~~~~~~~~~~~
 *	The following are default names that will be used if no
 *	alternative is supplied by the user as a runtime option.
 */
#define DEFAULT_STARTUP "crt.o" 		/* Default standard startup module */
#define DEFAULT_RLLSTART "crtrll.o" 	/* Default RLL startup module */
#define DEFAULT_LIBRARY "libc.a"		/* Default C library */
#define DEFAULT_RLL 	"libc.rll"		/* Default RLL library name */
#define DEFAULT_RLS 	"libc.rls"		/* Default RLS stub library name */
#define DEFAULT_PROGNAME "a.out"		/* Default name for program file */

/*
 *	Buffer Sizes
 *	~~~~~~~~~~~~
 *	There are two internal buffers in LD.
 *	One is used to hold the target program, and the other the current
 *	library module being worked on.  These buffers are allocated initial
 *	sizes, and then expanded if necessary.	However, expanding these
 *	buffers can lead to memory fragmentation, so we want to avoid doing
 *	it more than necessary.  Note that the default initial sizes can be
 *	over-ridden by command line options.
 */
#define BUFP_DEFAULT	32L * 1024L 	/* Default for program buffer */
#define BUFL_DEFAULT	8L * 1024L		/* Default for library module buffer */
#define BUFP_INCREMENT	16L * 1024L 	/* Increment for program buffer */
#define BUFL_INCREMENT	2L * 1024L		/* Increment for library module buffer */

/*
 *	Library routines
 *	~~~~~~~~~~~~~~~~
 *	The following are set to determine whether standard library routines
 *	are available, or whether the ones supplied with LD need to be used.
 *		0 (or undefined) LD supplied routines to be used
 *		1				 Library versions to be used
 *
 *	The following settings are available:
 *		HAS_GETOPT			The getopt() routine is in libraries
 *		HAS_STRICMP 		The stricmp() routine is in libraries
 *		HAS_STRNICMP		The strnicmp() routine is in libraries
 *		HAS_STRUPR			The strupr() routine is in libraries
 *		HAS_STRDUP			The strdup() routine is in libraries
 *		HAS_STRFND			he strfnd() routine is in libraries
 *		HAS_STRMFE			The strmfe() routine is in the libraries
 */

#if defined QDOS
#define HAS_LIBDEBUG 1
#define HAS_GETOPT 1
#define HAS_STRICMP 1
#define HAS_STRNICMP 1
#define HAS_STRUPR 1
#define HAS_STRDUP 1
#define HAS_STRFND 1
#define HAS_STRMFE 1
#endif /* QDOS */

#ifdef XTC68
#define HAS_STRFND 0
#endif /* XTC68 */

#ifdef __linux__
#define HAS_LIBDEBUG 0
#define HAS_GETOPT 1
#define HAS_STRICMP 1
#define HAS_STRNICMP 1
#define HAS_STRUPR 1
#define HAS_STRDUP 1
#define HAS_STRMFE 0
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif /* __linux__ */

#ifdef WIN32
#define HAS_LIBDEBUG 1
#define HAS_GETOPT 0
#define HAS_STRICMP 0
#define HAS_STRNICMP 0
#define HAS_STRUPR 0
#define HAS_STRDUP 0
#define HAS_STRMFE 0
#define MAXNAMELEN 128
#endif /* WIN32 */

 /*----------------------------------------------------------------------------
 *		Nothing beyond this point is user configureable
 *---------------------------------------------------------------------------*/

#ifdef QDOS
# define DSEP '_' 
# define DSEPS "_"
# define EXT '_'
# define EXTS "_"
#endif /* QDOS */

#ifdef XTC68
# define EXT '.'
# define EXTS "."
#ifdef __unix__
# define DSEP '/'
# define DSEPS "/"
# define DEFAULT_LIBPATH	"/usr/local/qllib/"
#else
# define DSEP '\\'
# define DSEPS "\\"
# define DEFAULT_LIBPATH	"c:\\qllib\\"
#endif /* __unix__ */
#endif /* XTC68 */


#ifndef GST_COMPATIBILITY
#ifndef UNIX_COMPATIBILITY
#define UNIX_COMPATIBILITY 1
#endif
#endif


#ifndef GST_RELOC_FORMAT
#ifndef C68_RELOC_FORMAT
#define C68_RELOC_FORMAT 1
#endif
#else
#ifdef C68_RELOC_FORMAT
#error	You can only define one of C68_RELOC_FORMAT and GST_RELOC_FORMAT
#endif
#endif

#define SYMTABLE_INC   100		/* Increment size for extending SYMBOL table */
#define SECTABLE_INC   5		/* Increment size for extending SECTION table */

#undef _P_

#endif /* LDCONFIG_H */
