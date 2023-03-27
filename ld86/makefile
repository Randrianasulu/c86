#
#       Makefile to build LD86 for CPOC
#

DEFINES = -DEPOC_TARGET
PROG=ld86
EXT=
CC = cc
RM = rm -f
ZIP = zip
S=.

#--------------------------------------------------------------
#     LD86 when hosted on QDOS

#CFLAGS = -error=3 -warn=4 -maxerr=3 -O
S=.
#--------------------------------------------------------------
#     LD86 when hosted on LINUX

#--------------------------------------------------------------
#     LD86 when hosted on MSDOS

#EXT=.exe
#CFLAGS = -DWIN32
#RM=del
#ZIP=pkzip
#-------------------------------------------------------------
#     LD86 when hosted on Psion 3a

#EXT=.img
#-------------------------------------------------------------

SRC= dumps$(S)c epoc$(S)c io$(S)c ld$(S)c lseek$(S)c \
     readobj$(S)c table$(S)c typeconv$(S)c writebin$(S)c  

OBJ= dumps$(S)o epoc$(S)o io$(S)o ld$(S)o lseek$(S)o \
     readobj$(S)o table$(S)o typeconv$(S)o writebin$(S)o  

     
HDR= align.h aout-gnu.h ar$(S)h a_out$(S)h byteord$(S)h \
     config$(S)h const$(S)h globvar$(S)h obj$(S)h type$(S)h      

.c.o:
	$(CC) -c $(DEFINES) $(CFLAGS) $<

all:    $(PROG)$(EXT)

$(PROG)$(EXT) : $(OBJ)
	$(CC) -O -o $(PROG)$(EXT) $(LDFLAGS) $(OBJ)

$(OBJ): $(HDR)

clobber: clean
	-$(RM) $(PROG)$(EXT)

clean:
	$(RM) $(OBJ)

zipbin:
	-$(RM) $(PROG)bin$(S)zip
	$(ZIP) $(PROG)bin$(S)zip $(PROG) 

zipsrc:
	-$(RM) $(PROG)src$(S)zip
	$(ZIP) $(PROG)src$(S)zip *$(S)c *$(S)h makefile *$(S)pr *$(S)1

