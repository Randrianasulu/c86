#
#       Makefile to build CC for CPOC or "C68 for QDOS"
#
#------------------------ CONFIGURATION -------------------------------
#
#   The exact facilities to be included in a particular version
#   of the front-en programcan be controlled by #defines.  Ideally
#   these are set through this makefile, or for environments like
#   Visual C via the project file for that environment.
#
#   The options are:
#
#   Set one of the following to specify the desired target environment:
#   QL      This is for versions of C68 for QDOS
#           (whatever the host system)
#   CPOC    This is for versions of C86 for the Psion 3a
#           (whatever the host system)
#
#   The following are used to give information about the operating environment
#   in which the system is to be hosted:
#   QDOS        This is for versions of C68 that are hosted on QDOS or a
#               compatible system such as Minerva or SMSQ(E)
#   __unix__    The enviroment us a Unix-like system
#   DOS_LIKE    MSDOS based system
#   WIN32       Windows 95 or Windows NT based system.
#   EPOC        Psion 3a based system
#
#
#   The following allow you to control optional features of the program
#   ENVVAR      Support for environment variables is to be included.  
#               Note that this requires the libraries to have getenv().
#   OBSOLETE    Builds in support for parameter options that are no
#               longer used, but were used in past versions.
#
#
#   The following options will only ever be set when trying to develop
#   or debug the CC front-end program itself:
#   LIBDEBUG    This can be set IF you have the "libdebug" library.
#   JDBG        This is an alternative that does an in-line print
#
#------------------------------------------------------------------------


PROG=cc
CC=cc
RM=rm -fv
ZIP=zip
D=/
S=.
LDFLAGS=
LIBS=
INDENT=indent
INDENTFLAGS= -bad -br -ce -di8 -i4 -npsl

#--------------------------------------------------------------
#     CC for QDOS when hosted on QDOS
#CFLAGS = -error=5 -warn=6 -maxerr=5 -O
#LDFLAGS = -ms
#DEFINES= -DQL -DENVVAR -DOBSOLETE -DHAVE_STRPOS \
#         -DHAVE_LIBGEN -DHAVE_STPCPY -DHAVE_ARGPACK
# DEFINES= -DQL -DENVVAR -DOBSOLETE -DHAVE_STRPOS \
#          -DHAVE_LIBGEN -DHAVE_STPCPY -DHAVE_ARGPACK -DLIBDEBUG
#LIBS= -ldebug
#S=_
#--------------------------------------------------------------
#     CPOC front-end when hosted on QDOS
#PROG=cpoc
#CFLAGS = -error=5 -warn=6 -maxerr=5 -O
#DEFINES= -DCPOC -DENVVAR
#S=_
#--------------------------------------------------------------
#     CC (XTC68 version) for QDOS when hosted on LINUX
#CC=gcc
#PROG=qcc
#DEFINES = -DQL -DXTC68 -DENVVAR -DHAVE_BASENAME -DHAVE_STPCPY
#--------------------------------------------------------------
#     CPOC front-end when hosted on LINUX
CC = gcc
PROG = cpoc_cc
DEFINES = -DCPOC -D__unix__ -DENVVAR
#--------------------------------------------------------------
#     CC (XTC68 version) for QDOS when hosted on WINDOWS 95/NT
#PROG=qcc
#DEFINES = -DQL -DXTC68 -DWIN32 -DENVVAR
#ZIP=pkzip
#D=\ 
#--------------------------------------------------------------
#     CPOC front-end when hosted on WINDOWS 95/NT
#PROG=cpoc_cc.exe
#DEFINES = -DCPOC -DDOS_LIKE -DWIN32 -DENVVAR
#RM=del
#ZIP=pkzip
#D=\ 
#--------------------------------------------------------------
#     CC front-end when hosted on WINDOWS 3.1x/MSDOS
#PROG=qcc.exe
#DEFINES = -DCPOC -DDOS_LIKE -DENVVAR
#RM=del
#ZIP=pkzip
#D=\ 
#-------------------------------------------------------------
#     CPOC front-end when hosted on WINDOWS 3.1x/MSDOS
#PROG=cpoc.exe
#DEFINES = -DCPOC -DDOS_LIKE -DENVVAR
#RM=del
#ZIP=pkzip
#D=\ 
#-------------------------------------------------------------

SRC= cc$(S)c ccutil$(S)c

OBJ= cc$(S)o ccutil$(S)o
#OBJ= cc$(S)o ccutil$(S)o _forkexec$(S)o argunpk$(S)o argfree$(S)o argpack$(S)o

HDR= cc$(S)h

#LNS=$(OBJ:.o=$.ln)

# .SUFFIXES:	$(S)o $(S)ln

$(PROG)	: $(OBJ)
	$(CC) -o $(PROG) $(LDFLAGS) $(OBJ) $(LIBS)

cc$(S)o : cc$(S)c cc$(S)h
	$(CC) -c $(CFLAGS) $(DEFINES) cc$(S)c

ccutil$(S)o : ccutil$(S)c cc$(S)h
	$(CC) -c $(CFLAGS) $(DEFINES) ccutil$(S)c

#   Library routines for special testing cases

_forkexec$(S)o : _forkexec$(S)c
	$(CC) -c $(CFLAGS) $(DEFINES) _forkexec$(S)c

argfree$(S)o : argfree$(S)c
	$(CC) -c $(CFLAGS) $(DEFINES) argfree$(S)c

argpack$(S)o : argpack$(S)c
	$(CC) -c $(CFLAGS) $(DEFINES) argpack$(S)c

argunpk$(S)o : argunpk$(S)c
	$(CC) -c $(CFLAGS) $(DEFINES) argunpk$(S)c

# .c.ln:
#	lint $(CFLAGS) -m -u -c $<

# $(OBJ) : $(HDR)

lint:	$(LNS)
	lint $(LNS)

indent:
	$(INDENT) $(INDENTFLAGS) $(SRC)
	@$(RM) *~
	$(INDENT) $(INDENTFLAGS) $(HDR)
	@$(RM) *~

clobber: clean
	@$(RM) $(PROG)
	@$(RM) *.exe

clean:
	@$(RM) debug/*.*
	@$(RM) release/*.*
	@$(RM) $(OBJ)
	@$(RM) *$(S)ln
	@$(RM) vc??.*
	@$(RM) *.obj
	@$(RM) *.o
	@$(RM) *.i
	@$(RM) *.s
	@$(RM) *.p??
	@$(RM) *.ilk
	@$(RM) *.ncb
	@$(RM) *.opt

zip:
	-$(RM) cc$(S)zip
	$(ZIP) cc$(S)zip $(PROG)
	$(ZIP) cc$(S)zip makefile cc$(S)pr $(SRC) $(HDR) cc$(S)1 cc$(S)ds?

zipbin:
	-$(RM) ccbin$(S)zip
	$(ZIP) ccbin$(S)zip $(PROG)

zipsrc:
	-$(RM) ccsrc$(S)zip
	$(ZIP) ccsrc$(S)zip makefile cc$(S)pr $(SRC) $(HDR) cc$(S)1 cc$(S)ds?
