/*
 *	   l d s e c t n _ c
 *
 *	   This module is the one that has SECTION related routines
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
#include <assert.h>

PRIVATE list_t	sec_list = NULL;

static	struct SPECIAL_SECTIONS {
			char * SectionName;
			short  SectionId;
			SECTION ** ppSection;
			BOOL   Output;
			BOOL   Reuse;
			} SpecialSections[] =
			  {
			  {SECTION_TEXT_TEXT,   SECTION_TEXT,  &text_sec, TRUE, FALSE},
			  {SECTION_DATA_TEXT,   SECTION_DATA,  &data_sec, TRUE, FALSE},
			  {SECTION_BSS_TEXT,    SECTION_BSS,   &bss_sec, TRUE, TRUE},
			  {SECTION_XDEF_TEXT,   SECTION_XDEF,  &xdef_sec, TRUE, TRUE},
			  {SECTION_RLIB_TEXT,   SECTION_RLIB,  &rlib_sec, TRUE, TRUE},
			  {SECTION_XREF_TEXT,   SECTION_XREF,  &xref_sec, TRUE, TRUE},
			  {SECTION_RELOC_TEXT,  SECTION_RELOC, &reloc_sec, TRUE, TRUE},
			  {SECTION_UDATA_TEXT,  SECTION_UDATA, &udata_sec, FALSE, TRUE},
			  {NULL, -2, NULL, TRUE, FALSE }     /* Used for all other sections */
			  };



/*======================================================= SECTIONID_COMPNODE */
PRIVATE
int 	SectionID_CompNode (node_t node1, node_t node2)
/*		~~~~~~~~~~~~~~~~~~
 *	Compare two SECTION nodes on their id's
 *	Note that these are negative and we want
 *	them in order of absolute value.
 *---------------------------------------------------------------------------*/
{
	return ((SECTION *)node2)->sec_id - ((SECTION *)node1)->sec_id;
}



/*====================================================== SECTION_INIT */
void	Section_Init (void)
/*		~~~~~~~~~~~~
 *	Initialise all special sections
 *--------------------------------------------------------------------*/
{
	struct SPECIAL_SECTIONS * pSpecialSections;

	DBG_ENTER (("SECTION_INIT", 0x11, ""));
	sec_list = LIST_Create (LIST_CLASS_SINGLE,
							sizeof (SECTION),
							NULL,
							NULL,
							SectionID_CompNode);

	if (NULL == sec_list)
	{
		Halt(ERR_NOMEMORY);
	}

	for (pSpecialSections = SpecialSections;
			NULL != pSpecialSections->SectionName ;
				pSpecialSections++)
	{
		(void) Section_Define (pSpecialSections->SectionName);
	}
	DBG_EXIT (("SECTION_INIT", 0x11, ""));
	return;
}

/*======================================================= SECTION_FIRST */
PUBLIC
SECTION *	Section_First (void)
/*			~~~~~~~~~~~~~
 *----------------------------------------------------------------------*/
{
	return (SECTION *)LIST_First(sec_list);
}

/*======================================================= SECTION_NEXT */
PUBLIC
SECTION *	Section_Next (SECTION * sec)
/*			~~~~~~~~~~~~
 *	Get the next section from the list
 *------------------------------------------------------------------*/
{
	return (SECTION *)LIST_Next(sec_list, sec);
}


/*========================================================= SECTION_DEFINE */
PUBLIC
SECTION *	Section_Define	 (char *name)
/*			~~~~~~~~~~~~~~
 * Define a new section structure.
 *-------------------------------------------------------------------------*/
{
	struct SPECIAL_SECTIONS * pSpecialSections;

	SECTION  *sec;
   
	DBG_ENTER(("SECTION_DEFINE", 0x81, "name='%s'", name));
	/*--------------------------------------
	 *	Check to see what section id to use
	 *	for which we want to give a
	 *	specific section ID.
	 *------------------------------------*/
	 for (pSpecialSections = SpecialSections;
			NULL != pSpecialSections->SectionName ;
				pSpecialSections++)
	 {
		DBG(("SECTION_DEFINE", 0x8008, "Checking %s against %s", \
									name, pSpecialSections->SectionName));
		if (0 == stricmp(name, pSpecialSections->SectionName))
		{
			DBG(("SECTION_DEFINE", 0x8008, "... found - exit loop"));
			break;
		}
		DBG(("SECTION_DEFINE", 0x8008,"... not found - try next entry"));
	}
	DBG(("SECTION_DEFINE", 0x88, ".. Section Id = %d", pSpecialSections->SectionId));

	if (NULL == (sec = LIST_NewNode (sec_list)))
	{
		 Halt(ERR_NOMEMORY);
	}

	*(pSpecialSections->ppSection) = sec;

	/*----------------------------------------
	 *	Set fields to standard default values
	 *---------------------------------------*/
	sec->sec_id 	= pSpecialSections->SectionId--;
	sec->sec_output = pSpecialSections->Output;
	sec->sec_reuse	= pSpecialSections->Reuse;
	sec->sec_name	= NameSpace_Use (name);
	sec->sec_start	= 0;
	sec->sec_offset = 0;
	sec->sec_length = 0;
	sec->sec_oldlen = 0;
	sec->sec_part	= NULL;
	/*----------------------------
	 *	Initialise attached lists
	 *---------------------------*/
	if (NULL == (sec->sec_parts = LIST_Clone (part_master)))
	{
		Halt (ERR_NOMEMORY);
	}
	if (NULL == (sec->sec_xref = LIST_Clone (xref_master)))
	{
		Halt (ERR_NOMEMORY);
	}

	/*----------------------------
	 *	Add node to current list
	 *---------------------------*/
	(void)LIST_Add (sec_list, (node_t)sec);

	if (udata_reuse_id == sec->sec_id)
	{
		DBG(("SECTION_DEFINE", 0x88, "Set section for REUSE start point"));
		reuse_sec = sec;
	}
	DBG_EXIT(("SECTION_DEFINE", 0x81, ""));
	return(sec);
}


/*================================================== SECTION_ADD_CODE */
PUBLIC
void	Section_Add_Code (SECTION * sec, char * code, long length)
/*		~~~~~~~~~~~~~~~~
 *	Adds code to the current section.
 *--------------------------------------------------------------------*/
{
#define SECPART_CODESIZE 1024

	register long		part_space;
	register long		part_offset;
	register char * 	ptr;

	SECPART *	secpart;

	DBG_ENTER(("SECTION_ADD_CODE", 0x2001, "length=%ld, section=%s (length=%d, offset=%ld($%p)) length=%d", \
							length, \
							sec->sec_name, \
							sec->sec_length, \
							sec->sec_offset, \
							sec->sec_offset));

	/*-------------------------
	 *	If we need a new part,
	 *	then go and get one.
	 *------------------------*/
	if (NULL == sec->sec_part )
	{

		DBG(("SECTION_ADD_CODE", 0x2008, ".. New Part needed"));
		if ((NULL == (secpart = LIST_NewNode (sec->sec_parts)))
		||	(NULL == (secpart->part_code = malloc (SECPART_CODESIZE))))
		{
			Halt (ERR_NOMEMORY);
		}
		secpart->part_start  = (NULL == LIST_First(sec->sec_parts)) ? 0 : sec->sec_length;
		secpart->part_size	 = SECPART_CODESIZE;
		secpart->part_used	 = 0;
		secpart->part_module = NameSpace_Use(curr_mod);
		sec->sec_part = secpart;
		(void) LIST_Append (sec->sec_parts, secpart);
		
		DBG(("SECTION_ADD_CODE", 0x2008, ".. New Part: start=%d, size=%d", \
											secpart->part_start, \
											secpart->part_size));
	}


	/*------------------------------------------
	 *	Check we are looking at the
	 *	correct section part, and if
	 *	not reset ourselves.
	 *
	 *	We can take advantage of the fact that
	 *	we know parts are in ascending address
	 *	order with no gaps!
	 *-----------------------------------------*/
	for (; ; )
	{
		static	unsigned int  newpartcount = 0;

		secpart = sec->sec_part;

		/*
		 *	If we are too late in the file,
		 *	then move backwards.  Not sure
		 *	if this will ever happen in practise
		 *	but it must be allowed for.
		 */
		if (sec->sec_offset < secpart->part_start)
		{
			DBG(("SECTION_ADD_CODE", 0x2008, "Moving to previous section"));
			secpart = sec->sec_part
					= LIST_Previous (sec->sec_parts, secpart);
			/*-----------------------------------------
			 *	Do not think we should ever run off
			 *	the front of the section part chain
			 *	as they will (probably) always start
			 *	at zero.  May need to revisit this
			 *	assumption if we start stopping here.
			 *----------------------------------------*/
			if (NULL == secpart)
			{
				DBG(("SECTION_ADD_CODE", 0x01, "Fell off start of list!"));
				Halt (ERR_BINFILE);
			}
			else
			{
			DBG(("SECTION_ADD_CODE", 0x2008, "..found previous part at $%p: start=%ld, used=%ld)", \
									secpart, \
									secpart->part_start, \
									secpart->part_used));
			}
			continue;
		}
		/*----------------------------------------------------
		 *	If we are too early in the file then move
		 *	forwards.  This could be because we need
		 *	a new part (and thus the part does not yet
		 *	exist, or alternatively we could be handling
		 *	the XREF calculations and moving forward
		 *	through the parts writing back claculated values.
		 *---------------------------------------------------*/
		if (sec->sec_offset > secpart->part_start + secpart->part_size)
		{
			DBG(("SECTION_ADD_CODE", 0x2008, "Moving to next section"));
			secpart = sec->sec_part
					= LIST_Next (sec->sec_parts, secpart);
			/*--------------------------------------
			 *	If the part does not exist,
			 *	the call ourselves recursively
			 *	to get a new one allocated.
			 *	(Should we put in a check to stop
			 *	this handling to much
			 *-------------------------------------*/
			if (NULL == secpart)
			{
				if (newpartcount > 5)
				{
					DBG(("SECTION_ADD_CODE", 0x01, "Excessive recursion!"));
					Halt (ERR_BINFILE);
				}
				newpartcount ++;
				DBG(("SECTION_ADD_CODE", 0x2008, "... does not exist - Call recursivel to get new part"));
				Section_Add_Code(sec, code, length);
				DBG_EXIT(("SECTION_ADD_CODE", 0x2001, "code=$%p, offset=$%p", \
									code, sec->sec_offset));
				return;
			}
			DBG(("SECTION_ADD_CODE", 0x2008, "..found next part at $%p: start=%ld, used=%ld)", \
									secpart, \
									secpart->part_start, \
									secpart->part_used));
			continue;
		}
		/*-----------------------------------
		 *	We got here - we must be in the
		 *	correct section part !
		 *----------------------------------*/
		newpartcount = 0;
		break;
	}

	/*--------------------------------
	 *	Work out how much of desired
	 *	data fits into current part.
	 *	Adjust 'sec' fields.
	 *-------------------------------*/
	part_offset = sec->sec_offset - secpart->part_start;
	part_space	= secpart->part_size - part_offset;
	if (part_space > length)
	{
		part_space = length;
	}
	length			-= part_space;
	part_offset 	+= part_space;
	sec->sec_offset += part_space;
	if (sec->sec_offset > sec->sec_length)
	{
		sec->sec_length = sec->sec_offset;
	}
	if (part_offset > secpart->part_used)
	{
		secpart->part_used = sec->sec_offset - secpart->part_start;
	}
	/*--------------------------------------
	 *	Copy across as much of the data as
	 *	we can fit into this section part
	 *-------------------------------------*/
	for (ptr =	&secpart->part_code[part_offset - part_space] ;
						part_space > 0 ; part_space--)
	{
		*ptr++ = *code++;
	}
	/*-------------------------------------
	 *	If all the code did not fit into
	 *	this part, then set up to put the
	 *	remainder into the next part.
	 *-------------------------------------*/
	if (0 != length)
	{
		sec->sec_part = LIST_Next(sec->sec_parts, sec->sec_part);
		DBG(("SECTION_ADD_CODE", 0x2008, "Call recursively to add last %ld bytes (sec->secpart=$%p", \
											length, sec->sec_part));
		Section_Add_Code (sec, code, length);
	}
	DBG_EXIT(("SECTION_ADD_CODE", 0x2001, "length=%ld($%p), offset=%ld($%p)", \
							sec->sec_length, sec->sec_length, \
							sec->sec_offset, sec->sec_offset));
	return;

#undef SECPART_CODESIZE
}

/*==================================================== SECTION_ADD_LONG */
PUBLIC
void	Section_Add_Long (SECTION * sec, long l)
/*
 *----------------------------------------------------------------------*/
{
	char	c[4];

	c[0] = (char) (l >> 24);
	c[1] = (char) (l >> 16);
	c[2] = (char) (l >> 8);
	c[3] = (char) l;
	Section_Add_Code (sec, c, 4);
}


/*=================================================== SECTION_ADD_SHORT */
PUBLIC
void	Section_Add_Short (SECTION * sec, short s)
/*
 *----------------------------------------------------------------------*/
{
	char	c[2];

	c[0] = (char) (s >> 8);
	c[1] = (char) s;
	Section_Add_Code (sec, c, 2);
}


/*=================================================== SECTION_ADD_BYTE */
PUBLIC
void	Section_Add_Byte (SECTION * sec, char b)
/*
 *----------------------------------------------------------------------*/
{
	char	c[1];

	c[0] = (char) b;
	Section_Add_Code (sec, c, 1);
}


/*============================================== SECTION_STARTMODULE */
PRIVATE
long		Section_StartModule (node_t node, va_list ap)
/*			~~~~~~~~~~~~~~~~~~~
 *	Routine called when at the start of a module.
 *
 *	It initialises the section structures ready to
 *	handle the new module.
 *
 *	It is written as a call-back function designed to be used
 *	by the LIST_Enumerate method from the LIBLIST library.
 *-------------------------------------------------------------------*/
{
#define sec ((SECTION *)node)

	(void) ap;	/* Dummy cast to stop warning about unused parameter */

	DBG_ENTER(("SECTION_STARTMODULE", 0x201, "Section %s, length=%ld($%p), offset=%ld($%p), oldlen=%ld($%p)", \
										sec->sec_name, \
										sec->sec_length, sec->sec_length, \
										sec->sec_offset, sec->sec_offset, \
										sec->sec_oldlen, sec->sec_oldlen ));
	/*
	 *	Set defaults for starting a new section.
	 */
	sec->sec_offset = sec->sec_length;
	sec->sec_oldlen = sec->sec_length;
	sec->sec_part = NULL;	/* No part currently active */

	DBG_EXIT(("SECTION_STARTMODULE", 0x201, "Section %s, length=%ld($%p), offset=%ld($%p), oldlen=%ld($%p)", \
										sec->sec_name, \
										sec->sec_length, sec->sec_length,  \
										sec->sec_offset, sec->sec_offset, \
										sec->sec_oldlen, sec->sec_oldlen ));
	return 0;
#undef sec
}


/*==================================================== SECTION_START */
PUBLIC
void	Section_Start (void)
/*		~~~~~~~~~~~~~
 *	Called at the start of a module set up
 *	correct section start conditions
 *--------------------------------------------------------------------*/
{
	DBG_ENTER(("SECTION_START",0x101,""));
	(void)LIST_Enumerate (sec_list, Section_StartModule);
	DBG_EXIT(("SECTION_START", 0x101, ""));
	return;
}


/*================================================= SECTION_ENDMODULE */
PRIVATE
long		Section_EndModule (node_t node, va_list ap)
/*			~~~~~~~~~~~~~~~~~
 *	Routine called when at the end of a module.
 *
 *	It ensures that all the section parts for this
 *	module end on an even boundary ready for any new
 *	module.
 *
 *	It is written as a call-back function designed to be used
 *	by the LIST_Enumerate method from the LIBLIST library.
 *-------------------------------------------------------------------*/
{
#define sec ((SECTION *)node)

	SECPART *	secpart;

	(void) ap;	/* Dummy cast to stop warning about unused parameter */

	DBG_ENTER(("SECTION_ENDMODULE", 0x201, "Section %s, length=%ld($%p), offset=%ld($%p), oldlen=%ld($%p)", \
										sec->sec_name, \
										sec->sec_length, sec->sec_length, \
										sec->sec_offset, sec->sec_offset, \
										sec->sec_oldlen, sec->sec_oldlen ));

	/*--------------------------------------
	 *	If we did anything to this section
	 *	during the last module then we
	 *	need to tidy up the last section.
	 *------------------------------------*/
	if (NULL != ((SECTION *)node)->sec_part)
	{
		secpart = LIST_Last (sec->sec_parts);
		/*--------------------------------
		 *	If it is odd, then even it up
		 *--------------------------------*/
		if (secpart->part_used & 1)
		{
			DBG(("SECTION_ENDMODULE", 0x208, "... adding NULL byte to last section part to make even address"));
			curr_sec = sec;
			Section_Add_Byte (sec, '\0');
		}
		/*----------------------------
		 *	Now shrink the code size
		 *	to fit the space used.
		 *---------------------------*/
		if (secpart->part_used != secpart->part_size)
		{
			DBG(("SECTION_ENDMODULE", 0x208, "... shrinking last part from %ld to %ld (start=%ld)", \
										secpart->part_size, \
										secpart->part_used, \
										secpart->part_start));
			secpart->part_code = realloc (secpart->part_code, secpart->part_used);
			secpart->part_size = secpart->part_used;
		}
	}

	if (sec->sec_offset > sec->sec_length)
	{
		sec->sec_length = sec->sec_offset;
	}
	DBG_EXIT(("SECTION_ENDMODULE", 0x201, "Section %s, length=%ld($%p), offset=%ld($%p), oldlen=%ld($%p)", \
										sec->sec_name, \
										sec->sec_length, sec->sec_length, \
										sec->sec_offset, sec->sec_offset, \
										sec->sec_oldlen, sec->sec_oldlen ));
	return 0;
#undef sec
}

/*====================================================== SECTION_END */
PUBLIC
void	Section_End (void)
/*		~~~~~~~~~~~
 *	Called at the start of a module set up
 *	correct section start conditions
 *--------------------------------------------------------------------*/
{
	DBG_ENTER(("SECTION_END",0x101,""));
	(void)LIST_Enumerate (sec_list, Section_EndModule);
	DBG_EXIT(("SECTION_END", 0x101, ""));
	return;
}



/*=================================================== SECTION SETSTART */
PRIVATE
long		Section_SetStart (node_t node, va_list ap)
/*			~~~~~~~~~~~~~~~~
 *	Set the section start fields.
 *
 *	This method is used to set up current values for the
 *	'start' address within the program for each section.
 *
 *	It also sets the flag to indicate whether this
 *	particular section should be output.
 *
 *	It is written as a call-back function designed to be used
 *	by the LIST_Enumerate method from the LIBLIST library.
 *---------------------------------------------------------------------*/
{
#define sec ((SECTION *)node)

	SECTION * nextsec;

	(void) ap;		/* Dummy cast to stop warning about unused variable */

	DBG_ENTER(("SECTION_SETSTART", 0x101, "section=%s, start=%ld, length=%ld", \
								sec->sec_name, sec->sec_start, sec->sec_length));

	if (SECTION_UDATA != sec->sec_id
	||	TRUE == udata_flag)
	{
		progfile_size = sec->sec_start + sec->sec_length;
	}
	nextsec = LIST_Next (sec_list, sec);
	if (NULL != nextsec)
	{
		nextsec->sec_start = progfile_size;
		DBG(("SECTION_SETSTART", 0x108, "Set section %s start to %ld", \
							nextsec->sec_name, nextsec->sec_start));
	}
	/*------------------------------------
	 *	The UDATA section has special
	 *	treatment as it can reuse space
	 *	from other sections (and normally
	 *	it will).
	 *------------------------------------*/
	if (SECTION_UDATA == sec->sec_id)
	{
		sec->sec_start = reuse_sec->sec_start;
	}
	DBG_EXIT(("SECTION_SETSTART", 0x101, "section=%s", sec->sec_name));
	return 0;

#undef sec
}


/*================================================= SECTION_SETSTARTS */
PUBLIC
void	Section_SetStarts (void)
/*		~~~~~~~~~~~~~~~~~
 *--------------------------------------------------------------------*/
{
	DBG_ENTER(("SECTION_SETSTARTS",0x101,""));
	(void)LIST_Enumerate (sec_list, Section_SetStart);
	DBG_EXIT(("SECTION_SETSTARTS", 0x101, ""));
	return;
}



/*====================================================== SECTION_FINDNODE */
PRIVATE
int 	Section_FindNode (node_t node, va_list ap)
/*		~~~~~~~~~~~~~~~~
 *	Find a section given the name.
 *
 *	This is a call-back function for use with
 *	the LIST_Find() library routine.
 *------------------------------------------------------------------------*/
{
	int 	reply;
	char *	name = va_arg(ap, char *);

#if 0
	DBG_ENTER(("SECTION_FINDNODE", 0x8001, "node name='%s', wanted='%s'", \
							((SECTION *)node)->sec_name, name));
#endif
	reply = strcmp ( ((SECTION *)node)->sec_name, name);
	if (reply != 0)
	{
		reply = LIST_ORDER_NA;
	}
#if 0
	DBG_EXIT(("SECTION_FINDNODE", 0x8001, "%d", reply));
#endif
	return reply;
}


/*======================================================== SECTION_CHECK */
PUBLIC
SECTION *	Section_Check (char * name)
/*			~~~~~~~~~~~~~
 *	See if the current section already exists.
 *	If it does return a pointer to its definition.
 *	If it does not, then create a new entry in
 *	the lst and return a pointer to the new entry.
 *----------------------------------------------------------------------*/
{
	SECTION * sec;

	DBG_ENTER(("SECTION_CHECK", 0x1001, "name=%s", name));

	sec = LIST_Find (sec_list, Section_FindNode, name);

	if (NULL == sec)
	{
		DBG (("SECTION_CHECK", 0x1001, "... does not exist - need to create new entry"));

		sec = Section_Define (name);
	}
	DBG_EXIT (("SECTION_CHECK", 0x1001, "reply=$%p", sec));
	return sec;
}


/*============================================================== CALC_OP */
PRIVATE
long	Calc_Op  (node_t node, va_list ap)
/*		~~~~~~
 *	Calculate the result of a single OP on an XREF.
 *
 *	Written as a call-back function for use in conjunction
 *	with the LIST_Enumerate() method from the lIBLIST library.
 *-----------------------------------------------------------------------*/
{
#define xoper ((XOPER *)node)

	XSYMBOL * xsym;
	long	 tempvalue;
	long	 pcounter;
	long *	 pvalue;

	pcounter = va_arg(ap, long);
	pvalue = va_arg(ap, long *);

	DBG_ENTER(("CALC_OP", 0x801, "value=%ld", *pvalue));

	switch (xoper->xop_optyp)
	{
	case OPTYPE_SECTION :

			assert (NULL != xoper->xop_ptr.xop_sec);

			tempvalue = xoper->xop_ptr.xop_sec->sec_start;

			DBG(("CALC_OP", 0x408, " %c Section: %s+$%x)", \
							  xoper->xop_oper, \
							  xoper->xop_ptr.xop_sec->sec_name, \
							  tempvalue - xoper->xop_ptr.xop_sec->sec_start));
			break;

	case  OPTYPE_PCOUNTER :

			tempvalue = pcounter;

			DBG(("CALC_OP", 0x408, " %c ProgCounter: $%x", \
							  xoper->xop_oper, \
							  tempvalue));
			break;

	case  OPTYPE_SYMBOL :
			assert (NULL != xoper->xop_ptr.xop_sym);
			xsym = xoper->xop_ptr.xop_sym;
			tempvalue = Symbol_Calc (xsym);
			DBG(("CALC_OP", 0x408, " %c Symbol: %s/$%x", \
							  xoper->xop_oper, \
							  xsym->xsy_name, \
							  tempvalue));
			break;

	default:
			Halt (ERR_BINFILE);
	}

	/*
	 *	Now apply the calculated
	 *	value to the result area
	 */
	if (xoper->xop_oper=='+')
	{
		  *pvalue += tempvalue;
	}
	else
	{
		  *pvalue -= tempvalue;
	}

	DBG_EXIT(("CALC_OP", 0x801, "value=%ld", *pvalue));
	return 0;
#undef xoper
}



/*=================================================== SECTION_CALCXREF */
PRIVATE
long	Section_CalcXref (node_t node, va_list ap)
/*		~~~~~~~~~~~~~~~~
 *
 *	Take a single XREF within a section and:
 *		a)	Update the entry with the XREF value
 *		b)	Create a relocation entry if needed.
 *
 *	It is written as a call-back function designed to be used
 *	by the LIST_Enumerate method from the LIBLIST library.
 *---------------------------------------------------------------------*/
{
#define x ((XREF * )node)

	SECTION * s;
	long  value;

	s = va_arg(ap, SECTION *);

	DBG_ENTER(("SECTION_CALCXREF", 0x408, "Section %s+$%p: XREF at %ld($%p): xref_abs=%ld($%p)",\
										s->sec_name, \
										x->xref_pos,
										x->xref_pos + s->sec_start, \
										x->xref_pos + s->sec_start, \
										x->xref_abs, x->xref_abs));

	/*-----------------------------------------
	 *	Set position where result must be put.
	 *----------------------------------------*/
	value = x->xref_abs;				/* Absolute value */
	s->sec_offset = x->xref_pos;		/* Position in section */

#if 0
	DBG(("SECTION_CALCXREF", 0x2008, ".. oper count = %ld", \
										LIST_Count (x->xref_oper)));
#endif
	(void) LIST_Enumerate (x->xref_oper,
							Calc_Op,
							s->sec_start + x->xref_pos,
							&value);

	DBG(("SECTION_CALCXREF", 0x2008, "..value=%ld($%p) - Now apply truncation rule ($%x)", \
									value, value, \
									x->xref_trunc));

	/*------------------------------------------
	 *	For PC-Relative change the value from
	 *	an absolute one to relative to the
	 *	current instruction.
	 *----------------------------------------*/
	if (0 != (x->xref_trunc & TRUNC_PCREL))
	{
		value -= s->sec_start + x->xref_pos;
		DBG(("SECTION_CALCXREF", 0x2008, "PC-Relative - changed to %ld($%p)", \
											value, value));
	}
	/*-------------------------
	 *	Now apply TRUNC rules
	 *------------------------*/
	switch(x->xref_trunc & (TRUNC_LONG | TRUNC_WORD | TRUNC_BYTE))
	{
		case TRUNC_LONG :
				if (((long)x->xref_pos) & 1)
				{
					Halt(ERR_PROGXREF);
				}
				Section_Add_Long (s, value);
				break;

		case TRUNC_WORD :
				if (((long)x->xref_pos) & 1)
				{
					Halt(ERR_PROGXREF);
				}
				Section_Add_Short (s, (short)value);
				if (x->xref_trunc & TRUNC_SIGNED)
				{
					if (value < -32768L || value > 32767L)
					{
						range_err++;
						(void)printf("XREF.W-value out of range in module '%s'\n",
							 x->xref_module);
					}
				}
				else
				{
					if (value > 65535L)
					{
						range_err++;
						(void)printf("XREF.UW-value out of range in module '%s'\n",
							 x->xref_module);
					}
				}
				break;

	  case TRUNC_BYTE :
			   Section_Add_Byte (s, (char)value);
			   if (x->xref_trunc & TRUNC_SIGNED)
			   {
				  if (value < -128L || value > 128L)
				  {
					 range_err++;
					 (void)printf("XREF.B-value out of range in module '%s'\n",
							 x->xref_module);
				  }
			   }
			   else
			   {
				  if (value>255L)
				  {
					 range_err++;
					 (void)printf("XREF.UB-value out of range in module '%s'\n",
							 x->xref_module);
				  }
			   }
			   break;

	  default:
				Halt(ERR_BINFILE);
   }

	DBG_EXIT(("SECTION_CALCXREF", 0x11, "Section=%s", \
										 s->sec_name));
	return 0;
}
#undef x



/*======================================================= SECTION_XREFS */
PRIVATE
long	Section_Xrefs (node_t node, va_list ap)
/*		~~~~~~~~~~~~~
 *
 *	Calculate all XREFs for a section.
 *
 *	It is written as a call-back function designed to be used
 *	by the LIST_Enumerate method from the LIBLIST library.
 *----------------------------------------------------------------------*/
#define sec ((SECTION *)node)
{
	(void) ap;		/* Dummy cast to stop warning about unused parameter */

	DBG_ENTER(("SECTION_XREFS", 0x11, "Section=%s", sec->sec_name));

	/*-----------------------------------
	 *	For convenience, reset section
	 *	pointers to first part.
	 *---------------------------------*/
	sec->sec_part = LIST_First (sec->sec_parts);
	(void) LIST_Enumerate (sec->sec_xref,
						   Section_CalcXref,
						   (SECTION *)node);

	DBG_EXIT(("SECTION_XREFS", 0x11, "Section=%s", sec->sec_name));
	return 0;
#undef sec
}


/*================================================== SECTION_ALLXREFS */
PUBLIC
void	Section_AllXrefs (void)
/*		~~~~~~~~~~~~~~~~
 *--------------------------------------------------------------------*/
{
	DBG_ENTER(("SECTION_ALLXREFS",0x101,""));
	Section_SetStarts();
	(void) LIST_Enumerate (sec_list, Section_Xrefs);

	DBG_EXIT(("SECTION_ALLXREFS", 0x101, ""));
	return;
}


/*================================================= SECTION_WRITE_PART */
PRIVATE
long	Section_Write_Part (node_t node, va_list ap)
/*		~~~~~~~~~~~~~~~~~~
 *	Write out a section part.
 *
 *	This is written as a call-back function for use with
 *	the LIST_Enumerate() function from the LIBLIST library.
 *---------------------------------------------------------------------*/
#define secpart ((SECPART*)node)
{
	int 	n;

	(void)ap;	/* Dummy cast to stop warning about unused parameter */

	DBG_ENTER(("SECTION_WRITE_PART", 0x01, "module '%s', start=%ld, length=%ld", \
								secpart->part_module, \
								secpart->part_start, \
								secpart->part_used));

	DBG(("SECTION_WRITE_PART", 0x18, "before write: filesize=%ld", \
								lseek (progfile_handle, 0, SEEK_END)));

	n = write(progfile_handle,
			  secpart->part_code,
			  secpart->part_used);

	if (n != secpart->part_used)
	{
		DBG(("SECTION_WRITE_PART", 0x01, " .. problem: n=%d", n));
		Halt(ERR_WRITEFILE);
	}

	DBG(("SECTION_WRITE_PART", 0x18, "after write: filesize=%ld", \
								lseek (progfile_handle, 0, SEEK_END)));

	DBG_EXIT(("SECTION_WRITE_PART", 0x01, ""));
	return 0;
}
#undef secpart


/*====================================================== SECTION_WRITE */
PRIVATE
long	Section_Write (node_t node, va_list ap)
/*		~~~~~~~~~~~~~
 *	Write out a single section as indicated by the 'node' parameter.
 *
 *	This is written as a call-back function for use with
 *	the LIST_Enumerate() function from the LIBLIST library.
 *
 *	When the special section SECTION_UDATA is reached,
 *	then the address of this is returned to stop
 *	enumerating the section list.
 *---------------------------------------------------------------------*/
{
	(void)ap;	/* Dummy cast to stop warning about unused parameter */

	DBG_ENTER(("SECTION_WRITE", 0x01, "section '%s' (id=%d), start=%d, length=%d", \
								((SECTION *)node)->sec_name, \
								((SECTION *)node)->sec_id, \
								((SECTION *)node)->sec_start, \
								((SECTION *)node)->sec_length));

	if (((SECTION *)node)->sec_id == SECTION_UDATA)
	{
		/*-----------------------------------------------
		 *	Normally we do not output the UDATA section.
		 *	However if the flag is set we must.
		 *----------------------------------------------*/
		DBG_EXIT(("SECTION_WRITE", 0x01, "Exit: SECTION_UDATA reached"));
		if (TRUE == udata_flag)
		{
			long	outlen;

			outlen = progfile_size - lseek (progfile_handle, 0, SEEK_END);
			DBG(("SECTION_WRITE", 0x08, "Section UDATA - outputting %ld zero bytes to make filesize up to %ld", \
										outlen, progfile_size));
			while (outlen-- > 0)
			{
				(void) write (progfile_handle, "\0", 1);
			}
		}
		return 0;
	}
	(void)LIST_Enumerate (((SECTION *)node)->sec_parts, Section_Write_Part);
	DBG_EXIT(("SECTION_WRITE", 0x01, "0"));
	return 0;
}


/*================================================== SECTION_WRITEALL */
PUBLIC
void	Section_WriteAll (void)
/*		~~~~~~~~~~~~~~~~
 *	Called at the start of a module set up
 *	correct section start conditions
 *--------------------------------------------------------------------*/
{
	DBG_ENTER(("SECTION_WRITEALL",0x101,""));
	(void)LIST_Enumerate (sec_list, Section_Write);
	DBG_EXIT(("SECTION_WRITEALL", 0x101, ""));
	return;
}
