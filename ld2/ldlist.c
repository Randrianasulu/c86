/*
 *		l d l i s t _ c
 *
 *	   This module is the listing control module for the LD program.
 *
 * (c) Copyright 1991-1999 David J. Walker
 *	   Email:  dave.walker@icl.com
 *
 *	   Permission to copy and/or distribute is granted under the
 *	   following conditions:
 *
 *	   1). This notice must remain intact.
 *	   2). The author is not responsible for the consequences of use
 *		   this software, no matter how awful, even if they arise
 *		   from defects in it.
 *	   3). Altered versions must not be represented as being the
 *		   original software.
 */


#include "ld.h"
#include <stdarg.h>


/*============================================================= EPRINTF */
PUBLIC
void	eprintf (const char * formatstr, ...)
/*
 *	This routine is a variant of printf that has the following
 &	characteristics:
 *	  - It will always print to the stderr file
 *	  - If listing is active it will also print to the listing file.
 *----------------------------------------------------------------------*/
{
	va_list ap;

	va_start (ap, formatstr);
	(void)vfprintf (stderr, formatstr, ap);
	if (lstng_flag)
	{
		(void) vfprintf (list_file,formatstr,ap);
	}
	va_end(ap);
	return;
}


/*============================================================= LPRINTF */
PUBLIC
void	lprintf (const char * formatstr, ...)
/*
 *	This routine is a variant of printf that has the following
 &	characteristics:
 *	  - It will do nothing if listing is not active
 *	  - It will print to the listing file rather than stdout.
 *----------------------------------------------------------------------*/
{
	va_list ap;

	va_start (ap, formatstr);
	if (lstng_flag)
	{
		(void) vfprintf (list_file,formatstr,ap);
	}
	va_end(ap);
	return;
}


/*============================================================== INIT_LIST */
void	Init_Listing ()
/*
 *	Do any listing initialisation that is only required once at
 *	the beginning of the link process.
 *-------------------------------------------------------------------------*/
{
	DBG_ENTER(("INIT_LISTING",0x21,""));
	list_file=stdout;
	if (lstng_flag)
	{
		list_file=fopen(lstng_name,"w");
		if (list_file == NULL)	{
			Halt (ERR_OPENLIST, lstng_name);
		}
		if (list_file != stdout)
		{
			(void) fprintf(list_file,"%s %s %s\n",_prog_name,_version, _copyright);
		}
	}
	if (verbose_flag)
		(void)fprintf(stderr,"%s %s %s",_prog_name,_version,_copyright);

	DBG_EXIT(("INIT_LISTING",0x21,""));
	return;
}



/*================================================ LIST_MODULE_DETAILS */
PUBLIC
void	List_Module_Details (void)
/*		~~~~~~~~~~~~~~~~~~~
 *	List the details of the last module that was
 *	added to the modules linked in.
 *----------------------------------------------------------------------*/
{
	int i;
	SECTION * sec;

	DBG_ENTER(("LIST_MODULE_DETAILS", 0x401, ""));

	if (0 == map_mod_flag)
	{
		DBG_EXIT(("LIST_MODULE_DETAILS",0x401,"...not required"));
		return;
	}

	lprintf ("%-12.12s:",module_name);

	for (i=0, sec=Section_First() ; sec != NULL ; sec = Section_Next(sec))
	{
		/*
		 *	We do not want to list the special
		 *	sections unless they are not zero
		 *	length.
		 */
		if (SECTION_BSS < sec->sec_id
		||	SECTION_UDATA == sec->sec_id
		||	0 != sec->sec_length)
		{
			/*
			 *	We can fix a max of 4 entries per line
			 */
			if (i++ > 3)
			{
				lprintf ("\n");
				i=0;
			}
			DBG(("LIST_MODULE_DETAILS", 0x408, " %8.8s=%08lX", \
								  sec->sec_name, \
								  (unsigned long)sec->sec_length - sec->sec_oldlen));
			lprintf(" %8.8s=%08lX",
					  sec->sec_name,
					  (unsigned long)sec->sec_length-sec->sec_oldlen);
		}
	}

	lprintf ("\n");
	DBG_EXIT(("LIST_MODULE_DETAILS", 0x401, ""));
	return;
}


/*============================================================= STATISTICS */
PRIVATE
void	Statistics(void)
/*		~~~~~~~~~
 *		Print out statistics on the link
 *-------------------------------------------------------------------------*/
{
#ifdef QDOS
	extern	long _mtotal;
#endif

	SECTION  *s;

	DBG_ENTER(("STATISTICS",0x21, ""));
	lprintf("\n");
	lprintf("---------------------------\n");
	lprintf("              (hexadecimal)\n");
	lprintf("SECTION      START   LENGTH\n");
	lprintf("---------------------------\n");
	for (s = Section_First() ; s != NULL ; s = Section_Next(s))
	{
		if (0 == s->sec_length)
		{
			DBG(("STATISTICS", 0x28, "Section %s zero length - ignored", \
									s->sec_name));
			continue;
		}
		lprintf("%-9s %8X %8X\n",
				s->sec_name,
				s->sec_start,
				s->sec_length);
	}
	lprintf("---------------------------\n");
	lprintf("Program length   = %8X\n", progfile_size);
	lprintf("Relocation table = %8X\n", reloc_sec->sec_length);
#ifdef QDOS
	lprintf("Memory Usage     = %8ld\n",  _mtotal);
#endif /* QDOS */
	lprintf("---------------------------\n");

	DBG_EXIT(("STATISTICS", 0x21, ""));
	return;
}




/*======================================================== LIST_UNDEFINED */
PRIVATE
long	List_Undefined (node_t node, va_list ap)
/*		~~~~~~~~~~~~~~
 *	This function lists undefined symbols
 *
 *	It is implemented as a call-back function for use
 *	with the LIST_Enumerate() method of the LIBLIST library.
 *------------------------------------------------------------------------*/
{
	(void)ap;	/* Cast to stop compiler warning */

	if (0 == (((XSYMBOL *)node)->xsy_defd & XSYMBOL_DEFINED))
	{
		lprintf ("Undefined Symbol: '%s'\n", ((XSYMBOL *)node)->xsy_name);
		undefd_sym++;
	}
	return 0;
}


/*========================================================= LIST_SYMBOL */
PRIVATE
long	List_Symbol (node_t node, va_list ap)
/*		~~~~~~~~~~~
 *	This function lists a symbol from a symbol list node.
 *
 *	It is implemented as a call-back function for use
 *	with the LIST_Enumerate() method of the LIBLIST library.
 *----------------------------------------------------------------------*/
#define xsy ((XSYMBOL *)node)
{

	(void)ap;	/* Cast to stop compiler warning */

	DBG_ENTER(("LIST_SYMBOLS", 0x1001, "Symbol '%s'", xsy->xsy_name));
	if (0 != (xsy->xsy_defd & XSYMBOL_DEFINED))
	{
		lprintf ( "%-20s %08lX%c",
						 xsy->xsy_name,
						(unsigned long) Symbol_Calc(xsy),
						xsy->xsy_defd & XSYMBOL_USED ? ' ':'*');
		if (NULL != xsy->xsy_sect)
		{
		   lprintf(" %15s", xsy->xsy_sect->sec_name);
		}
		else
		{
			lprintf(" %20s"," ");
		}
		if (NULL != xsy->xsy_mod)
		{
		   lprintf(" %15s", xsy->xsy_mod);
		}
		lprintf("\n");
	}
	else
	{
		lprintf("%-20s undefined\n", xsy->xsy_name);
	}

	DBG_EXIT(("LIST_SYMBOLS", 0x1001, ""));
	return 0L;
}
#undef xsy


/*========================================================= LIST_HEADER */
PRIVATE
void	List_Header (char * order)
/*		~~~~~~~~~~~
 *-----------------------------------------------------------------------*/
{
	lprintf("\nSymbol Table: (%s)\n", order);
	lprintf("-------------\n");
	return;
}

#if 0
/*====================================================== LIST_SYMBOLLIST */
PRIVATE
long	List_SymbolList (node_t node, va_list ap)
/*		~~~~~~~~~~~~~~~
 * Used when a level of indiretion is needed
 *-----------------------------------------------------------------------*/
{
	(void)ap;	/* Cast to stop compiler warning */

	(void)LIST_Enumerate ((list_t)node, List_Symbol);
	lprintf("\n");
	return 0L;
}
#endif

/*=========================================================== LIST_ALPHA */
PRIVATE
void	List_Alpha (void)
/*		~~~~~~~~~~
 *-----------------------------------------------------------------------*/
{
	DBG_ENTER(("LIST_ALPHA",0x11,""));
	if (! map_alpha_flag)
	{
		DBG_EXIT(("LIST_ALPHA", 0x11, "not wanted"));
		return;
	}
	List_Header ("alphabetical");
	(void)Symbol_Enumerate (List_Symbol);
	lprintf("\n");
	DBG_EXIT(("LIST_ALPHA",0x11,""));
	return;
}


/*======================================================== LIST_ADDRESS */
PRIVATE
void	List_Address (void)
/*		~~~~~~~~~~~~
 *	Produce a listing in address order.
 *	Takes advantage of sections being in
 *	ascending address order.
 *----------------------------------------------------------------------*/
{
#if 0
	SECTION * sec;
#endif

	DBG_ENTER(("LIST_ADDRESS",0x11,""));

	if (! map_addr_flag)
	{
		DBG_EXIT(("LIST_ADDRESS", 0x11, "not wanted"));
		return;
	}
#if 0
	List_Header ("address");
	for (sec = Section_First() ; NULL != sec ; sec = Section_Next(sec))
	{
		(void)LIST_Enumerate (sec->sec_xref, List_SymbolList);
	}
#endif
	DBG_EXIT(("LIST_ADDRESS",0x11,""));
	return;
}


/*======================================================== LIST_LIBRARY */
PRIVATE
void	List_Library (void)
/*		~~~~~~~~~~~~
 *----------------------------------------------------------------------*/
{
	DBG_ENTER(("LIST_LIBRARY", 0x11, ""));

	if (! map_lib_flag)
	{
		DBG_EXIT(("LIST_LIBRARY", 0x11, "not wanted"));
		return;
	}

		/**** STILL TO DO ****/

	DBG_EXIT(("LIST_LIBRARY",0x11,""));
	return;
}


/*========================================================== LIST_XREFS */
PRIVATE
void	List_Xrefs (void)
/*		~~~~~~~~~~
 *----------------------------------------------------------------------*/
{
	DBG_ENTER(("LIST_XREFS", 0x11, ""));

	if (!map_xref_flag)
	{
		DBG_EXIT(("LIST_XREFS", 0x11, "not wanted"));
		return;
	}

	/**** STILL TO DO ****/

	DBG_EXIT(("LIST_XREFS",0x11,""));
	return;
}


/*========================================================== LIST_UNREF */
PRIVATE
void	List_Unrefs (void)
/*		~~~~~~~~~~~
 *----------------------------------------------------------------------*/
{
	DBG_ENTER(("LIST_UNREFS", 0x11, ""));

	if (! map_unref_flag)
	{
		DBG_EXIT(("LIST_UNREFS", 0x11, "not wanted"));
		return;
	}

		/**** STILL TO DO ****/

	DBG_EXIT(("LIST_UNREFS",0x11,""));
	return;
}


/*======================================================== LIST_SYMBOLS */
PUBLIC
int 	Listings (void)
/*		~~~~~~~~
 *	Produce symbol listings.
 *	Which ones are produced depends on parameter settings.
 *----------------------------------------------------------------------*/
{
	int err_code=0;

	DBG_ENTER(("LISTINGS",0x11,""));
	Statistics();

	List_Library();
	List_Alpha();
	List_Address();
	List_Xrefs();
	List_Unrefs();

	(void)Symbol_Enumerate (List_Undefined);

	if (undefd_sym)
	{
	  eprintf("Undefined Symbols: %8d\n",undefd_sym);
	  err_code = -1;
	}
	if (double_sym)
	{
		eprintf("Multiply defined : %8d\n",double_sym);
		err_code = -1;
	}
	if (range_err)
	{
		eprintf("Range errors     : %8d\n",range_err);
		err_code = -1;
	}

	/*
	 *	Output a completion message.
	 *	Where it is sent depends on whether
	 *	verbose method is active or not.
	 */
	if (verbose_flag)
	{
		eprintf("\nLink completed\n");
	}
	else
	{
		lprintf("\nLink completed\n");
	}

	if (lstng_flag)
	{
		(void) fclose(list_file);
	}
#ifdef VMS
	err_code = 1;
#endif
	DBG_EXIT(("LISTINGS", 0x11, ""));
	return err_code;

}
