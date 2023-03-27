/*
 *	   l d s y m b o l _ c
 *
 *	   This module is the one that has all the routines related to
 *		handling the SYMBOL object.
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

PRIVATE  list_t   xsy_list = NULL;	   /* List of External symbols */


/*==================================================== SYMBOL_COMPNODE */
PRIVATE
int 	Symbol_CompNode (node_t  node1,  node_t node2)
/*		~~~~~~~~~~~~~~~
 *	This is a symbol compare routine.
 *
 *	It is a call-back function for use with the
 *	LIBLIST library.
 *---------------------------------------------------------------------*/
{
	return strcmp (((XSYMBOL *)node1)->xsy_name,
				   ((XSYMBOL *)node2)->xsy_name);
}


/*======================================================== SYMBOL_INIT */
PUBLIC
void	Symbol_Init (void)
/*		~~~~~~~~~~~
 *	Initialise symbol handling
 *---------------------------------------------------------------------*/
{
	xsy_list = LIST_Create (LIST_CLASS_BTREE,
							sizeof (XSYMBOL),
							NULL,
							NULL,
							Symbol_CompNode);

	if (NULL == xsy_list)
	{
		Halt (ERR_NOMEMORY);
	}
	return;
}

/*====================================================== SYMBOL_COUNT */
PUBLIC
long	Symbol_Count (void)
/*		~~~~~~~~~~~~
 *--------------------------------------------------------------------*/
{
	return LIST_Count(xsy_list);
}


/*================================================== SYMBOL_ENUMERATE */
PUBLIC
long	Symbol_Enumerate (long (*func)(node_t, va_list))
/*		~~~~~~~~~~~~~~~~
 *	Enumerate through the symbol list using the supplied
 *	function as theenumeration routine.
 *--------------------------------------------------------------------*/
{
	return LIST_Enumerate (xsy_list, func);
}

/*=================================================== SYMBOL_FINDNODE */
PRIVATE
int 	Symbol_FindNode (node_t node, va_list ap)
/*		~~~~~~~~~~~~~~~
 *	This is a symbol find callback routine
 *	used in conjunction with the LIBLIST library.
 *--------------------------------------------------------------------*/
{
	return strcmp(((XSYMBOL *)node)->xsy_name,
					va_arg (ap, char *));
}


/*====================================================== SYMBOL_FIND */
PUBLIC
XSYMBOL *	Symbol_Find  (char * name)
/*			~~~~~~~~~~~
 *	Find if a symbol already exists.
 *	If it does not then return NULL.
 *	If it does, then return a pointer to the entry
 *-------------------------------------------------------------------*/
{
	XSYMBOL * xsy;

#if 0
	DBG_ENTER(("SYMBOL_FIND", 0x2001, "name='%s'", name));
#endif
	xsy = LIST_Find (xsy_list, Symbol_FindNode, name);

#if 0
	DBG_EXIT(("SYMBOL_FIND", 0x2001, "reply=$%p", xsy));
#endif

	return xsy;
}

/*===================================================== SYMBOL_CHECK */
PUBLIC
XSYMBOL *	Symbol_Check (char * name)
/*			~~~~~~~~~~~~
 *	This routine is used to find a symbol if it exists,
 *	and if it does not to create a new entry.
 *
 *	Returns an pointer to the result.
 *-------------------------------------------------------------------*/
{
	XSYMBOL * xsy;

#if 0
	DBG_ENTER(("SYMBOL_CHECK", 0x2001, "name='%s'", name));
#endif
	if (NULL == (xsy = LIST_Find (xsy_list, Symbol_FindNode, name)))
	{
		DBG(("SYMBOL_CHECK", 0x2008, "... '%s' not found - new entry created", name));
		/*---------------------------
		 *	Create a new symbol node
		 *--------------------------*/
		xsy = LIST_NewNode(xsy_list);
		if (NULL == xsy)
		{
			Halt(ERR_NOMEMORY);
		}
		/*---------------------------------
		 *	Initialise with default values
		 *--------------------------------*/
		xsy->xsy_name	= NameSpace_Use(name);
		xsy->xsy_defd	= XSYMBOL_UNDEFINED;
		xsy->xsy_mod	= NULL;
		xsy->xsy_sect	= NULL;
		xsy->xsy_offset = 0;
		xsy->xsy_value	= 0;
		/*------------------------------
		 *	Add node to list of symbols
		 *-----------------------------*/
		(void)LIST_Add (xsy_list, xsy);
	}
	else
	{
		DBG(("SYMBOL_CHECK", 0x2008, "... '%s' found", name));
	}
#if 0
	DBG_EXIT(("SYMBOL_CHECK", 0x2001, ""));
#endif
	return xsy;
}


/*======================================================= SYMBOL_CASE */
PUBLIC
void	Symbol_Case 	(int flag)
/*		~~~~~~~~~~~
 *	Decide if a symbol should be treated as case-independent.
 *	If so convert it to upper case.
 *--------------------------------------------------------------------*/
{
	DBG_ENTER(("SYMBOL_CASE", 0x8008, "name=%s, flag=%d", \
								sy.string, flag));
	/*--------------------------------------------
	 *	Section names are always case independent
	 *	so always convert them to upper case.
	 *-------------------------------------------*/
	if (flag > 0)
	{
		/*--------------------------------
		 *	For symbols do this according
		 *	to flag setting.
		 *-------------------------------*/
		DBG_EXIT(("SYMBOL_CASE", 0x8008, "symbol - not changed"));
		return;
	}

	(void) strupr (sy.string);

	DBG_EXIT(("SYMBOL_CASE", 0x8008, "name=%s", sy.string));
	return;
}


/*=============================================== SYMBOL_CHECKUNDEFINED */
PRIVATE
long	Symbol_CheckUndefined (node_t node, va_list ap)
/*		~~~~~~~~~~~~~~~~~~~~~
 *	Routine to say if a symbol is UnDefined.
 *	Return 0 if it is not, and some other value if it is.
 *----------------------------------------------------------------------*/
{
	(void)ap;
	if (((XSYMBOL *)node)->xsy_defd & XSYMBOL_DEFINED)
	{
		return 0;
	}
	DBG(("SYMBOL_CHECKUNDEFINED", 0x08, "Symbol %s undefined", \
								((XSYMBOL *)node)->xsy_name));
	return 1;
}

/*==================================================== SYMBOL_ALLDEFINED */
PUBLIC
int 		Symbol_AllDefined (void)
/*			~~~~~~~~~~~~~~~~~~~
 *	Routine to find out if there are any undefined symbols left
 *-----------------------------------------------------------------------*/
{
	return LIST_Enumerate (xsy_list, Symbol_CheckUndefined);
}


/*========================================================== SYMBOL_CALC */
PUBLIC
long	Symbol_Calc  (XSYMBOL * xsy)
/*		~~~~~~~~~~~
 *	Calculate the value of an external symbol.
 *	Basically this means (unless it is an absolute
 *	symbol) convert its offset in a section into
 *	an offset in the program as a whole).
 *-----------------------------------------------------------------------*/
{
	long	reply;

#if 0
	DBG_ENTER (("SYMBOL_CALC", 0x8001, "symbol='%s', offset=$%p", \
									xsy->xsy_name, \
									xsy->xsy_offset));
#endif

	reply = xsy->xsy_offset;

	if (NULL != xsy->xsy_sect)
	{
#if 0
		DBG(("SYMBOL_CALC", 0x8008, "+ Section %s ($%p)", \
								xsy->xsy_sect->sec_name, \
								xsy->xsy_sect->sec_start));
#endif
		reply += xsy->xsy_sect->sec_start;
	}

#if 0
	DBG_EXIT (("SYMBOL_CALC", 0x8001, "reply=$%p", reply));
#endif

	return reply;
}
