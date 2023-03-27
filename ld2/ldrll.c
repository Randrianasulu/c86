/*
 *	   l d r l l _ c
 *
 *	   This module is the one that contains all routines that are
 *	   directly aware of the QDOS RLL object and binary formats.
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
#include <librll.h>
#include <assert.h>

#ifdef RLL_SUPPORT
/*========================================================== RLL_HEADER */
PUBLIC
void	RLL_Header (void)
/*		~~~~~~~~~~
 *	Output a RLL header.
 *
 *	Most of this has preset values.
 *----------------------------------------------------------------------*/
{
	int x;

	DBG_ENTER (("RLL_HEADER", 0x11, ""));

	Section_Start();

	curr_sec = Section_Check(SECTION_TEXT_TEXT);

	DBG(("RLL_HEADER", 0x18, "THING header space"));

	for (x = 0 ; x < 19 ; x++)
	{
		Section_Add_Short (curr_sec, 0);
	}
	DBG(("RLL_HEADER", 0x18, "RLL Version number"));
	Section_Add_Code (curr_sec,
					  NULL == rll_version
						? "1.00"
						: rll_version,
					  4);
	DBG(("RLL_HEADER", 0x18, "THING name"));
	x = strlen (rll_name);
	Section_Add_Short (curr_sec, x);
	Section_Add_Code (curr_sec, rll_name, x);
	if (x & 1)
	{
		DBG(("RLL_HEADER", 0x18, "Add extra byte to make RLL name even length"));
		Section_Add_Byte (curr_sec, 0);
	}
	DBG(("RLL_HEADER", 0x18, "Default THING"));
	Section_Add_Code (curr_sec, "THG%", 4);
	Section_Add_Long (curr_sec, 0);

	DBG(("RLL_HEADER", 0x18, "RLL tag"));
	Section_Add_Code (curr_sec, "<<RLL>>", 8);

	/*--------------------------
	 *	Pointers to BSS_Header,
	 *	INIT and LINK routines.
	 *-------------------------*/
	DBG(("RLL_HEADER", 0x18, "Pointer to BSS area, INIT and LINK routines"));
	for (x = 0 ; x < 3 ; x++)
	{
		Section_Add_Long (curr_sec, 0);
	}
	/*-----------------
	 *	Special flags
	 *----------------*/
	DBG(("RLL_HEADER", 0x18, "Flag bytes"));
	Section_Add_Byte(curr_sec, rll_reentrant);
	Section_Add_Byte(curr_sec, 0);

	/*-------------------------------
	 *	Set up work fields that are
	 *	required for use by RLM
	 *------------------------------*/

	DBG(("RLL_HEADER", 0x18, "RLM work fields"));
	for (x = 0 ; x < 4 ; x++)
	{
		Section_Add_Long(curr_sec, 0);
	}
	Section_End();

	DBG_EXIT (("RLL_HEADER", 0x11, ""));
	return;
}


/*==================================================== RLL_HEADER_INIT */
PRIVATE
void	RLL_Header_Init (char * name, long pos)
/*		~~~~~~~~~~~~~~~
 *	Patch one of the initialisation offsets in
 *	the RLL_Header if the symbol exists.
 *---------------------------------------------------------------------*/
{
	XSYMBOL * xsy;

	xsy = Symbol_Find (name);
	DBG(("RLL_HEADER_FIX", 0x18, "Symbol %s = $%p", name, xsy));
	if (NULL != xsy
	&&	(XSYMBOL_DEFINED | XSYMBOL_STATIC) == (xsy->xsy_defd & (XSYMBOL_DEFINED | XSYMBOL_STATIC)))
	{
		DBG(("RLL_HEADER_FIX", 0x18, "Set %s() Offset to $%p", \
									name, Symbol_Calc(xsy)));
		xsy->xsy_defd |= XSYMBOL_USED;
		text_sec->sec_offset = pos;
		Section_Add_Long (text_sec, Symbol_Calc (xsy));
	}
	return;
}


/*====================================================== RLL_HEADER_FIX */
PUBLIC
void	RLL_Header_Fix (void)
/*		~~~~~~~~~~~~~~
 *	Called at the end of the link to add update some
 *	fields in the header with values determined from
 *	the link process.
 *----------------------------------------------------------------------*/
{
	int 	hdrlen;

	DBG_ENTER (("RLL_HEADER_FIX", 0x11, ""));

	/*-----------------------
	 *	Offset to BSS Header
	 *-----------------------*/
	hdrlen = (offsetof (THING_LINKAGE, th_name_text) + 1 + strlen (rll_name)) & (unsigned long)~1;
	DBG(("RLL_HEADER_FIX", 0x18, "Size of THING_LINKAGE = %ld", \
										hdrlen));
	DBG(("RLL_HEADER_FIX", 0x18, "offset of BSS_Header = %ld", \
										offsetof (RLL_HEADER, rll_bss)));

	DBG(("RLL_HEADER_FIX", 0x18, "Set BSS Offset to $%p", \
							bss_sec->sec_start));
	text_sec->sec_offset = hdrlen + offsetof(RLL_HEADER, rll_bss);
	Section_Add_Long (text_sec, bss_sec->sec_start);

	/*-----------------------
	 *	INIT routine offsets
	 *----------------------*/
	RLL_Header_Init (RLL_LOADINIT, hdrlen + offsetof(RLL_HEADER, rll_loadinit));
	RLL_Header_Init (RLL_LINKINIT, hdrlen + offsetof(RLL_HEADER, rll_linkinit));

	DBG_EXIT (("RLL_HEADER_FIX", 0x11, ""));
	return;
}
#endif /* RLL_SUPPORT */


/*========================================================== XDEF_ENTRY */
PRIVATE
long	XDEF_Entry	(node_t node, va_list ap)
/*		~~~~~~~~~~
 *	Create a single entry in the BSS XDEF area
 *----------------------------------------------------------------------*/
#define xsy ((XSYMBOL *)node)
{
	int 	length;

	(void) ap;

	DBG_ENTER(("XDEF_ENTRY",0x8001, "name='%s'",xsy->xsy_name));

	/*------------------------------------
	 *	Check that this symbol is defined
	 *	in this module.
	 *-----------------------------------*/
	if (XSYMBOL_UNDEFINED == xsy->xsy_defd)
	{
		DBG_EXIT(("XDEF_ENTRY", 0x8001, "Not created - symbol undefined"));
		return 0;
	}
	/*--------------------------------------
	 *	Output an entry for this definition
	 *--------------------------------------*/
	length = strlen (xsy->xsy_name);
	Section_Add_Byte (xdef_sec, length);
	Section_Add_Long (xdef_sec, Symbol_Calc(xsy));
	Section_Add_Code (xdef_sec, xsy->xsy_name, length);
	DBG_EXIT(("XDEF_ENTRY", 0x8001, ""));
	return 0;
}
#undef xdef


/*=========================================================== XDEF_AREA */
PRIVATE
void	XDEF_Area (void)
/*		~~~~~~~~~
 *----------------------------------------------------------------------*/
{
	DBG_ENTER (("XDEF_AREA", 0x11, ""));

	if (0 == xdef_flag)
	{
		DBG_EXIT  (("XDEF_AREA", 0x11, "Not created - XDEF not wanted"));
		return;
	}
	/*------------------------
	 *	Set up the XDEF area
	 *-----------------------*/
	if ( 0 == Symbol_Count())
	{
		DBG_EXIT  (("XDEF_AREA", 0x11, "Not created - XDEF list empty"));
		return;
	}
	/*--------------------
	 *	Create the entry
	 *-------------------*/
	xdef_sec = Section_Check (SECTION_XDEF_TEXT);
	Section_Add_Code (xdef_sec, BSS_XDEFFLAG, sizeof (BSS_XDEFFLAG) - 1);
	(void)Symbol_Enumerate(XDEF_Entry);
	Section_Add_Long (xdef_sec, 0);

	DBG_EXIT  (("XDEF_AREA", 0x11, ""));
	return;
}


#ifdef RLL_SUPPORT
/*========================================================== RLIB_ENTRY */
PRIVATE
long	RLIB_Entry	(node_t node, va_list ap)
/*		~~~~~~~~~~
 *	Create single entry in the BSS RLIB area.
 *----------------------------------------------------------------------*/
#define rll  ((RLL_ENTRY *)node)
{
	(void) ap;

	DBG_ENTER(("RLIB_ENTRY", 0x8001, "filename='%s', thingname='%s', version='%s', autoload=%d, number=%d", \
												rll->filename, \
												rll->thingname, \
												rll->version, \
												rll->autoload, \
												rll->number));
	Section_Add_Byte (rlib_sec, rll->number + (rll->autoload << 7));
	Section_Add_Code (rlib_sec, rll->version, 4);
	Section_Add_Code (rlib_sec, rll->thingname, strlen(rll->thingname)+1);
	DBG_EXIT(("RLIB_ENTRY", 0x8001, ""));
	return 0;
}
#undef rll


/*=========================================================== RLIB_AREA */
PRIVATE
void	RLIB_Area (void)
/*		~~~~~~~~~
 *	IF RLL Libraries were used, then set up
 *	a RLIB area to specify them.
 *----------------------------------------------------------------------*/
{
	int 	count;

	DBG_ENTER (("RLIB_AREA", 0x11, ""));

	/*---------------------------
	 *	Check first to see if we
	 *	need a RLIB AREA.
	 *--------------------------*/
	if (NULL == rlib_list)
	{
		DBG_EXIT(("RLIB_AREA", 0x11, "RLIB LIST not set up"));
		return;
	}
	count = LIST_Count(rlib_list);
	if (0 == count)
	{
		DBG_EXIT(("RLIB_AREA", 0x11, "RLIB LIST empty"));
		return;
	}
	/*----------------------------
	 *	Now set up the RLIB area
	 *---------------------------*/
	rlib_sec = Section_Check (SECTION_RLIB_TEXT);
	Section_Add_Code (rlib_sec, BSS_RLIBFLAG, sizeof (BSS_RLIBFLAG)-1);
	(void)LIST_Enumerate (rlib_list, RLIB_Entry);
	Section_Add_Byte (rlib_sec, 0);

	DBG_EXIT  (("RLIB_AREA", 0x11, ""));
	return;
}

long	XrefEntryOpAddress;

/*======================================================= XREF_ENTRY_OP */
PRIVATE
long	XREF_Entry_Op (node_t node, va_list ap)
/*		~~~~~~~~~~~~~
 *	Add a particular 'op' entry to the XREF area
 *----------------------------------------------------------------------*/
#define rll_xoper ((RLL_XOPER *)node)
{
	UCHAR	flagbyte;
	long	address;

	(void)ap;
	DBG_ENTER(("XREF_ENTRY_OP", 0x801, ""));

	address = rll_xoper->rll_xref->xref_pos + rll_xoper->rll_sec->sec_start;

	/*------------------------------------------
	 *	See if entry is out of range as a
	 *	displacement - if so add new absolute
	 *	entry to set new address.
	 *-----------------------------------------*/
	if ((address - XrefEntryOpAddress) > 0x7fff)
	{
		Section_Add_Byte (xref_sec, 0xf0 + ((address >> 16) & 0x0f));
		Section_Add_Short (xref_sec, address & 0xffff);
		XrefEntryOpAddress = address;
	}
	/*--------------------------------
	 *	Write entry for this address
	 *-------------------------------*/
	flagbyte = rll_xoper->rll_xref->xref_trunc & 0x1f;
	if ('-' == rll_xoper->rll_op)
	{
	   flagbyte |= 0x80;
	}
	Section_Add_Byte (xref_sec, flagbyte);
	Section_Add_Short (xref_sec, XrefEntryOpAddress - address);
	XrefEntryOpAddress = address;

	DBG_EXIT (("XREF_ENTRY_OP", 0x801, ""));
	return 0;
}
#undef rll_xoper


/*========================================================== XREF_ENTRY */
PRIVATE
long	XREF_Entry (node_t node, va_list ap)
/*		~~~~~~~~~~
 *	Examine a particular symbol, and if needed
 *	create an entry in the XREF area for this
 *	symbol.
 *----------------------------------------------------------------------*/
#define xsy ((XSYMBOL *)node)
{
	(void)ap;
	DBG_ENTER(("XREF_ENTRY", 0x81, "Symbol %s", xsy->xsy_name));


	/*----------------------------------
	 *	Any statically linked symbols
	 *	are not required in XREF area.
	 *---------------------------------*/
	if (0 != (xsy->xsy_defd & XSYMBOL_STATIC))
	{
		DBG_EXIT(("XREF_ENTRY", 0x81, "Not needed - statically defined"));
		return 0;
	}
	/*----------------------------------------
	 *	Undefined symbols are only required
	 *	if flag set to allow this.
	 *---------------------------------------*/
	if (0 == (xsy->xsy_defd & XSYMBOL_DEFINED)
	&&	0 == defs_flag)
	{
		DBG_EXIT(("XREF_ENTRY", 0x81, "Not needed - undefined"));
		return 0;
	}
	else
	{
		xsy->xsy_defd |= XSYMBOL_RLL;
	}
	if (xsy->xsy_defd & XSYMBOL_RLL)
	{
#if 0
		Section_Add_Short(xref_sec, LIST_Count(xsy->xsy_rllxref));
#endif
		Section_Add_Byte (xref_sec, xsy->xsy_rlib);
#if 0
		Section_Add_Byte (xref_sec, strlen(xsy->xsy_name + 1));
#endif
		Section_Add_Code (xref_sec, xsy->xsy_name, strlen(xsy->xsy_name) + 1);
		XrefEntryOpAddress = 0;
		(void) LIST_Enumerate (xsy->xsy_rllxref, XREF_Entry_Op);
	}
	DBG_EXIT(("XREF_ENTRY", 0x81, ""));
	return 0;
}
#undef xsy


/*=========================================================== XREF_AREA */
PRIVATE
void	XREF_Area (void)
/*		~~~~~~~~~
 *----------------------------------------------------------------------*/
{
	DBG_ENTER (("XREF_AREA", 0x11, ""));

	if (0 == xref_flag)
	{
		DBG_EXIT(("XREF_AREA",0x11,"XREF area not required"));
		return;
	}
	xref_sec = Section_Check (SECTION_XREF_TEXT);
	Section_Add_Code (xref_sec, BSS_XREF_FLAG, sizeof(BSS_XREF_FLAG)-1);

	(void) Symbol_Enumerate (XREF_Entry);

	Section_Add_Short (xref_sec, BSS_XREF_END_ENTRY);
	DBG_EXIT  (("XREF_AREA", 0x11, ""));
	return;
}

#endif /* RLL_SUPPORT */



/*========================================================= RELOC_XREF */
PRIVATE
long	Reloc_Xref (node_t node, va_list ap)
/*		~~~~~~~~~~
 *
 *	Take a single XREF within a section and
 *	create a relocation entry if needed.
 *
 *	It is written as a call-back function designed to be used
 *	by the LIST_Enumerate method from the LIBLIST library.
 *---------------------------------------------------------------------*/
{
#define x ((XREF * )node)

	static long altxref = 0;		/* previous relocation offset in program */

	SECTION * s;

	s = va_arg(ap, SECTION *);

	DBG_ENTER(("RELOC_XREF", 0x408, "Section %s+$%p: XREF at %ld($%p): xref_abs=%ld($%p)",\
										s->sec_name, \
										x->xref_pos,
										x->xref_pos + s->sec_start, \
										x->xref_pos + s->sec_start, \
										x->xref_abs, x->xref_abs));

	/*--------------------------------------------
	 *	Add entries to the Relocation stream if
	 *	this entry needs relocating at runtime.
	 *-------------------------------------------*/
	if (x->xref_trunc & TRUNC_RELOC)
	{
		long  target;

		target = x->xref_pos + s->sec_start;	   /* Position in program */

		assert (target >= 0);			/* Check target is positive */
		assert (bss_sec != NULL);		/* Check we have pointer to BSS section */

		if (0 == altxref)
		{
			/*-------------------------------
			 *	If this is the first entry
			 *	then set the initial offset.
			 *-------------------------------*/
			altxref = target;
			DBG(("RELOC_XREF", 0x2008, "Initial offset set to %ld($%p)", \
											   altxref, altxref));
			Section_Add_Long (reloc_sec, target);
		}
		else
		{
			/*------------------------------------------
			 *	If displacement to next relocation
			 *	greater than 254 output reset entries.
			 *-----------------------------------------*/
			DBG(("RELOC_XREF", 0x2008, "Write relocation entry for program offset %ld($%p)", \
													target, target));
			while ( (target - altxref) > 254)
			{

				altxref += 254;
				DBG(("RELOC_XREF", 0x2008, "Increasing offset by 254 to %ld($%p)", \
												altxref, altxref));
				Section_Add_Byte(reloc_sec, '\001');
			}
			/*---------------------------------------
			 *	Finally output relocation required.
			 *--------------------------------------*/
			DBG(("RELOC_XREF", 0x2008, "Relocation entry at %ld($%p) = %ld($%p) relative to previous)", \
											target, target, \
											target - altxref, target - altxref));
			Section_Add_Byte(reloc_sec, (char)(target - altxref));
			altxref = target;
		}
	}
	DBG_EXIT(("RELOC_XREF", 0x11, "Section=%s", \
										 s->sec_name));
	return 0;
#undef x
}


/*=================================================== RELOC_SECTION */
PRIVATE
void	Reloc_Section (SECTION * sec)
/*		~~~~~~~~~~~~~
 *
 *	Generate the relocation stream for the given section.
 *----------------------------------------------------------------------*/
{
	DBG_ENTER(("RELOC_SECTION", 0x11, "Section=%s", sec->sec_name));

	/*-----------------------------------
	 *	For convenience, reset section
	 *	pointers to first part.
	 *---------------------------------*/
	sec->sec_part = LIST_First (sec->sec_parts);
	(void) LIST_Enumerate (sec->sec_xref,
						   Reloc_Xref,
						   sec);

	DBG_EXIT(("RELOC_SECTION", 0x11, "Section=%s", sec->sec_name));
	return;
}



/*========================================================== RELOC_AREA */
PRIVATE
void	RELOC_Area (void)
/*		~~~~~~~~~~
 *----------------------------------------------------------------------*/
{
	SECTION * sec;
	DBG_ENTER (("RELOC_AREA", 0x11, ""));

	/*----------------------------------------=
	 *	Check we have a RELOC section defined
	 *----------------------------------------*/
	reloc_sec = Section_Check (SECTION_RELOC_TEXT);

	/*---------------------------------
	 *	Decide on the header (if any)
	 *	that we need to add to it.
	 *--------------------------------*/
	switch (format_flag)
	{
		case FORMAT_GST:
		case FORMAT_LD1:
					break;

		case FORMAT_LD2:
					Section_Add_Code (reloc_sec,
									  BSS_RELOCFLAG,
									  sizeof (BSS_RELOCFLAG) -1);
					break;
	}

	/*-------------------------
	 *	Now resolve all XREF's
	 *------------------------*/
	Section_SetStarts();

	for (sec=Section_First(); NULL != sec ; sec = Section_Next(sec))
	{
		Reloc_Section(sec);
	}
	/*
	 *	Terminate the current relocation stream
	 */
	if (FORMAT_GST != format_flag)
	{
		Section_Add_Byte(reloc_sec, '\0');
	}
	/*---------------------------------
	 *	Write out long words of zero
	 *	in case no relocation done.
	 *	(probably not really needed?).
	 *--------------------------------*/
	Section_Add_Long(reloc_sec, 0);
	Section_End();

	DBG_EXIT  (("RELOC_AREA", 0x11, ""));
	return;
}




/*=========================================================== BSS_AREA */
PUBLIC
void	BSS_Area (void)
/*		~~~~~~~~
 *----------------------------------------------------------------------*/
{
	DBG_ENTER (("BSS_AREA", 0x11, ""));


	XDEF_Area();
#ifdef RLL_SUPPORT
	RLIB_Area();
	XREF_Area();
#endif /* RLL_SUPPORT */

	Section_End();
	RELOC_Area();
	Section_SetStarts();

	/*------------------------------------
	 *	We always have a BSS section
	 *	- whatever output format we use!
	 *-----------------------------------*/
	switch (format_flag)
	{
		SECTION * sec;

		default:
#ifdef LIBDEBUG
				DBG(("BSS_AREA", 0x18, "Unexpected value for format flag of %d", \
											format_flag));
				break;

		case FORMAT_GST:
		case FORMAT_LD1:
#endif /* lIBDEBUG */
				break;

		case FORMAT_LD2:
		case FORMAT_RLL:

				/*------------------------------
				 *	Initialise the BSS section
				 *-----------------------------*/
				bss_sec->sec_length = sizeof (BSS_HEADER);
				Section_SetStarts();
				Section_Add_Code (bss_sec, BSSFLAG, sizeof (BSSFLAG) - 1);
				/*
				 *	Now overwrite section fields
				 *	with their correct values
				 */
				for (sec = Section_Next(bss_sec) ;
							sec != NULL ;
									sec = Section_Next(sec))
				{
					DBG(("BSS_AREA", 0x18, "Section %s: start=$%p, length=$%p", \
											sec->sec_name, \
											(0 == sec->sec_length ? 0 : sec->sec_start), \
											sec->sec_length));

					Section_Add_Long (bss_sec, 0 == sec->sec_length
												? 0
												: sec->sec_start);
					Section_Add_Long (bss_sec, sec->sec_length);
				}
	} /* end of switch */

	DBG_EXIT  (("BSS_AREA", 0x11, ""));
	return;
}

#if 0
/*============================================================ TEST_RLL */
PRIVATE
int 	Test_RLL (void)
/*		~~~~~~~~
 *	This routine examines a RLL library to see if it is needed
 *----------------------------------------------------------------------*/
{
	int 	result = 0;

	DBG_ENTER(("TEST_RLL",0x21,""));
	DBG_EXIT(("TEST_RLL",0x21,"result=%d", result));
	return (result);
}
#endif


/*============================================================= LINK_RLL */
PUBLIC
void	Link_RLL (node_t filenode)
/*		~~~~~~~~
 *	This routine links in a RLL library if it is needed.
 *-----------------------------------------------------------------------*/
{
	char	buffer[256];
	UCHAR	symlength;
	long	symoffset;
	char	rlib_number;
	RLL_FILE *	rp;
	size_t	UseRll = 0;
	BSS_HEADER * bssheader;

	DBG_ENTER(("LINK_RLL",0x21,""));

	if (NULL == (rp = RLL_OpenRllFile (inp_hnd)))
	{
		Halt (ERR_NOMEMORY);
	}
	bssheader = RLL_BssHeader(rp);

	/*--------------------------------
	 *	Position at XDEF area
	 *	and then read through entries
	 *	setting up use where needed
	 *------------------------------*/
	DBG(("LINK_RLL", 0x18, "Go to XDEF area at %ld", bssheader->bss_xdef));
	(void) lseek (inp_hnd, bssheader->bss_xdef, SEEK_SET);
	DBG(("LINK_RLL", 0x18, "read XDEF area tag"));
	(void) read (inp_hnd, buffer, 8);
	if (strncmp(buffer, BSS_XDEFFLAG, 8))
	{
		Halt (ERR_BINFILE);
	}
	/*----------------------------
	 *	Read XDEF names and check
	 *	if they are needed
	 *---------------------------*/
	DBG(("LINK_RLL", 0x18, "now work through symbols in this RLL"));
	rlib_number = LIST_Count(rlib_list) + 1;
	for ( ;  ; )
	{
		XSYMBOL * xsy;

		DBG(("LINK_RLL", 0x18, "current file position=%ld", \
										lseek (inp_hnd, 0, SEEK_CUR)));
		if (sizeof(symlength) != read (inp_hnd, &symlength, sizeof(symlength))
		||	sizeof(symoffset) != read (inp_hnd, &symoffset, sizeof(symoffset)))
		{
			Halt (ERR_BINFILE);
		}
		DBG(("LINK_RLL", 0x18, "Symbol offset=%ld ($%p), length=%d", \
									symoffset, symoffset, symlength));
		if (0 == symlength)
		{
			DBG(("LINK_RLL", 0x11, "Exit - end of table reached"));
			break;
		}
		(void) read (inp_hnd, buffer, symlength);
		buffer[symlength] = '\0';
		DBG(("LINK_RLL", 0x18, "Next symbol is '%s'", buffer));
		if (NULL != (xsy = Symbol_Find(buffer)))
		{
			if (0 == (xsy->xsy_defd & XSYMBOL_DEFINED))
			{
				DBG(("LINK_RLL", 0x18, "Symbol %s used from this RLL", buffer));
				xsy->xsy_defd |= (XSYMBOL_DEFINED | XSYMBOL_RLL);
				xsy->xsy_rlib = rlib_number;
				UseRll++;
				xref_flag++;
			}
			else
			{
				DBG(("LINK_RLL", 0x18, "Symbol not used - already defined"));
			}
		}
		else
		{
			DBG(("LINK_RLL", 0x18, "Symbol not used - not wanted"));
		}
	}
	DBG(("LINK_RLL", 0x18, "finished checking symbols in this RLL"));

	/*-------------------------------
	 *	If we used any XDEFs then
	 *	add this to the RLL's used.
	 *------------------------------*/
	DBG(("LINK_RLL", 0x18, "%ld symbols used from this RLL", UseRll));
	if (0 != UseRll)
	{
		RLL_ENTRY * rll_entry;

		if (NULL == (rll_entry = LIST_NewNode (rlib_list)))
		{
			Halt (ERR_NOMEMORY);
		}
		rll_entry->number = rlib_number;
		rll_entry->filename =  NameSpace_Use (((NAMELIST *)filenode)->name);
		rll_entry->thingname = NameSpace_Use (rp->rll_thing);
		rll_entry->version = NameSpace_Use (rp->rll_version);
		DBG(("LINK_RLL", 0x18, "filename='%s', thingname='%s', version='%s'", \
									rll_entry->filename, \
									rll_entry->thingname, \
									rll_entry->version));
		(void) LIST_Append (rlib_list, rll_entry);
	}

	DBG_EXIT(("LINK_RLL",0x21,""));
	return;
}
