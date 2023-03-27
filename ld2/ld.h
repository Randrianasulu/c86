/*
 *		l d _ h
 *
 * (c) Copyright 1991-1999 David J. Walker
 *	   Email:  dave.walker@icl.com
 *
 *	   This is the header file for the LD linker
 *
 *	   Permission to copy and/or distribute granted under the
 *	   following conditions
 *
 *	   1). This notice must remain intact.
 *	   2). The author is not responsible for the consequences of use
 *		   this software, no matter how awful, even if they arise
 *		   from defects in it.
 *	   3). Altered versions must not be represented as being the
 *		   original software.
 */

/*==================================================================
						DEVELOPMENT HISTORY
						~~~~~~~~~~~~~~~~~~~
Dec 89				QDOS port done by Jeremy Allison
20/07/91  1.12	DJW Program version message not output if listing to
					file unless verbose flag set
20/07/92  1.13	DJW Changed -bufp and -bufl to assume any value less
					than 1000 is a size in K (used to be less than 250)
02/12/92  1.14	DJW Caused more error messages to give the FILE and
					position on which a problem occurred
10/01/93  1.15	DJW Suppressed progress messages to screen unless the
					-v flag is specified.
01/06/93  1.16	DJW Minor alterations to compile OK with new ANSI
					version of c68
10/03/94  1.17	DJW Removed some '' keywords that were applied
					to variables used with & operator.	This had been
					missed by earlier releases of C68.
06/11/94  1.18	DJW Added new error message for table size exceeded,
					and changed source accordingly.
16/02/96  1.19	DJW If a library file is not found on any of the library
					search paths, check current directory as well.
24/03/96  1.20	DJW Incorporated Jonathan Hudson's XTC68 changes.
10/05/98  1.21	DJW Changed minimum dataspace to be 100 rather than 50
28/05/98  1.22	DJW Changed directory to be 'LIB' rather than 'lib'

01/05/99  1.30	DJW Reworked to use LIBLIST library
					Many constants changed to symbolic names
					Fixed problem with same section being present mor
					than once in same module, and part ending on an
					odd boundary (was causing NULL byte to be added).

17/05/99  1.50	DJW Reworked to remove all fixed constraints and use
					dynamic memory allocation instead.

20/05/99  2.00	DJW Added ability to write a RLL library.
					Added ability to link in RLL libraries.
===================================================================*/


#ifndef LD_H
#define LD_H

#ifndef INIT
#define INIT(param)
#define EXTERN extern
#else
#define EXTERN
#endif

#include "ldconfig.h"

#include <sys/types.h>
#include <sroff.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <liblist.h>

#ifdef XTC68
# ifdef DOS_LIKE
#  include <io.h>
#  include <malloc.h>
#  define _version ldversion
#  define movmem(a,b,c) memcpy(b,a,c);
# endif /* DOS_LIKE */
#endif	/* XTC68 */

#ifdef WIN32
#  include <io.h>
#endif /* WIN32 */

EXTERN	char _prog_name[]	INIT("ld");
EXTERN	char _version[] 	INIT("v2.00 (beta 2)");
EXTERN	char _copyright[]	INIT("QDOS 68000 SROFF Linker");


#ifdef __linux__
#include <unistd.h>
#include <sys/stat.h>
#define O_BINARY 0
#define movmem(a,b,c) memcpy(b,a,c);
#define stricmp strcasecmp
#define strnicmp strncasecmp
#endif /* __linux__ */

#ifdef VMS
#define movmem(a,b,c) memcpy(b,a,c);
extern char * strdup(char *);
#endif /* VMS */

#ifdef QDOS
#include <unistd.h>
#include <qdos.h>
#endif /* QDOS */

#if defined HAS_LIBDEBUG
#include <libdebug.h>
#else
#define DBG(params)
#define DBG_ENTER(params)
#define DBG_EXIT(params)
#define DBG_INIT()
#endif /* LIBDEBUG */

#ifndef PRIVATE
#define PRIVATE static
#endif
#ifndef PUBLIC
#define PUBLIC extern
#endif

#ifndef BOOL
#define BOOL char
#endif

#ifndef TRUE
#define TRUE (!0)
#endif

#ifndef FALSE
#define FALSE  0
#endif

typedef unsigned char UCHAR;
typedef unsigned short USHORT;
typedef unsigned long ULONG;

#define FMSIZE	   64
#define BLEN	 1024

#define MIN_DATASPACE	100


/*
 *	 Special SECTION ID's
 */
#define SECTION_TEXT   -1
#define SECTION_DATA   -9900
#define SECTION_BSS    -9940
#define SECTION_XDEF   -9950
#define SECTION_RLIB   -9960
#define SECTION_XREF   -9970
#define SECTION_RELOC  -9980
#define SECTION_UDATA  -9990
#define SECTION_DUMMY  -9999

/*
 *	 Special SECTION Name's
 */
#define SECTION_TEXT_TEXT	"TEXT"
#define SECTION_DATA_TEXT	"DATA"
#define SECTION_BSS_TEXT	"BSS"
#define SECTION_XDEF_TEXT	"XDEF"
#define SECTION_RLIB_TEXT	"RLIB"
#define SECTION_XREF_TEXT	"XREF"
#define SECTION_RELOC_TEXT	"RELOC"
#define SECTION_UDATA_TEXT	"UDATA"

/*
 *	Special routines for RLL libraries
 */
#define RLL_LOADINIT		"__RLL_LoadInit"
#define RLL_LINKINIT		"__RLL_LinkInit"

#define sgn(x)	((x)<0?-1:((x)==0?0:1))

/*
 *	Directives that can be found in object file.
 *	These are independent of object format
 */
typedef enum o_direct
	{
	DIRECTIVE_DATA, 			/* Standard data */
	DIRECTIVE_SOURCE,			/* Start a new module */
	DIRECTIVE_COMMENT,			/* Comment follows */
	DIRECTIVE_ORG,				/* Reset the current base address */
	DIRECTIVE_SECTION,			/* Switch to a new section */
	DIRECTIVE_OFFSET,			/* Define an offset */
	DIRECTIVE_XDEF, 			/* Define an externally visible definition */
	DIRECTIVE_XREF, 			/* Defaine an external reference */
	DIRECTIVE_DEFINE,			/* Associate a local ID and a name */
	DIRECTIVE_COMMON,			/* Define a coomon area */
	DIRECTIVE_END,				/* End of a module reached */
	DIRECTIVE_EOF				/* End of a file reached */
	}	O_DIRECT;

/*
 *	Sequential List of files/names
 */

typedef struct
	{
	char *	name;
	short	type;
	} NAMELIST;
/*
 *	The following give values for the type field
 */
#define OBJECT_FILE 	0x01	/* Must be included */
#define LIBSROFF_FILE	0x02	/* Only include referenced modules */
#define LIBRLL_FILE 	0x04	/* Only include symbol information */

#define OPTIONAL_FILE	0x08	/* No error if not found */
#define LIBRARY_PATH	0x10	/* Search library path for file */
#define LIBRARY_STARTUP 0x20	/* Startup file flag */

#define MAX_DATA_BYTES	4

/*
 *	Operator to be applied to an XDEF relocation
 */
typedef struct
   {
	  short id;
	  char	op;
   } OPER;


/*
 *	Program symbol
 *
 *	An SROFF symbol is actually analysed into this structure
 *	be the SROFF specific code so that the remainder of the
 *	linker code can be relatively independent of the
 *	detailed structure of SROFF.
 */

typedef struct
   {
	  short 	length; 			/* Length for variable length */
	  O_DIRECT	directive;			/* Directive type */
	  char		string[81]; 		/* String or data */
	  char		trunc_rule;
	  long		longword;
	  short 	id;
	  list_t	xref_oper;			/* List of OPER nodes */
   } SYMBOL;


/*
 *	This is the structures in which parts of
 *	the generated code are stored.	A section
 *	consists of a linked list of these.
 */
typedef struct
	{     
		long	part_start; 			 /* start address in section of fragment */
		long	part_size;				 /* total size allocated to fragment */
		long	part_used;				 /* amount used in fragment */
		char *	part_code;				 /* pointer to start of fragment */
		char *	part_module;			 /* module that this fragment belongs to */
	}  SECPART;

/*
 *	Section definition
 *	(typcially TEXT, DATA or BSS)
 */

typedef struct
   {
	  short 	 sec_id;
	  BOOL		 sec_output;	/* Set if section should be output (if not zero length) */
	  BOOL		 sec_reuse; 	/* Set if section re-used as UDATA space */
	  long		 sec_start; 	/* Start of section in program (when set) */
	  long		 sec_length;	/* Total length of section */
	  long		 sec_oldlen;	/* Length before starting this module */
	  list_t	 sec_xref;		/* List of XREF nodes for this section */
	  long		 sec_offset;	/* Current offset into this section */
	  char *	 sec_name;		/* Section name */
	  list_t	 sec_parts; 	/* List of SECPART nodes making a section */
	  SECPART *  sec_part;		/* Current part that we are in */
   } SECTION;

/*
 *	External (XDEF) symbol
 */
typedef struct
   {
	  SECTION * 	 xsy_sect;				/* Section in which symbol name defined */
	  char *		 xsy_mod;				/* Module name where symbol defined */
	  long			 xsy_offset;			/* Offset within a section */
	  long			 xsy_value; 			/* Value of symbol (calculated) */
	  UCHAR 		 xsy_rlib;				/* RLL library containing symbol */
	  UCHAR 		 xsy_defd;				/* Flags */
	  char *		 xsy_name;				/* Symbol name */
	  list_t		 xsy_rllxref;			/* RLL_XOPER entries for this symbol */
   } XSYMBOL;

/*
 *	Values for the 'xsy_defd' field
 */
#define XSYMBOL_UNDEFINED	 0		/* Symbol referenced but not (yet) defined */
#define XSYMBOL_DEFINED 	 1		/* Symbol is defined */
#define XSYMBOL_STATIC		 2		/* Symbol is in static linked part */
#define XSYMBOL_RLL 		 4		/* Symbol is in a RLL */
#define XSYMBOL_USED		 8		/* Symbol referenced */
#define XSYMBOL_OPTIONAL	 16 	/* Symbol is "optional" */

typedef struct
   {
	  char	   xop_oper;
	  char	   xop_optyp;
	  union
	  {
		 SECTION * xop_sec;
		 XSYMBOL * xop_sym;
	  } xop_ptr;
   } XOPER;
/*
 *	Values for the 'xop_optype' field
 */
#define OPTYPE_SECTION		-1
#define OPTYPE_PCOUNTER 	0
#define OPTYPE_SYMBOL		1

/*
 *	XREF symbol
 */

typedef struct {
	  long		xref_pos;			/* Position within section */
	  long		xref_abs;			/* Absolute value */
	  long		xref_progpos;		/* Position within program */
	  short 	xref_trunc; 		/* Truncation rule */
	  char *	xref_module;		/* Module where this XREF was defined */
	  list_t	xref_oper;			/* List of XOPER nodes */
   } XREF;

#define SYMBOL_UNDEF	0
#define SYMBOL_XDEF 	1
#define SYMBOL_XREF 	2
#define SYMBOL_RLL		4


typedef struct
	{
	  XREF *		rll_xref;		/* Relevant XREF entry */
	  SECTION * 	rll_sec;		/* Relevant SECTION entry */
	  char			rll_op; 		/* Operation +/- */
	} RLL_XOPER;


/*
 *	Structure used with some parameter options
 */
struct OPTLIST {
		char *	Text;					/* Parameter string to change */
		BOOL *	pFlag;					/* Place to store change */
		BOOL	Value;					/* Value to store */
		short	ReuseId;				/* ID of Section that can be reused */
		};

extern	struct OPTLIST foptions[];
extern	struct OPTLIST moptions[];
extern	struct OPTLIST zoptions[];

/*
 *	The following defines a RLL that
 *	has been linked in to this binary.
 */

typedef struct
	{
	char *	filename;
	char	number;
	char	autoload;
	char *	thingname;
	char *	version;
	}	RLL_ENTRY;



/*======================================================================
 *
 *		D A T A   areas
 *
 *=====================================================================*/

EXTERN	list_t	 linkfiles	   INIT(NULL);	   /* List of files to link */
EXTERN	list_t	 libpaths	   INIT(NULL);	   /* List of library pathnames */
EXTERN	list_t	 rlib_list	   INIT(NULL);	   /* List of RLL libraries needed */

EXTERN	SYMBOL	 sy;
EXTERN	char *	 module_name;					/* name if current module */

EXTERN	char **  pdef_name		INIT(NULL); 	/* table for storing +ve module id's */
EXTERN	char **  ndef_name		INIT(NULL); 	/* table for stroing -ve module id's */
EXTERN	size_t	 pdef_size		INIT(0);		/* Current size of pdef table */
EXTERN	size_t	 ndef_size		INIT(0);		/* Current size of ndef table */
#define PDEF_INCR 100				/* Increment size for pdef_name table */
#define NDEF_INCR	3				/* Increment size for ndef_name table */

EXTERN	char	 lstng_name[FMSIZE];
EXTERN	char *	 prgm_name		INIT(DEFAULT_PROGNAME);  /* Output filename */
EXTERN	BOOL	 defs_flag		INIT(TRUE);  /* Set if all symbols must be defined */
EXTERN	BOOL	 bss_flag		INIT(TRUE);  /* BSS type required. Default = 0 */
EXTERN	BOOL	 xdef_flag		INIT(FALSE);   /* set if XDEF area to be output */
EXTERN	BOOL	 rlib_flag		INIT(FALSE);   /* set if RLIB area to be output */
EXTERN	BOOL	 xref_flag		INIT(FALSE);   /* Set if XREF area to be output */
EXTERN	BOOL	 reloc_flag 	INIT(FALSE);   /* Set if RELOC area to be kept */
EXTERN	BOOL	 udata_flag 	INIT(FALSE);   /* Set if UDATA area to be output */
EXTERN	BOOL	 case_flag		INIT(TRUE);  /* Case significant. Default = TRUE */

EXTERN	BOOL	 rll_flag		INIT(0);	/* Set if RLL libraries being linked */
#define FORMAT_GST			0
#define FORMAT_LD1			1
#define FORMAT_LD2			2
#define FORMAT_RLL			3
EXTERN	BOOL	 format_flag	INIT(FORMAT_LD2);  /* Output format mode */
EXTERN	short	 udata_reuse_id INIT(SECTION_BSS); /* Reuse from here as UDATA */
EXTERN	BOOL	 lstng_flag 	INIT(FALSE);   /* listing flag: default=FALSE */
EXTERN	BOOL	 map_addr_flag	INIT(FALSE);   /* Map of symbols in address order */
EXTERN	BOOL	 map_alpha_flag INIT(FALSE);   /* Map of symbols in name order */
EXTERN	BOOL	 map_lib_flag	INIT(FALSE);   /* List Libraries used */
EXTERN	BOOL	 map_mod_flag	INIT(FALSE);   /* List Modules used */
EXTERN	BOOL	 map_sum_flag	INIT(FALSE);   /* Link summary */
EXTERN	BOOL	 map_xref_flag	INIT(FALSE);   /* Map of symbol cross-reference */
EXTERN	BOOL	 map_unref_flag INIT(FALSE);
EXTERN	BOOL	 name_flag		INIT(FALSE);   /* Program name needs to be output */
EXTERN	BOOL	 prgm_flag		INIT(TRUE);    /* Output required */
EXTERN	BOOL	 verbose_flag	INIT(FALSE);

EXTERN	char *	 rll_name		INIT(NULL);
EXTERN	char *	 rll_version	INIT(NULL);
EXTERN	BOOL	 rll_reentrant	INIT(FALSE);

#ifdef GST_COMPATIBILITY
EXTERN	BOOL	 control_flag	INIT(FALSE);	/* GST control file used. Default = FALSE */
EXTERN	BOOL	 debug_flag 	INIT(FALSE);	/* Debug symbol listing. Default = FALSE */
EXTERN	BOOL	 gst_flag		INIT(FALSE);	/* GST compatible mode: Default=FALSE */
EXTERN	BOOL	 spar_flag		INIT(FALSE;
#endif /* GST_COMPATIBILITY */

EXTERN	UCHAR *  module_buffer; 				/* Address of current module buffer */
EXTERN	size_t	 module_end 	INIT(0);		/* End of data in module buffer */
EXTERN	size_t	 module_ptr 	INIT(0);		/* Current point in module buffer */
EXTERN	size_t	 module_top 	INIT(0);		/* Current size of module buffer */

EXTERN	SECTION *  curr_sec 	INIT(NULL);
EXTERN	SECTION *  text_sec 	INIT(NULL);
EXTERN	SECTION *  data_sec 	INIT(NULL);
EXTERN	SECTION *  bss_sec		INIT(NULL);
EXTERN	SECTION *  rlib_sec 	INIT(NULL);
EXTERN	SECTION *  xdef_sec 	INIT(NULL);
EXTERN	SECTION *  xref_sec 	INIT(NULL);
EXTERN	SECTION *  reloc_sec	INIT(NULL);
EXTERN	SECTION *  udata_sec	INIT(NULL);
EXTERN	SECTION *  reuse_sec	INIT(NULL);

EXTERN	list_t	  mod_liste 	INIT(NULL); 	   /* list of NAMELIST nodes */
EXTERN	char *	  curr_mod		INIT(NULL); 	   /* Current module name */

EXTERN	list_t	 xsy_rll		INIT(NULL); 	/* List of RLL symbols */

EXTERN	list_t	 xref_master;
EXTERN	list_t	 xoper_master;
EXTERN	list_t	 part_master;
EXTERN	list_t	 rll_xoper_master;

EXTERN	int 	 double_sym 	INIT(0);		/* Count of multiply defined symbols */
EXTERN	int 	 undefd_sym 	INIT(0);		/* Count of undefined symbols */
EXTERN	int 	 range_err		INIT(0);
EXTERN	size_t	 symb_size		INIT(0);
EXTERN	size_t	 unsatisfied	INIT(0);

EXTERN	FILE  *  inp_fp;		/* Current input file being processed */
EXTERN	FILE  *  outp_fp;		/* Output (binary program) file */
EXTERN	FILE  *  list_file; 	/* Listing file */

EXTERN	char currentname[50];

#define sgn(x)	((x)<0?-1:((x)==0?0:1))

EXTERN	unsigned char  inp_buf[BLEN];
EXTERN	unsigned char  *buf_ptr;
EXTERN	unsigned char  *buf_end;
EXTERN	int 		   inp_hnd			INIT(-1);
EXTERN	int 		   progfile_handle	INIT(-1);
EXTERN	long		   progfile_size	INIT(0);
/*
 *	These definitions allow us to simply handle
 *	handle case dependency when required
 */

#define MSG(number,text)  number,
enum message_numbers
	{
#include "ldmsgs.h"
	};
#undef MSG

#include "ldproto.h"


#endif /* LD_H */
