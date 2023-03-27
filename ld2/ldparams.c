/*
 *		l p a r a m s _ h
 *
 * (c) Copyright 1991-1999 David J. Walker
 *	   Email:  dave.walker@icl.com
 *
 *	   This is the parameter handling file for the LD linker
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

#include "ld.h"
#include <ctype.h>

struct OPTLIST foptions[] = {
				{"gst",       &format_flag,     FORMAT_GST, SECTION_BSS},
				{"ldv1",      &format_flag,     FORMAT_LD1, SECTION_BSS},
				{"keepbss",   &bss_flag,        TRUE, SECTION_XDEF},
				{"keepxdef",  &xdef_flag,       TRUE, SECTION_RLIB},
				{"keeprlib",  &rlib_flag,       TRUE, SECTION_XREF},
				{"keepxref",  &xref_flag,       TRUE, SECTION_RELOC},
				{"keepreloc", &reloc_flag,      TRUE, SECTION_UDATA},
				{"keepudata", &udata_flag,      TRUE, 0},
				{NULL,        NULL,             0, 0}
		};

struct OPTLIST zoptions[] = {
				{"defs",      &defs_flag,   TRUE, SECTION_XREF},
				{"nodefs",    &defs_flag,   FALSE,  0},
				{"udata",     &udata_flag,  TRUE, SECTION_UDATA},
				{"xref",      &xref_flag,   TRUE, SECTION_RELOC},
				{"xdef",      &xdef_flag,   TRUE, SECTION_XREF},
				{NULL,        NULL,         0, 0}
		};

struct OPTLIST moptions [] = {
				{"a",       &map_addr_flag,   TRUE, 0},
				{"l",       &map_lib_flag,    TRUE, 0},
				{"m",       &map_mod_flag,    TRUE, 0},
				{"s",       &map_alpha_flag,  TRUE, 0},
				{"u",       &map_unref_flag,  TRUE, 0},
				{"x",       &map_xref_flag,   TRUE, 0},
				{NULL, NULL, 0, 0}
			};


/*==================================================== CHECK_TEXT_OPTARG */
PRIVATE
void	Check_Text_Optarg ( char letter, struct OPTLIST *optptr)
/*		~~~~~~~~~~~~~~~~~
 *	Check through the list of options that are
 *	valid.	If found, then set the indicated
 *	flag to the given value.   If not, then
 *	output the approriate error message.
 *
 *	Note that there may be multiple options separated
 *	by the plus ('+') symbol.
 *-----------------------------------------------------------------------*/
{
	char *	ptrend;
	char *	ptrstart;

	DBG_ENTER(("CHECK_TEXT_OPTION", 0x101, "option='%c'", letter));

	ptrstart = optarg;
	ptrend = optarg+strlen(optarg);

	while (ptrstart < ptrend)
	{
		char *	ptrtemp;
		char	temp;

		/*
		 *	Look for '+' seperator, and if found
		 *	temporarily replace with NULL byte
		 */
		ptrtemp = strchr (ptrstart, '+');

		if (NULL == ptrtemp)
		{
			ptrtemp = ptrend;
		}
		temp = *ptrtemp;
		*ptrtemp = '\0';

		/*
		 *	Compare options with possible values
		 */
		do
		{
			if (! stricmp(optptr->Text,ptrstart))
			{
				if (0 != optptr->Value)
				{
					*(optptr->pFlag) = optptr->Value;
				}
				if (0 != optptr->ReuseId)
				{
					DBG(("CHECK_TEXT_OPTION", 0x108, "Reuse id changed from %d to %d", \
									udata_reuse_id, optptr->ReuseId));
					udata_reuse_id = optptr->ReuseId;
				}
				DBG_EXIT (("CHECK_TEXT_OPTION", 0x108, "found option '%s'", \
											optptr->Text));
				break;
			}
			optptr++;
		} while (NULL != optptr->Text);

		/*
		 *	Output an error message
		 *	if option was not matched
		 */
		if (NULL == optptr->Text)
		{
			Message (ERR_TEXTOPTION, letter, ptrstart);
		}
		/*
		 *	Get ready for next option following
		 *	the '+' symbol (if there was one).
		 */
		*ptrtemp = temp;
		ptrstart = ptrtemp + 1;
	}
	DBG_EXIT(("CHECK_TEXT_OPTION", 0x101, ""));
	return;
}



/*======================================================= COMMAND_LINE */
PUBLIC
void	Command_Line(int   argc, char  **argv)
/*		~~~~~~~~~~~~
 *---------------------------------------------------------------------*/
{
#ifdef QDOS
   extern char _prog_use[];
#endif
	NAMELIST *	pnamelist;
	char	filename[MAXNAMELEN+1];
	char *p;
	int   num_files = 0;

	DBG_ENTER(("UNIX_COMMAND_LINE", 0x11, "argc=%d, argv=$%p", argc, argv));
	DBG(("UNIX_COMMAND_LINE", 0x11, "setup default startup file '%s'", DEFAULT_STARTUP));
	(void) NameList_Append (&linkfiles, DEFAULT_STARTUP, OBJECT_FILE | LIBRARY_PATH | LIBRARY_STARTUP);
	/*
	 *	Process the command line arguments
	 */
	while (optind < argc)
	{
		switch (getopt(argc, argv,"B:b:f:L:l:M:m:No:R:r:s:uUVvZ:z:@:"))
		{

		case 'B': /* Could be buffer set */
		case 'b':
				DBG(("COMMAND_LINE",0x84,"'B' option - remainder='%s'",optarg));
				#if 0
				if (!strnicmp(optarg,"UFL",(size_t)3))	 /* Set library buffer size */
				{
					bufl_size = Get_Size(&optarg[3]);
					DBG(("COMMAND_LINE",0x84,"'BUFL' option - size = %ld",bufl_size));
					break;
				}
				if(!strnicmp(optarg,"UFP",(size_t)3))	/* Set program size */
				{
					bufp_size = Get_Size(&optarg[3]);
					DBG(("COMMAND_LINE",0x84,"'BUFP' option - size = %ld",bufp_size));
					break;
				}
				#else
				if ( 0 == strnicmp (optarg, "UFL", 3)
				||	 0 == strnicmp (optarg, "UFP", 3))
				{
					eprintf ("option %s no longer needed - ignored\n",
							  argv[optind - 1]);
					break;
				}
				#endif
				goto BADOPTION;

		case 'f': /* Output format */
				DBG(("UNIX_COMMAND_LINE",0x84,"'f' option - format type='%s'",optarg));
				if (0 == stricmp(optarg,"gst")
				||	0 == stricmp(optarg,"ldv1"))
				{
					bss_sec->sec_reuse = TRUE;
					bss_sec->sec_output = FALSE;
					udata_flag = FALSE;
					case_flag = FALSE;
					defs_flag = TRUE;
				}
				Check_Text_Optarg('f',foptions);
				break;

		case 'l': /* A new library to add */
				DBG(("UNIX_COMMAND_LINE",0x84,"'l' option - library='%s'",optarg));
#ifdef XTC68
				(void) strcpy( filename, "");
#else
				(void) strcpy( filename, "lib");
#endif
				(void) strcat( filename, optarg);
				(void) strcat( filename, EXTS "a");
				(void) NameList_Append (&linkfiles,
										filename,
										(LIBSROFF_FILE | LIBRARY_PATH));
				break;

		case 'L': /* A new library search path */
				DBG(("UNIX_COMMAND_LINE",0x84,"'L' option - path='%s'",optarg));
				(void)NameList_Append (&libpaths,
										optarg,
										LIBRARY_PATH);
				break;


		case 'm': /* MAP file wanted */
		case 'M':
				DBG(("UNIX_COMMAND_LINE",0x84,"'m' option, qualifiers '%s'", optarg));
				lstng_flag = TRUE;
				map_sum_flag = TRUE;


				/*
				 *	If just -m, then must not
				 *	consume next argument.
				 */
				if (optarg[0] == '-')
				{
					optind--;
					break;
				}

				Check_Text_Optarg('m',moptions);

				break;


		case 'N': /* No program please */
				DBG(("UNIX_COMMAND_LINE",0x84,"'N' option"));
				prgm_flag = FALSE;
				break;


		case 'o': /* Output program name */
				DBG(("UNIX_COMMAND_LINE",0x84,"'o' option - program name ='%s'",optarg));
				prgm_name = NameSpace_Use(optarg);
				break;

		case 'r': /* A new RLL library to add */
				DBG(("UNIX_COMMAND_LINE",0x84,"'r' option - library='%s'",optarg));
#ifndef RLL_SUPPORT
				goto BADOPTION;
#else
				rll_flag++;
				(void) strcpy( filename, "lib");
				(void) strcat( filename, optarg);
				(void) strcat( filename, EXTS "rls");
				(void) NameList_Append(&linkfiles,
										filename,
										LIBSROFF_FILE | OPTIONAL_FILE | LIBRARY_PATH);
				(void) strcpy( filename, "lib");
				(void) strcat( filename, optarg);
				(void) strcat( filename, EXTS "rll");
				(void) NameList_Append (&linkfiles,
										filename,
										LIBRLL_FILE | LIBRARY_PATH);
				break;

#endif /* RLL_SUPPORT */

		case 'R':	/* Inform linker that we are building RLL object */
#ifdef RLL_SUPPORT
				DBG(("UNIX_COMMAND_LINE",0x84,"'R' option"));
				format_flag =  FORMAT_RLL;
				bss_flag = TRUE;
				xdef_flag = TRUE;
				udata_reuse_id = SECTION_RLIB;
				pnamelist = LIST_First (linkfiles);
				if (0 == strcmp(pnamelist->name, DEFAULT_STARTUP))
				{
					DBG(("COMMAND_LINE",0x84,"Changing startup module to %s'",\
												DEFAULT_RLLSTART));
					LIST_NameSpace_Free (&pnamelist->name);
					pnamelist->name = DEFAULT_RLLSTART;
					pnamelist->type = OBJECT_FILE | LIBRARY_PATH;
				}
				rll_name = NameSpace_Use(optarg);
				rll_version = strchr(rll_name,'/');
				if (rll_version == NULL)
				{
					rll_version = NameSpace_Use ("1.0");
				}
				else
				{
					*rll_version = '\0';	/* Terminate name string early */
					rll_version++;			/* ... and set name to following */
					if (strlen(rll_version) != 4)
					{
						Halt (ERR_RLLVERSION);
					}
				}
				break;
#else
				goto BADOPTION;
#endif /* RLL_SUPPORT */

		case 'S': /* New startup file */
		case 's':
				DBG(("COMMAND_LINE",0x84,"'s/S' option - startup name ='%s'",
															optarg));
				/*
				 *	Get first file.
				 *	If there is a startup module this
				 *	will be it!
				 */
				pnamelist = LIST_First(linkfiles);

				/*
				 *	Insert new file at start
				 *	(unless it is special case of '-')
				 */
				if (0 != strcmp("-", optarg))
				{
					DBG(("COMMAND_LINE", 0x08, "Add new start-up module %s", optarg));
					if (NULL == LIST_NewInsert (linkfiles, optarg,
											OBJECT_FILE | LIBRARY_PATH))
					{
						Halt(ERR_NOMEMORY);
					}
				}
				/*
				 *	Remove existing startup file (if any)
				 */
				if (pnamelist->type & LIBRARY_STARTUP)
				{
					DBG(("COMMAND_LINE", 0x08, "Removve old start-up module %s", \
													pnamelist->name));
					(void)LIST_Delete(linkfiles, &pnamelist);
				}
				break;

		case 'U':
		case 'u':
				DBG(("UNIX_COMMAND_LINE",0x84,"'u' option"));
				case_flag = FALSE;
				break;

		case 'v':
				DBG(("UNIX_COMMAND_LINE",0x84,"'v/V' option"));
				verbose_flag = TRUE;
				map_sum_flag = TRUE;
				/* FALLTHRU */
		case 'V':
				name_flag = TRUE;
				break;

		case 'z':	/* Special options */
		case 'Z':
				DBG(("UNIX_COMMAND_LINE",0x84,"'z/Z' option, optarg=%s", optarg));
				Check_Text_Optarg('z',zoptions);
				break;

		case '@':  /* File list from a file */
				{
					FILE *fp;
					char fnam[80];

					if((fp = fopen (optarg, "r")) != NULL)
					{
						while(fscanf(fp, "%s", fnam) != EOF)
						{
							if(*fnam)
							{
								(void) NameList_Append (&linkfiles, fnam, OBJECT_FILE);
								num_files++;
							}
						}
						(void) fclose (fp);
					}
					else
					{
						Halt(ERR_LINKFILE,optarg);
					}
				}
				break;

		case '?' :
BADOPTION:
				Halt (ERR_BADPARAM,argv[optind]);
				/* NOTREACHED */

		default :	/* Should never get here ! */
				DBG(("UNIX_COMMAND_LINE",0x81,"default - **** how did we get here ****"));

		case EOF :
				DBG(("UNIX_COMMAND_LINE",0x81,"Add file to link '%s'",argv[optind]));
				(void) NameList_Append (&linkfiles, argv[optind], OBJECT_FILE);
				num_files++;
				optind++;
				break;
		} /* End of switch */
	}	/* End of while loop */

	if( lstng_flag )
	{
		/* Set the MAP file name to be the program name plus _MAP */
		(void)strcpy( lstng_name, prgm_name);
		(void)strcat( lstng_name, DSEPS);
		(void)strcat( lstng_name, "MAP");
	}
	if ( num_files == 0)
	{
		(void) printf("No files specified\n"); exit(-15);
	}
	/*
	 *	Add the last library path to search (_pdir_lib_)
	 */
#ifdef QDOS
	p = getenv(_prog_use);
	if (p == NULL)
	{
		p = "";
	}
	(void)strcpy( filename, p);
	(void)strcat( filename, "LIB_");
#else
	p = getenv("QLLIB");
	if (p == NULL)
	{
		p = DEFAULT_LIBPATH;
	}
	(void)strcpy(filename, p);
	p = filename + strlen(filename) - 1;
	switch (*p)
	{
		case '/':
		case '\\':
				break;
		default:
				*++p = DSEP;
				*++p = '\0';
	}
#endif
	DBG(("UNIX_COMMAND_LINE",0x81,"Add default library path '%s' to link", filename));
	(void) NameList_Append(&libpaths, filename, LIBRARY_PATH);
	/*
	 *	Default library file(s)
	 */
	DBG(("UNIX_COMMAND_LINE",0x81,"Add default library '%s' to link", DEFAULT_LIBRARY));
	switch (format_flag)
	{
		case FORMAT_RLL:
					(void) NameList_Append(&linkfiles, DEFAULT_RLS, LIBSROFF_FILE | LIBRARY_PATH | OPTIONAL_FILE);
					(void) NameList_Append(&linkfiles, DEFAULT_RLL, LIBRLL_FILE | LIBRARY_PATH | OPTIONAL_FILE);
					/* FALLTHRU */
		default:
					(void) NameList_Append(&linkfiles, DEFAULT_LIBRARY, LIBSROFF_FILE | LIBRARY_PATH);
					break;
	}
	DBG_EXIT(("UNIX_COMMAND_LINE", 0x81, ""));
	return;
}
