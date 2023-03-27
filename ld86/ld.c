/* ld.c - linker for Introl format (6809 C) object files 6809/8086/80386 */

#ifdef STDC_HEADERS_MISSING
extern int errno;
#else
#include <errno.h>
#endif

#include "const.h"
#include "byteord.h"
#include "type.h"
#include "globvar.h"

#ifdef EPOC
#include <cpoclib.h>
#endif /* EPOC */

#ifdef QDOS
#include <qdos.h>
char _prog_name[] = "ld86";
char _version[]   = " ? ";
char _copyright[] = "CPOC Linker";

void (*_consetup)() = consetup_title;			/* Fancy titled screen */
int (*_cmdwildcard)() = cmdexpand;				/* Wild card expansion */
int (*_Open)(const char *, int, ...) = qopen;
#endif /* QDOS */

#define NAME_MAX 14

PRIVATE bool_t flag[128];
#ifdef EPOC_TARGET
#ifdef EPOC
#define LIBDIR "\\cpoc\\lib\\"
#else
#ifdef WIN32
#define LIBDIR "\\cpoc\\lib\\"
#else
#define LIBDIR "/cpoc/lib/"
#endif /* WIN32 */
#endif /* EPOC */
#else
#define LIBDIR "/usr/local/lib/"
#endif
PRIVATE char * libdir = LIBDIR;
#ifdef MC6809
PRIVATE char libsubdir[] = "m09/";
PRIVATE char lib[sizeof LIBDIR - 1 + sizeof libsubdir - 1 + NAME_MAX + 1];
#else
#ifdef EPOC
PRIVATE char lib[P_FNAMESIZE + 1];
#else
PRIVATE char lib86subdir[] = "i86/";
PRIVATE char lib386subdir[] = "i386/";
PRIVATE char lib[sizeof LIBDIR - 1 + sizeof lib386subdir - 1 + NAME_MAX + 1];
#endif
#endif
PRIVATE char libprefix[] = "lib";
PRIVATE char libsuffix[] = ".a";

long text_base_address;


PUBLIC int main(argc, argv)
int argc;
char **argv;
{
    register char *arg;
    int argn;
    char *outfilename;

#ifdef EPOC
    _openerror(STDOUT_FILENO);
    CommandLineParameters(&argc, &argv);
#endif /* EPOC */
    ioinit(argv[0]);
    objinit();
    syminit();
    typeconv_init(BIG_ENDIAN, LONG_BIG_ENDIAN);
    flag['3'] = sizeof(char *) >= 4;
    outfilename = NULL;
    for (argn = 1; argn < argc; ++argn)
    {
	arg = argv[argn];
	if (*arg != '-')
	    readsyms(arg);
	else
	    switch (arg[1])
	    {
	    case '0':		/* use 16-bit libraries */
	    case '3':		/* use 32-bit libraries */
	    case 'M':		/* print symbols linked */
	    case 'i':		/* separate I & D output */
	    case 'm':		/* print modules linked */
#ifdef BSD_A_OUT
	    case 'r':		/* relocatable output */
#endif
	    case 's':		/* strip symbols */
	    case 'z':		/* unmapped zero page */
		if (arg[2] == 0)
		    flag[arg[1]] = TRUE;
		else if (arg[2] == '-' && arg[3] == 0)
		    flag[arg[1]] = FALSE;
		else
		    usage();
		if (arg[1] == '0')	/* flag 0 is negative logic flag 3 */
		    flag['3'] = !flag['0'];
		break;
	    case 'T':		/* text base address */
		if (arg[2] != 0 || ++argn >= argc)
		    usage();
		errno = 0;    
		text_base_address = strtoul(argv[argn], (char **) NULL, 16);
		if (errno != 0)
		    fatalerror("invalid text address");
		break;
	    case 'l':		/* library name */
		strcpy(lib, libdir);
#ifdef MC6809
		strcat(lib, libsubdir);
#endif
#ifdef I80386
		strcat(lib, flag['3'] ? lib386subdir : lib86subdir);
#endif
		strncat(lib, libprefix, NAME_MAX + 1);
		strncat(lib, arg + 2, NAME_MAX - (sizeof libprefix - 1)
					       - (sizeof libsuffix - 1));
		strcat(lib, libsuffix);
		readsyms(lib);
		break;
	    case 'o':		/* output file name */
		if (arg[2] != 0 || ++argn >= argc || outfilename != NULL)
		    usage();
		outfilename = argv[argn];
		break;
#ifndef GENERIC
            case 'L':           /* new library path */
                if (arg[2] != 0 || ++argn >= argc )
                    usage();
                libdir = argv[argn];
                break;
#endif /* ! GENERIC */
	    default:
		usage();
	    }
    }
    linksyms(flag['r']);
    if (outfilename == NULL)
	outfilename = "a.out";
    writebin(outfilename, flag['i'], flag['3'], flag['s'], flag['z']);
    if (flag['m'])
	dumpmods();
    if (flag['M'])
	dumpsyms();
    flusherr();
    return errcount ? 1 : 0;
}
