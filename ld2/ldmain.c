/*
 *	   l d m a i n _ c
 *
 *		Linker for use with the QDOS C68 system.
 *
 *		Based on a linker for the Atari ST posted to the Public
 *		Domain via Usenet.
 *
 *	   This module is the main module.
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



#define  INIT(params)	= params
#include "ld.h"

#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <assert.h>

#ifdef QDOS
void consetup_title();
void (*_consetup)() = consetup_title;
long (*_writetrans)() = NULL;
int  (*_Open)(const char*,int,...)			= qopen;
  
long _stack = 4*1024; /* 4K stack */
long _stackmargin = 1024L;
#endif /* QDOS */



/*======================================================== PRINT_ERROR */
PUBLIC
void	print_error (char	 *string)
/*		~~~~~~~~~~~
 *---------------------------------------------------------------------*/
{
	eprintf ("Error errno=%d: %s\n", errno, string);
#if 0
	if (errno < 0)
	{
	   (void) eprintf ("Unknown error %d\n", errno);
#ifdef QDOS
		(void)poserr ("poserr:");
#endif
	}
	else
	{
		perror (string);
	}
#endif
	return;
}



/*============================================================= NXSY */
PRIVATE
void	nxsy (void)
/*		~~~~
 *	Get next symbol.
 *
 *	This is the basic function that is used to analyse
 *	the SROFF file into entities that can then be
 *	processed by the remainder of the linker.
 *--------------------------------------------------------------------*/
{
	static	int  stored_flag = 0;

	char * p;
	int    c;

	if (0 == stored_flag)
	{
		c = get_byte();
	}
	else
	{
		c = stored_flag;
		stored_flag = 0;
	}

	switch (c)
	{
	case EOF:
			if (c == EOF)
			{
				sy.directive= DIRECTIVE_EOF;
				sy.length=0;
			}
			break;

	default:
			sy.directive = DIRECTIVE_DATA;
			/*
			 *	Get bytes until we either fill up the
			 *	space allocated or encounter a directive.
			 */
			for (p = sy.string, *p++ = c, sy.length=1 ;
					sy.length < sizeof (sy.string) ;
						sy.length++, p++)
			{
				c = get_byte();
				if (c == SROFF_FLAG || c == EOF)
				{
					stored_flag = c;
					break;
				}
				*p = c;
			}
			break;

	case SROFF_FLAG:
	{
	  int	i, ch;
	  sy.id = 0;
	  sy.length = 0;
	  switch(ch=get_byte())
	  {
		 case EOF :
						DBG(("NXSY", 0x11, "Unexpected EOF encountered"));
						sy.directive= DIRECTIVE_EOF;
						break;

		 case SROFF_SOURCE :
						sy.directive= DIRECTIVE_SOURCE;
						p=sy.string;
						for(i=get_byte();i--;)
						{
							*p++ = get_byte();
						}
						*p='\0';
						break;

		 case SROFF_COMMENT :
						sy.directive = DIRECTIVE_COMMENT;
						p=sy.string;
						for(i=get_byte();i--;)
						{
							*p++=get_byte();
						}
						*p='\0';
						break;

		 case SROFF_ORG  :
						sy.directive= DIRECTIVE_ORG;
						sy.longword = get_long();
						break;

		 case SROFF_SECTION :
						sy.directive= DIRECTIVE_SECTION;
						sy.id=get_short();
						break;

		 case SROFF_OFFSET :
						sy.directive= DIRECTIVE_OFFSET;
						sy.longword=get_long();
						break;

		 case SROFF_XDEF :
						sy.directive= DIRECTIVE_XDEF;
						p=sy.string;
						for(i=get_byte();i--;)
						{
							*p++=get_byte();
						}
						*p='\0';
						sy.longword= get_long();
						sy.id = get_short();
						Symbol_Case(case_flag);
						break;

		 case SROFF_XREF :
						sy.directive= DIRECTIVE_XREF;
						sy.longword= get_long();
						sy.trunc_rule=get_byte();
						sy.length = sy.trunc_rule&7;
						(void) LIST_Clear (sy.xref_oper);
						while (SROFF_FLAG != (c=get_byte()))
						{
							switch (c)
							{
							OPER *	opernode;

							case '+':
							case '-':
									opernode = LIST_NewNode (sy.xref_oper);
									if (NULL == opernode)
									{
										Halt (ERR_NOMEMORY);
									}
									opernode->op = c;
									opernode->id = get_short();
#if 0
									DBG(("NXSY", 0x8008, "Added XREF op (op='%c' id=%d)", \
														opernode->op, opernode->id));
#endif
									(void)LIST_Append(sy.xref_oper, opernode);
									break;

							default:
									(void)printf("Illegal XREF Operator : %c\n",c);
									Halt(ERR_BINFILE);
						   }
						}
						if (c != SROFF_FLAG)
						{
							Halt(ERR_XREFOPERS);
						}
						break;

		 case SROFF_DEFINE :
						sy.directive = DIRECTIVE_DEFINE;
						sy.id=get_short();
						p=sy.string;
						for (i=get_byte();i--;)
						{
							*p++=get_byte();
						}
						*p='\0';
						Symbol_Case((sy.id < 0) ? 0 : case_flag);
						break;

		 case SROFF_COMMON :
						sy.directive= DIRECTIVE_COMMON;
						sy.id=get_short();
						break;

		 case SROFF_END :
						sy.directive= DIRECTIVE_END;
						break;

		 case SROFF_FLAG :
						sy.directive = DIRECTIVE_DATA;
						sy.length = 1;
						sy.string[0] = (char)SROFF_FLAG;
						break;

		 default	 :	eprintf("Illegal Directive $%02.2x\n",
										(unsigned)ch);
						Halt(ERR_BINFILE);
						break;

	  } /* end of SROFF_FLAG switch */
	  break;
	} /* end of case SROFF_FLAG */
   } /* end of 'c' switch */
   return;
}


/*========================================================= COMMENT_DIR */
PRIVATE
void	comment_dir (void)
/*		~~~~~~~~~~~
 * Write out a comment
 *---------------------------------------------------------------------*/
{
	DBG_ENTER(("COMMENT_DIR", 0x81, ""));
	if (list_file !=stdout || verbose_flag)
	{
	   (void)fprintf(list_file,"COMMENT: %s\n",sy.string);
	}
	nxsy();
	DBG_EXIT(("COMMENT_DIR", 0x81, ""));
}


/*============================================================= XDEF_DIR */
PRIVATE
void	xdef_dir(void)
/*		~~~~~~~~
 * Define a XDEF symbol
 *-----------------------------------------------------------------------*/
{
	XSYMBOL  *s;
   
	DBG_ENTER(("XDEF_DIR", 0x81, "name='%s', section id=%d", \
							sy.string, sy.id));
	s = Symbol_Check (sy.string);
   if (0 != (s->xsy_defd & XSYMBOL_DEFINED))
   {
		lprintf ("Double defined Symbol: %s\n",sy.string);
		double_sym++;
		assert ("double defined symbol");
   }
   else
   {
		if (sy.id)
		{
			if (sy.id>0)
			{
				Halt(ERR_SECTIONID, sy.id, "Defining XDEF symbol");
			}
			if ((size_t)(-sy.id) > ndef_size)
			{
				Halt(ERR_TABLE, "XDEF symbols");
			}
			s->xsy_sect = Section_Check (ndef_name[-sy.id]);
		}
		else
		{
			/*
			 *	Absolute symbol
			 */
			assert ("Absolute Symbol!");
			s->xsy_sect = NULL;
		}
		s->xsy_offset = sy.longword;
		/*
		 *	If in a section, add in
		 *	displacement from start.
		 */
		if (s->xsy_sect != NULL)
		{
			s->xsy_offset += s->xsy_sect->sec_oldlen;
		}
		s->xsy_defd |= XSYMBOL_DEFINED | XSYMBOL_STATIC;
		s->xsy_mod = NameSpace_Use(curr_mod);
	}
	nxsy();
	DBG_EXIT(("XDEF_DIR", 0x81, ""));
	return;
}


/*======================================================== DEFINE_DIR */
PRIVATE
void	define_dir (void)
/*		~~~~~~~~~~
 *	Define a name ?
 *
 *	If necessary try and extend the space used
 *	for string id's for the current module.
 *--------------------------------------------------------------------*/
{
	int 	id;
	int 	 increment;
	char *** ptable;
	size_t * psize;

	DBG_ENTER(("DEFINE_DIR", 0x81, "id=%d, name='%s'", sy.id, sy.string));

	/*
	 *	Work out which table to check against
	 */
	if (sy.id > 0)
	{
		id = sy.id;
		increment = PDEF_INCR;
		ptable = &pdef_name;
		psize = &pdef_size;
	}
	else
	{
		id = - sy.id;
		increment = NDEF_INCR;
		ptable = &ndef_name;
		psize = &ndef_size;
	}
	/*------------------------------------------
	 *	Check table has room for another entry,
	 *	and if not go about expanding it.
	 *-----------------------------------------*/
	if ((size_t)id >= *psize)
	{
		*psize += increment;
		DBG(("DEFINE_DIR", 0x81, "Increasing table size from %d to %d entries", \
									*psize - increment, *psize));
		*ptable = realloc(*ptable, (*psize) * sizeof(char *));
		if (*ptable == NULL)
		{
			Halt (ERR_NOMEMORY);
		}
		/*--------------------------
		 *	Clear down new entries
		 *-------------------------*/
		while (increment > 0)
		{
			(*ptable)[(*psize) - increment--] = NULL;
		}
		DBG(("DEFINE_DIR", 0x81, ".. OK"));
	}
	/*-------------------------------------
	 *	Set up the entry to use
	 *	(first release any existing entry)
	 *-------------------------------------*/
	if (NULL != (*ptable)[id])
	{
		LIST_NameSpace_Free (&(*ptable)[id]);
	}
	(*ptable)[id] = NameSpace_Use (sy.string);
	DBG(("DEFINE_DIR", 0x88, "read next symbol"));
	nxsy();
	DBG_EXIT(("DEFINE_DIR", 0x81, ""));
	return;
}


/*======================================================== SEC_COM_DIR */
PRIVATE
void	sec_com_dir (void)
/*		~~~~~~~~~~~
 * Deal with a new section (or common area)
 *---------------------------------------------------------------------*/
{
	DBG_ENTER(("SEC_COM_DIR", 0x101, "id=%d, name=%s", \
								sy.id, ndef_name[-sy.id]));
	if (sy.id >= 0)
	{
		Halt(ERR_SECTIONID, sy.id, "New Section");
	}
	if ((size_t)(-sy.id) >= ndef_size)
	{
		Halt(ERR_TABLE, "Section Names");
	}
	curr_sec = Section_Check (ndef_name[-sy.id]);
	DBG_EXIT(("SEC_COM_DIR", 0x101, ""));
	return;
}


/*=========================================================== SECT_DIR */
PRIVATE
void	sect_dir (void)
/*		~~~~~~~~
 * Section directive
 *---------------------------------------------------------------------*/
{
	DBG_ENTER(("SECT_DIR",0x11,""));

	sec_com_dir();
	nxsy();

	DBG_EXIT(("SECT_DIR",0x11,""));
	return;
}


/*============================================================ ORG_DIR */
PRIVATE
void org_dir(void)
/*
 * ORG directive (fail)
 *---------------------------------------------------------------------*/
{
	DBG_ENTER(("ORG_DIR", 0x11, ""));
	Halt(ERR_ORGFOUND);
	nxsy();
	DBG_EXIT(("ORG_DIR", 0x11, ""));
	return;
}

/*========================================================== COMM_DIR */
PRIVATE
void comm_dir(void)
/*
 * Common directive
 *--------------------------------------------------------------------*/
{
   DBG_ENTER(("COMM_DIR",0x11,""));

   sec_com_dir();
   nxsy();

   DBG_EXIT(("DCOMM_DIR",0x11,""));
   return;
}


/*============================================================ DATA_DIR */

void data_dir(void)
/*
 * Data directive
 *---------------------------------------------------------------------*/
{
	DBG_ENTER(("DATA_DIR",0x11,"length=%d", sy.length));

	Section_Add_Code (curr_sec, sy.string, sy.length);
	nxsy();
	DBG_EXIT(("DATA_DIR",0x11,""));
	return;
}


/*============================================================= OFFSET_DIR */

void	offset_dir (void)
/*		~~~~~~~~~~
 * Offset directive
 *-------------------------------------------------------------------------*/
{
	DBG_ENTER(("OFFSET_DIR",0x11,"(current offset=%ld)", curr_sec->sec_offset));

	curr_sec->sec_offset += sy.longword;
	nxsy();

	DBG_EXIT(("OFFSET_DIR",0x11,"(new offset=%ld)", curr_sec->sec_offset));
}


/*============================================================ XREF_DIR_OP */
PRIVATE
long	Xref_Dir_Op  (node_t node, va_list ap)
/*		~~~~~~~~~~~
 *-------------------------------------------------------------------------*/
#define sy_oper ((OPER *)node)
{
	XREF *	   xref;
	XSYMBOL *	xsy;
	SECTION *	sec;
	short		xid;
	XOPER * 	xoper;

	DBG_ENTER(("XREF_DIR_OP", 0x4001, ""));
	xref = va_arg(ap, XREF *);

	xoper = LIST_NewNode (xref->xref_oper);
	if (NULL == xoper)
	{
		Halt (ERR_NOMEMORY);
	}
	xoper->xop_oper = sy_oper->op;
	xid = sy_oper->id;
	(void) LIST_Append(xref->xref_oper, xoper);
	switch(xoper->xop_optyp = sgn(xid))
	{
		case OPTYPE_SECTION:
				 if ((size_t)(-xid) >= ndef_size)
				 {
					 Halt(ERR_TABLE, "XREF section name");
				 }
				 sec = Section_Check (ndef_name[-xid]);
				 xoper->xop_ptr.xop_sec = sec;
				 if (xoper->xop_oper == '+')
				 {
					 xref->xref_abs += sec->sec_oldlen;
				 }
				 else
				 {
					xref->xref_abs -= sec->sec_oldlen;
				 }
				 DBG(("XREF_DIR_OP",0x208," '%c' Section '%s'+%ld", \
											xoper->xop_oper, \
											ndef_name[-xid], \
											xref->xref_abs));
				 break;

		case  OPTYPE_PCOUNTER :
				break;

		case  OPTYPE_SYMBOL :
				if ((size_t)xid >= pdef_size)
				{
					Halt(ERR_TABLE, "XREF symbol names");
				}
				xsy = Symbol_Check(pdef_name[xid]);
				xsy->xsy_defd |= XSYMBOL_USED;
				xoper->xop_ptr.xop_sym = xsy;
				DBG(("XREF_DIR_OP",0x208," '%c' Symbol '%s'", \
											xoper->xop_oper, \
											pdef_name[xid]));
				/*
				 *	We also need to build up a list
				 *	against a symbol for the case
				 *	where this might end up being
				 *	a reference to a RLL library.
				 */
				if (rll_flag)
				{
					RLL_XOPER * rll_xoper;

					if (NULL == xsy->xsy_rllxref)
					{
						xsy->xsy_rllxref = LIST_Clone (rll_xoper_master);
						if (NULL == xsy->xsy_rllxref)
						{
							Halt (ERR_NOMEMORY);
						}
					}
					DBG(("XREF_DIR_OP", 0x808, "New RLL_XOPER node added"));
					rll_xoper = LIST_NewNode (xsy->xsy_rllxref);
					rll_xoper->rll_xref  = xref;
					rll_xoper->rll_op	 = xoper->xop_oper;
					rll_xoper->rll_sec	 = curr_sec;
					(void)LIST_Append (xsy->xsy_rllxref, rll_xoper);
				}
				break;
	}
	DBG_EXIT(("XREF_DIR_OP", 0x4001, ""));
	return 0;
}
#undef sy_oper


/*=============================================================== XREF_DIR */
PRIVATE
void	xref_dir (void)
/*		~~~~~~~~
 * XREF directive
 *-------------------------------------------------------------------------*/
{
	XREF	 *x;


	DBG_ENTER(("XREF_DIR", 0x201, "section %s xref_pos=%ld ($%p)", \
							curr_sec->sec_name, \
							curr_sec->sec_offset - curr_sec->sec_start, \
							curr_sec->sec_offset - curr_sec->sec_start));

	if (curr_sec == NULL)
	{
		Halt(ERR_BINFILE);
	}

	x = LIST_NewNode (curr_sec->sec_xref);
	if (NULL == x)
	{
		Halt(ERR_NOMEMORY);
	}

	x->xref_pos    = curr_sec->sec_offset - curr_sec->sec_start;
	x->xref_abs    = sy.longword;
	x->xref_trunc  = sy.trunc_rule;
	x->xref_module = NameSpace_Use (curr_mod);
	x->xref_oper = LIST_Clone (xoper_master);
	if (NULL == x->xref_oper)
	{
		Halt (ERR_NOMEMORY);
	}
	(void) LIST_Append(curr_sec->sec_xref, x);
	Section_Add_Code(curr_sec, "\0\0\0\0",
					 sy.trunc_rule & (TRUNC_BYTE | TRUNC_WORD | TRUNC_LONG));


#if 0
	DBG(("XREF_DIR", 0x208, "Now add ops (LIST_Count(sy.xref_oper) = %ld)", \
									LIST_Count(sy.xref_oper)));
#endif
	assert (LIST_Count(sy.xref_oper) > 0);

	(void) LIST_Enumerate (sy.xref_oper, Xref_Dir_Op, x);

	nxsy();
	DBG_EXIT(("XREF_DIR", 0x201, ""));
	return;
}



/*======================================================== HEADER_COMMAND */
PRIVATE
void	header_command (void)
/*		~~~~~~~~~~~~~~
 *	Deal with header commands
 *------------------------------------------------------------------------*/
{
	int   in_header_com;
   
	DBG_ENTER(("HEADER_COMMAND",0x41, "module=%s", curr_mod));
	for (in_header_com = 1 ; in_header_com != 0 ; )
	{
		switch (sy.directive)
		{
		 case DIRECTIVE_COMMENT :
						comment_dir();
						break;
		 case DIRECTIVE_XDEF :
						xdef_dir();
						break;
		 case DIRECTIVE_DEFINE	:
						define_dir();
						break;
		 default	  :
						in_header_com=0;
						break;
		}
	}
	DBG_EXIT(("HEADER_COMMAND", 0X41, "module=%s", curr_mod));
	return;
}


/*======================================================== SECT_COMMAND */
PRIVATE
void	sect_command (void)
/*		~~~~~~~~~~~~
 * Deal with section commands
 *----------------------------------------------------------------------*/
{
	DBG_ENTER(("SECT_COMMAND", 0x41, "module=%s", curr_mod));
	switch (sy.directive)
	{
	  case DIRECTIVE_SECTION :
					 sect_dir();
					 break;
	  case DIRECTIVE_ORG :
					 org_dir();
					 break;
	  case DIRECTIVE_COMMON :
					 comm_dir();
					 break;
	  default	   : Halt(ERR_BINFILE);
					 break;
	}
	DBG_EXIT(("SECT_COMMAND", 0x41, "module=%s", curr_mod));
}


/*============================================================ BODY */
PRIVATE
void	body (void)
/*		~~~~
 * Deal with body commands (after section)
 *------------------------------------------------------------------*/
{
	BOOL	body_flag;

	DBG_ENTER(("BODY", 0X41, "module=%s", curr_mod));
	for (body_flag = 1 ; body_flag != 0 ; )
	{
		switch(sy.directive)
		{
			case DIRECTIVE_DATA    :
						data_dir();
						break;
			case DIRECTIVE_OFFSET :
						offset_dir();
						break;
			case DIRECTIVE_XDEF    :
						xdef_dir();
						break;
			case DIRECTIVE_XREF :
						xref_dir();
						break;
			case DIRECTIVE_DEFINE :
						define_dir();
						break;
			case DIRECTIVE_COMMENT :
						comment_dir();
						break;
			default :
						body_flag = 0;
						break;
		}
	}
	DBG_EXIT(("BODY", 0x41, "module=%s", curr_mod));
	return;
}


/*========================================================== CHUNK */
PRIVATE
void	chunk (void)
/*		~~~~~
 * Deal with a header/body chunk
 *-----------------------------------------------------------------*/
{
	BOOL	header_flag;

	DBG_ENTER(("CHUNK",0x31,"module=%s", curr_mod));
	for (header_flag = 1 ; header_flag != 0 ; )
	{
		switch (sy.directive)
		{
			case DIRECTIVE_XDEF:
			case DIRECTIVE_COMMENT:
			case DIRECTIVE_DEFINE:
					header_command();
					break;
			default:
					header_flag = 0;
					break;
		}
	}
	switch (sy.directive)
	{
		default:
				return;
		case DIRECTIVE_SECTION:
		case DIRECTIVE_ORG :
		case DIRECTIVE_COMMON :
				break;
	}
	sect_command();
	body();
	DBG_EXIT(("CHUNK", 0x31, "module=%s", curr_mod));
	return;
}


/*============================================================= MODULE */
PRIVATE
void	module (void)
/*		~~~~~~
 * Deal with a new module
 *---------------------------------------------------------------------*/
{
	BOOL	chunk_flag;

	DBG_ENTER(("MODULE", 0x11, "module='%s'", sy.string));
	if (sy.directive != DIRECTIVE_SOURCE)
	{
		Halt(ERR_BINFILE);
	}
	(void) NameList_Append(&mod_liste,sy.string, 0);
	curr_mod = NameSpace_Use (sy.string);
	DBG(("MODULE", 0x18, "...Name='%s'",curr_mod));
	module_name = NameSpace_Use(sy.string);
	nxsy();

	Section_Start();

	for (chunk_flag = 1 ; chunk_flag != 0 ; )
	{
		switch (sy.directive)
		{
			case DIRECTIVE_XDEF :
			case DIRECTIVE_COMMENT :
			case DIRECTIVE_DEFINE :
			case DIRECTIVE_SECTION :
			case DIRECTIVE_ORG :
			case DIRECTIVE_COMMON :
					chunk();
					break;

			case DIRECTIVE_END:
					DBG(("MODULE", 0x11, "END Directive found"));
					chunk_flag = 0;
					break;

			default:
					Halt(ERR_BINFILE);
		}
	}

	Section_End();

	List_Module_Details();

	module_name = NameSpace_Use("NO MODULE");
	nxsy();
	DBG_EXIT(("MODULE", 0x11, "(%s)", curr_mod));
	return;
}

/*=============================================== MODULE_BUFFER_ADD */
PRIVATE
int 	Module_Buffer_Add (size_t length)
/*		~~~~~~~~~~~~~~~~~
 *	Read in the specified number of btes and add them
 *	to the current module_buffer.  Update 'module_end'
 *	to the new offset within the buffer.
 *
 *	If necessary the buffer will be extended.
 *
 *	Returns the value of the last byte read.
 *	This is useful in cases where the calling
 *	code wants to act on its value.
 *------------------------------------------------------------------*/
{
	int 	c;
	char *	ptr;

	/*-------------------------------
	 *	First work out if we need to
	 *	extend the buffer to handle
	 *	the current module.
	 *------------------------------*/
	if (module_end + length >= module_top)
	{
#define MODULE_INCREMENT 1024
		DBG(("MODULE_BUFFER_ADD", 0x01, "Increasing buffer size from %ld to %ld", \
										module_end, \
										(long)(module_end + MODULE_INCREMENT)));
		module_buffer = realloc (module_buffer, module_top + MODULE_INCREMENT);
		if (NULL == module_buffer)
		{
			Halt (ERR_NOMEMORY);
		}
		module_top += MODULE_INCREMENT;
#undef MODULE_INCREMENT
	}
	/*----------------------------------
	 *	Now read in the bytes required
	 *---------------------------------*/
	for (ptr = (char *) (&module_buffer[module_end]) ; length; length--, module_end++)
	{
		c = get_drct();
		*ptr++ = c;
	}
	return c;
}


/*===================================================== TEST_MODULE */
PRIVATE
int 	test_module(void)
/*		~~~~~~~~~~~
 *	Tests a library module to see if it is needed.
 *	Exits as soons as it is known it is needed.
 *	Module is cached in memory for faster access later.
 *------------------------------------------------------------------*/
{
	char *		name;
	int   c;
	int 		   end_test;
	int 		   result;
	long	p;
	XSYMBOL 	   *s;
   
	DBG_ENTER(("TEST_MODULE", 0x01, "(buffer size=%ld)", module_top));
	module_end = module_ptr = 0;

	/*----------------------------------------
	 *	Check that we really are at the start
	 *	of a module when we enter.
	 *---------------------------------------*/
	if (sy.directive != DIRECTIVE_SOURCE)
	{
		Halt (ERR_BINFILE);
	}
	else
	{
		name = NameSpace_Use (sy.string);
	}
	DBG(("TEST_MODULE",0x08,"... SOURCE='%s'",sy.string));

	for (end_test = 0, result = 0 ; 0 == end_test ; )
	{
		c = Module_Buffer_Add (1);
		if (c < 0)
		{
		   Halt(ERR_BINFILE);
		}
		/*
		 *	Only need to do special
		 *	processing on directives
		 */
		if (c == SROFF_FLAG)
		{
			switch (Module_Buffer_Add(1))
			{
			int i;

			case  EOF :
						Halt(ERR_BINFILE);
						break;

			case SROFF_FLAG :
						break;

			case SROFF_SOURCE :
						Halt (ERR_BINFILE);

			case SROFF_COMMENT :
						i = Module_Buffer_Add(1);
						(void)Module_Buffer_Add(i);
						break;

			case SROFF_ORG :
			case SROFF_OFFSET :
						(void)Module_Buffer_Add(4);
						break;

			case SROFF_SECTION :
			case SROFF_COMMON :
						(void)Module_Buffer_Add(2);
						break;

			case SROFF_XDEF :
						i = Module_Buffer_Add(1);
						p = module_end;
						if (i > sizeof(sy.string) - 1)
						{
							Halt (ERR_BINFILE);
						}
						(void)Module_Buffer_Add(i);
						(void)memcpy (sy.string, &module_buffer[p], i);
						sy.string[i] = '\0';
						sy.id = 0;
						Symbol_Case(case_flag);
						DBG(("TEST_MODULE",0x1008," checking symbol '%s'", sy.string));
						if (NULL != (s = Symbol_Find(sy.string)))
						{
							if (0 == (s->xsy_defd & XSYMBOL_DEFINED))
							{
								DBG(("TEST_MODULE",0x1008," .. needed!"));
								end_test = result = 1;
							}
							else
							{
								DBG(("TEST_MODULE",0x1008," .. already defined"));
							}
						}
						else
						{
							DBG(("TEST_MODULE",0x1008," .. not needed"));
						}
						(void) Module_Buffer_Add (6);
						break;

			case SROFF_XREF :
						(void)Module_Buffer_Add (5);
						while (1)
						{
						   c = Module_Buffer_Add (1);
						   if (c == SROFF_FLAG || c == EOF)
						   {
							   break;
						   }
						   (void) Module_Buffer_Add (2);
						}
						break;

			case SROFF_DEFINE :
						i = Module_Buffer_Add (3);
						(void) Module_Buffer_Add (i);
						break;

			case SROFF_END :
						end_test=1;
						break;
			default   :
						Halt(ERR_BINFILE);
			}
		}
	} /* End of for loop */

	if (0 != result)
	{
		sy.directive = DIRECTIVE_SOURCE;
		(void) strcpy (sy.string, name);
		module_ptr = 0;
	}
	LIST_NameSpace_Free (&name);
	DBG_EXIT(("TEST_MODULE", 0x01, "%d", result));
	return(result);
}


/*=========================================================== LINK_FILE */
PRIVATE
long	Link_File	(node_t filenode, va_list ap)
/*		~~~~~~~~~
 *	ŸRoutine called in to link in an individual file.
 *	The 'filenode' parameter will be the address of a NAMELIST
 *	structure which defines the file.
 *
 *	For library paths, the path list is also a list of NAMELIST nodes.
 *----------------------------------------------------------------------*/
{
	DBG_ENTER(("LINK_FILE", 0x01, "filenode=$%p (name='%s')", \
							filenode,((NAMELIST *)filenode)->name));
	(void)ap;
	(void)strcpy(currentname, ((NAMELIST *)filenode)->name);
	inp_hnd = -1;

	/*---------------------------------------
	 *	As a small (potential) optimisation
	 *	for library files we check that we
	 *	actually have outstanding undefined
	 *	symbols.
	 *--------------------------------------*/
	if (((NAMELIST *)filenode)->type & (LIBSROFF_FILE | LIBRLL_FILE))
	{
		if (0 == Symbol_AllDefined())
		{
			DBG_EXIT(("LINK_FILE", 0x01, "Exit early - no undefined symbols at present"));
			return 0;
		}
	}
	/*-------------------------------------------------
	 *	For library files we search the library paths
	 *-----------------------------------------------*/
	if (((NAMELIST *)filenode)->type & LIBRARY_PATH)
	{
		char fname[MAXNAMELEN];
		node_t	pathnode;

		DBG(("LINK_FILE", 0x18, "appears to be on library path"));
		/* Use library search paths for file */
		for (pathnode=LIST_First(libpaths) ;
			pathnode != NULL ;
				pathnode = LIST_Next(libpaths, pathnode))
		{
			(void)strcpy( fname, ((NAMELIST *)pathnode)->name);
			(void)strcat( fname, ((NAMELIST *)filenode)->name);
			DBG(("LINK_FILE", 0x18, "... trying '%s'", fname));
			if((inp_hnd = open(fname, O_RDONLY|O_BINARY, 0)) >=0)
			{
				DBG(("LINK_FILE", 0x18, "OK - inp_hnd=%d", inp_hnd));
				break;
			}
			DBG(("LINK_FILE", 0x18, "failed (inp_+hnd=%d", inp_hnd));
		}
	}
	/*-----------------------------------------------
	 *	If it was not a libary file, or if it was
	 *	a libary file but not on the path then
	 *	try and open without adding a path.
	 *	(NOTE.	Should we really try this BEFORE
	 *			searching paths for library files?)
	 *----------------------------------------------*/
	if (inp_hnd < 0)
	{
		DBG(("LINK_FILE", 0x18, "... now trying '%s'", ((NAMELIST *)filenode)->name));
		inp_hnd = open(((NAMELIST *)filenode)->name,O_RDONLY|O_BINARY, 0);
	}

	if (inp_hnd < 0)
	{
		if (((NAMELIST*)filenode)->type & OPTIONAL_FILE)
		{
			DBG_EXIT(("LINKFILE", 0x11, ".. file not found - but optional"));
			return 0;
		}
		DBG(("LINK_FILE", 0x18, "failed (inp_+hnd=%d", inp_hnd));
		Halt(ERR_OPENFILE,((NAMELIST *)filenode)->name);
	}

#ifdef RLL_SUPPORT
	if (((NAMELIST *)filenode)->type & LIBRLL_FILE)
	{
		Link_RLL(filenode);
	}
	else
#endif /* RLL_SUPPORT */
	{
		/*----------------------------------------------
		 *	Clear any waiting data in the input buffer
		 *---------------------------------------------*/
		buf_ptr = buf_end = inp_buf + BLEN;
		module_name = NameSpace_Use("*NO MODULE");
		nxsy();

		/*
		 *	 Now work through all modules
		 *	 in the current file.  There
		 *	 should only normally be more
		 *	 than one for a library file.
		 */
		while (sy.directive != DIRECTIVE_EOF)
		{
			if (((NAMELIST*)filenode)->type & LIBSROFF_FILE)
			{
				if	(0 == test_module())
				{
					module_end = module_ptr = 0;
					nxsy();
					continue;
				}
			}
			module();
		}
	}
	(void)close(inp_hnd);
	DBG_EXIT(("LINK_FILE", 0x01,"0"));
	return 0;
}


/*======================================================== WRITE_PROG */
PRIVATE
void	write_prog (void)
/*		~~~~~~~~~~
 *	Write out the binary program.
 *--------------------------------------------------------------------*/
{
	int dspace;

	DBG_ENTER(("WRITE_PROG", 0x01, ""));

	/*-----------------------------------------
	 *	UDATA size minus relocation info size
	 *	minimum of MIN_DATASPACE
	 *	bytes to allow for setting up stack
	 *----------------------------------------*/
	dspace = (progfile_size - reuse_sec->sec_start) & ~1;

	if (dspace < MIN_DATASPACE)
	{
		dspace = MIN_DATASPACE;
	}
	DBG(("WRITE_PROG", 0x01, "Dataspace size=%d (udata_size=%d, reuse from section %s, reuse_start=$%p, MIN_DATASPACE=%d)", \
							dspace, udata_sec->sec_length, \
							reuse_sec->sec_name, reuse_sec->sec_start, \
							MIN_DATASPACE));

#ifdef XTC68
	/*--------------------------------------
	 *	For XtC68, write out the flag plus
	 *	dataspace after the BSS data.
	 *-------------------------------------*/
	Section_Add_Code (reloc_sec, "XTcc", 4);
	Section_Add_Long (reloc_sec, dspace);
	eprintf("%s: dataspace %d (%x)\n", prgm_name, dspace, dspace);
#endif /* XTC68 */


#ifndef XTC68
	progfile_handle = open(prgm_name,O_CREAT|O_WRONLY|O_BINARY|O_TRUNC);
#else
#ifdef WIN32
	progfile_handle = open(prgm_name,O_CREAT|O_WRONLY|O_BINARY|O_TRUNC);
#else
	progfile_handle = open(prgm_name,O_CREAT|O_WRONLY|O_BINARY|O_TRUNC,
						S_IRUSR |S_IWUSR);
#endif
#endif /* XTC68 */
	if (progfile_handle < 0)
	{
		DBG(("WRITE_PROG", 0x01, "failed to open '%s' for writing", prgm_name));
		Halt(ERR_OPENWRITE, prgm_name);
	}

	Section_WriteAll();

	if (progfile_size & 1)
	{
		/* Write an extra byte of zero to even up the program file */
		(void)write(progfile_handle, "\0", 1);
	}
#ifdef QDOS
	{
	struct qdirect stat;

	/*-------------------------------------
	 *	Read the QDOS header information
	 *------------------------------------*/
	(void)fs_headr(getchid(progfile_handle),-1L, &stat, sizeof(stat));
	/*--------------------
	 *	Now write it out
	 *-------------------*/
	stat.d_type = QF_PROG_TYPE;
	stat.d_datalen = dspace;
	(void)fs_heads( getchid(progfile_handle), -1L, &stat, 14);
	DBG(("WRITE_PROG", 0x01, "Written QDOS file header"));
	}
#endif /* QDOS */

   if (close(progfile_handle)<0)
   {
		DBG(("WRITE_PROG", 0x01, "failure closing file (errno=%d)", errno));
		Halt(ERR_WRITEFILE);
   }
   DBG_EXIT(("WRITE_PROG", 0x01, ""));
}


/*============================================================= MAIN */
PUBLIC
int 	main (int	argc, char	**argv)
/*		~~~~
 *-------------------------------------------------------------------*/
{
#ifdef QDOS
	int 	eargc;
	char ** eargv;
	char *	env;
#endif

	/*--------------------------------------------
	 *	Process parameters.
	 *
	 *	We do any in environemnt variables
	 *	followed by those on the command line
	 *-------------------------------------------*/
#ifdef QDOS
	env = getenv("LD_OPTS");
	if(NULL != env)
	{
		if (-1 != argunpack(env, &eargv, &eargc, cmdexpand))
		{
			Command_Line (eargc, eargv);
		}
	}
#endif
	Command_Line(argc, argv);

	/*---------------------------------
	 *	Initialise memory structures
	 *--------------------------------*/

	xref_master = LIST_Create (LIST_CLASS_FIFO,
							   sizeof (XREF),
							   NULL,
							   NULL,
							   NULL);

	xoper_master = LIST_Create (LIST_CLASS_FIFO,
								sizeof (XOPER),
								NULL,
								NULL,
								NULL);

#ifdef RLL_SUPPORT
	rlib_list = LIST_Create (LIST_CLASS_FIFO,
								sizeof (RLL_ENTRY),
								NULL,
								NULL,
								NULL);

	rll_xoper_master = LIST_Create (LIST_CLASS_FIFO,
								sizeof (RLL_XOPER),
								NULL,
								NULL,
								NULL);
#endif /* RLL_SUPPORT */

	part_master = LIST_Create ( LIST_CLASS_FIFO,
								sizeof (SECPART),
								NULL,
								NULL,
								NULL);

	sy.xref_oper = LIST_Create (LIST_CLASS_FIFO,
								sizeof (OPER),
								NULL,
								NULL,
								NULL);

	if (NULL == xref_master
	||	NULL == xoper_master
#ifdef RLL_SUPPORT
	||	NULL == rlib_list
	||	NULL == rll_xoper_master
#endif * RLL_SUPPORT */
	||	NULL == part_master
	||	NULL == sy.xref_oper)
	{
		 Halt(ERR_NOMEMORY);
	}
	   

	Init_Listing ();
	Section_Init ();
	Symbol_Init ();

#ifdef RLL_SUPPORT
	switch (format_flag)
	{
		case FORMAT_RLL:
						RLL_Header();
						break;

		default:
#ifdef LIBDEBUG
						DBG(("MAIN", 0x18, "Unexpected value for format_flag of %d", \
													format_flag));
						break;

		case FORMAT_GST:
		case FORMAT_LD1:
		case FORMAT_LD2:
#endif /* LIBDEBUG */
						break;
	}
#endif /* RLL_SUPPORT */

	/*----------------------------------------
	 *	Now work through files to be linked
	 *---------------------------------------*/

	DBG(("MAIN", 0x08, "Now work through all the files/libraries to be linked"));

	(void)LIST_Enumerate (linkfiles, Link_File);

	DBG(("MAIN", 0x08, "Finished linking all required files/libraries"));

	/*-------------------------
	 *	Create the BSS areas
	 *------------------------*/
	BSS_Area();

	DBG(("MAIN", 0x08, "Finished creating BSS areas"));

	/*-------------------------
	 *	Now resolve all XREF's
	 *------------------------*/

	Section_AllXrefs();

	/*
	 *	Terminate the current relocation stream
	 */


#ifdef RLL_SUPPORT
	switch (format_flag)
	{
		case FORMAT_RLL:

						RLL_Header_Fix();
						break;

		default:
#ifdef LIBDEBUG
						DBG(("MAIN", 0x18, "Unexpected valud for format_flag of %d", \
													format_flag));
						break;

		case FORMAT_GST:
		case FORMAT_LD1:
		case FORMAT_LD2:
#endif /* LIBDEBUG */
						break;
	}
#endif /* RLL_SUPPORT */


	if (prgm_flag)
	{
		write_prog();
	}

	return Listings();

}
