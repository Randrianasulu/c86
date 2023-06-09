/*************************************************************************
 *
 *	m a k e :	m a i n . c
 *
 *========================================================================
 * Edition history
 *
 *	#	 Date						  Comments						 By
 * --- -------- ---------------------------------------------------- ---
 *	 1	  ??														 ??
 *	 2 01.07.89 strcmp(makefile,..) only if makefile a valid ptr.	 RAL
 *	 3 23.08.89 initname() added									 RAL
 *	 4 30.08.89 argument parsing impr., indention ch., macro fl. add.PSH,RAL
 *	 5 03.09.89 k-option added, initname -> init changed			 RAL
 *	 6 06.09.89 environment, MAKEFLAGS, e,d,a options added,		 RAL
 *	 7 09.09.89 tos support added, fatal args added, fopen makefile  PHH,RAL
 *	 8 17.09.89 setoptions fixed for __STDC__						 RAL
 * ------------ Version 2.0 released ------------------------------- RAL
 *	 9 01.05.91 QDOS support added									 DJW
 *	10 18.02.96 Fixed problem with _s and _x default rules				 DJW
 *
 *************************************************************************/

/*
 *	make:
 *
 *	-a try to guess undefined ambiguous macros (*,<)
 *	-d print debugging info
 *	-e environment macro def. overwrite makefile def.
 *	-f makefile name
 *	-i ignore exit status
 *	-k continue on errors
 *	-n pretend to make
 *	-p print all macros & targets
 *	-q question up-to-dateness of target.  Return exit status 1 if not
 *	-r don't not use inbuilt rules
 *	-s make silently
 *	-t touch files instead of making them
 *	-m Change memory requirements (EON only)
 */

#define EXTERN
#define INIT(x) = x
#define INITARRAY
#include "make.h"

char _version[]= "2.0h";

#ifdef QDOS
void (*_consetup)() = consetup_title;
char _prog_name[] = "Make";
long _stackmargin = 1024L;
#include <fcntl.h>
int (*_Open)(const char *,int,...) = qopen;
#endif /* QDOS */

static char *makefile;		/*	The make file  */
static FILE *ifd;			/*	Input file desciptor  */
static char *ptrmakeflags;
static char  makeflags[] = "MAKEFLAGS=                    ";
   /* there must be enough 'space' for all possible flags ! */

#ifndef GENERIC
#include <sys/stat.h>
 static char *RCSname = (char *)0;

FILE *mopen (fname, mode)
  char *fname;
  char *mode;
{
  FILE *fd;
  struct stat st;
  char buf[256];

  if ((fd = fopen(fname,mode)) != (FILE *)0)
	return fd;

  RCSname = malloc((size_t)strlen(fname+7));

  strcpy(RCSname,"RCS/");
  strcat(RCSname,fname);
  strcat(RCSname,",v");
  if (stat(RCSname,&st) != 0) {
	strcpy(RCSname,&RCSname[4]);
	if (stat(RCSname,&st) != 0) {
		RCSname = (char *) 0;
		return fd;
	}
  }

#ifdef QDOS
  execl ("co", -1L, "co","-q",fname, NULL);
#else
  strcpy(buf,"co -q ");
  strcat(buf,fname);
  system(buf);
#endif
  makefile = fname;

  return fopen(fname,mode);
}

#endif

void main(argc, argv)
int    argc;
char **argv;
{
  register char 	   *p;		/*	For argument processing  */
  int					estat = 0;	/*	For question  */
  register struct name *np;
  struct macro		   *mp;
  int					targc;		/* temporary for multiple scans */
  char				  **targv;
  char				  **nargv;		/* for removing items from argv */
  char				  **envp;	   /* enivironment ptr */


  ptrmakeflags = &makeflags[10];
  myname = (argc-- < 1) ? "make" : *argv++;
#ifdef tos
  myname = "Make";
#endif

  targc = argc;
  targv = nargv = argv;
  while (targc--) {
	if((p = strchr(*targv, '=')) != (char *)NULL) {
		*p = '\0';
		mp = setmacro(*targv, p + 1);
		mp->m_flag |= M_OVERRIDE;
		--argc;
	} else
		*nargv++ = *targv;

	++targv;
  }

  targc = argc;
  targv = nargv = argv;
  while (targc--) {
	if (**targv == '-') {
		--argc;
		p = *targv++;
		while (*++p != '\0') {
			switch(mylower(*p)) {
			case 'f':	/*	Alternate file name  */
				if (*++p == '\0') {
					--argc;
					if (targc-- == 0)
						usage();
					p = *targv++;
				}
				makefile = p;
				goto end_of_args;
#ifdef eon
			case 'm':	/*	Change space requirements  */
				if (*++p == '\0') {
					--argc;
					if (targc-- <= 0)
						usage();
					p = *targv++;
				}
				memspace = atoi(p);
				goto end_of_args;
#endif
			default :
				setoption(*p);
				break;
			}
		}
	end_of_args:;
	} else
		*nargv++ = *targv++;
  }

  /* evaluate and update environment MAKEFLAGS */
  if((p =getenv("MAKEFLAGS")) != (char *)0)
	while(*p) setoption(*p++);
  for( p = ptrmakeflags; !isspace((int)*p); p++) ;
  *p = '\0';
  putenv(makeflags);


#ifdef eon
  if (initalloc(memspace) == 0xffff)  /*  Must get memory for alloc  */
	fatal("Cannot initalloc memory",(char *)0,0);
#endif

  if (makefile && strcmp(makefile, "-") == 0)  /*	use stdin as makefile  */
	ifd = stdin;
  else if (!makefile) {    /*  If no file, then use default */
#ifdef GENERIC
	if ((ifd = fopen(DEFN1, "r")) == (FILE *)0) {
		if (errno != MNOENT || !DEFN2)
			fatal("Can't open %s; error %02x", DEFN1, errno);
		else if ((ifd = fopen(DEFN2, "r")) == (FILE *)0)
			fatal("Can't open %s; error %02x", DEFN2, errno);
	}
  }
  else if ((ifd = fopen(makefile, "r")) == (FILE *)0)
#else
	if ((ifd = mopen(DEFN1, "r")) == (FILE *)0) {
		ifd = mopen(DEFN2, "r");
	}
  }
  else if ((ifd = mopen(makefile, "r")) == (FILE *)0)
#endif
	fatal("Can't open %s; error %2x", makefile, errno);

  init();

  makerules();

  mp = setmacro("MAKE", myname);
  mp->m_flag |= M_MAKE;
  setmacro("$", "$");

  /* set environment macros */
  envp = environ; /* get actual environment ptr. */
  while (*envp) {
	if((p = strchr(*envp, '=')) != (char *)NULL) {
		*p = '\0';
		mp = setmacro(*envp, p + 1);
		*p = '=';
		if (useenv) mp->m_flag |= M_OVERRIDE;
	} else
		fatal("invalid environment: %s",*envp,0);

	++envp;
  }

#ifdef GENERIC
  input(ifd);	/*	Input all the gunga  */
  fclose(ifd);	/*	Finished with makefile	*/
#else
  if (ifd){
	input(ifd); /*	Input all the gunga  */
	fclose(ifd);	/*	Finished with makefile	*/
  }
  if (RCSname) {
	unlink(makefile);
	RCSname = (char *) 0;
  }
#endif

  lineno = 0;	/*	Any calls to error now print no line number */

  if (print)
	prt();	/*	Print out structures  */

  np = newname(".SILENT");
  if (np->n_flag & N_TARG)	silent = TRUE;

  np = newname(".IGNORE");
  if (np->n_flag & N_TARG)	ignore = TRUE;

  precious();

#ifdef GENERIC
  if (!firstname)
#else
  if (!firstname && argc == 0)
#endif
	fatal("No targets defined",(char *)0,0);

  circh();	/*	Check circles in target definitions  */

  if (!argc)
	estat = make(firstname, 0);
  else {
	while (argc--) {
		estat |= make(newname(*argv++), 0);
	}
  }

	exit(quest ? estat : 0);
}

#ifdef __STDC__
void setoption(char option)
#else
void setoption(option)
char option;
#endif
{
  register char *c;

  option = mylower(option);
  switch(option) {
	case 'n':	/*	Pretend mode  */
		domake = FALSE;
		break;
	case 'i':	/*	Ignore fault mode  */
		ignore = TRUE;
		break;
	case 'k':	/*	Continue on errror	*/
		conterr = TRUE;
		break;
	case 's':	/*	Silent about commands  */
		silent = TRUE;
		break;
	case 'p':
		print = TRUE;
		break;
	case 'r':
		rules = FALSE;
		break;
	case 't':
		dotouch = TRUE;
		break;
	case 'q':
		quest = TRUE;
		break;
	case 'e':
		useenv = TRUE;
		break;
	case 'd':
		dbginfo = TRUE;
		break;
	case 'a':
	ambigmac = TRUE;
		break;
	default:	/*	Wrong option  */
		usage();
  }
  for( c = ptrmakeflags; !isspace((int)*c); c++)
	if ( *c == option) return;
  *c = option;
}

void usage()
{
  fprintf(stderr, "Syntax: %s [{options | macro=val | target}]\n", myname);
  fprintf(stderr, "Function: maintaining computer programs      V%s\n",_version);
  fprintf(stderr, "Options : -a : try to guess undefined ambiguous macros (*,<)\n");
  fprintf(stderr, "          -d : print debugging information\n");
  fprintf(stderr, "          -e : environment macro def. overwrite makefile def.\n");
  fprintf(stderr, "          -f filename : makefile name (default: makefile, Makefile)\n");
  fprintf(stderr, "          -i : ignore exit status of executed commands\n");
  fprintf(stderr, "          -k : continue with unrelated branches on errors\n");
  fprintf(stderr, "          -n : pretend to make\n");
  fprintf(stderr, "          -p : print all macros & targets\n");
  fprintf(stderr, "          -q : question up-to-dateness of target\n");
  fprintf(stderr, "          -r : don't use inbuilt rules\n");
  fprintf(stderr, "          -s : make silently\n");
  fprintf(stderr, "          -t : touch files instead of making them\n");
  fprintf(stderr, "Environment: MAKEFLAGS\n");
  exit(1);
}


void fatal(msg, a1, a2)
char *msg;
char *a1;
int   a2;
{
  fprintf(stderr, "%s: ", myname);
  fprintf(stderr, msg, a1, a2);
  fputc('\n', stderr);
  exit(1);
}
