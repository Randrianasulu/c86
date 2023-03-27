#
#   Makefile for C68 LD v2 command
#
#   Choose the appropriate set up to suit your system
#

CC = cc

#   Normal settings

# LDFLAGS= -ms -fldv1
LDFLAGS= -m+s+a
LDLIBS = -lrll -llist
DEFINES= -DNDEBUG -DRLL_SUPPORT
S = .
O = o

#   Settings for when debugging
#   (Comment out for normal use)
LDLIBS = -lrll -llistd -ldebug
DEFINES = -DRLL_SUPPORT -DLIBDEBUG

#------------------------------------------------------------
#  ... for compiling with C68 on QDOS
CC = cc 
CFLAGS = -Qerror=5 -Qwarn=5 -maxerr=5 -O
I = ${P}INCLUDE_
S = _
#------------------------------------------------------------
#   .. for compiling with MINIX-68K
# CC = cc68
# CFLAGS = -Werror=5 -Qwarn=6 -maxerr=5 -D_MINIX -I../ -O
# I = /usr/include/
#------------------------------------------------------------
#   .. for compiling with MINIX-PC
# CC = cc
# CFLAGS = -F -D_MINIX -I../
# LDFLAGS = -i
# LDLIBS=
# I = /usr/include/
# O = s
#------------------------------------------------------------
#   .. for compiling with MINIX-386 and GNU C
# CC = gcc
# CFLAGS = -D_MINIX -I../ -I/usr/include -I../include -O
# I = /usr/include/
#------------------------------------------------------------
#   .. for compiling with MINIX-386 and Bruce Evans BCC
# CC = bcc
# CFLAGS = -D_MINIX -I../ -O
# I = /usr/include/
#------------------------------------------------------------
#   .. for compiling with MINIX-386 and c386
# CC = gcc
# CFLAGS = -Qerror=5 -Qwarn=6 -Qmaxerr=5 -D_MINIX -I../ -O
# I = /usr/include/
# S = .
# O = o
#------------------------------------------------------------
#   .. for compiling with LINUX-386
# CC = gcc
# CFLAGS = -D_MINIX -I../ -O
# LDFLAGS =
# LDLIBS=
# I = /usr/include/
#------------------------------------------------------------
#   .. for compiling with UNIX SVR4
# CC = cc
# CFLAGS = -c -D_MINIX -DSYSV -I../include
# I = /usr/include/
# LINTFLAGS = -m -x
#------------------------------------------------------------

LD = ld
RM = rm
CP = cp

SRC  =  ldmain${S}c     ldparams$(S)c \
        ldlist$(S)c     ldsectn$(S)c    ldsymbol$(S)c \
        ldrll$(S)c      ldutils$(S)c

HDR  =  ld$(S)h         ldconfig$(S)h \
        ldmsgs$(S)h     ldproto$(S)h

DOCS =  ReadMe$(S)txt   ld$(S)htm

MISC =  Makefile        ld$(S)dsw       ld$(S)dsp

OBJS =  ldmain${S}${O}  ldparams$(S)$(O) \
        ldlist$(S)$(O)  ldsectn$(S)$(O) ldsymbol$(S)$(O) \
        ldrll$(S)$(O)   ldutils$(S)$(O)

LOBJ =  ldmain$(S)ln    ldparams$(S)ln \
        ldlist$(S)ln    ldsectn$(S)ln   ldsymbol$(S)ln \
        ldrll$(S)ln     ldutils$(S)ln


.SUFFIXES : ${S}x ${S}s ${S}asm ${S}o ${S}rel ${S}hdr ${S}h

all :   ${OBJS}
    ${CC} ${LDFLAGS} -o ld ${OBJS} ${LDLIBS}

testasm : testasm1_o testasm2_o
    ld -ms -s- -o testasm -Rtestasm/1.01 testasm1_o testasm2_o

makerll : testasm2_o
    ld -ms -s- -o libasm2_rll -Rasm2/1.01 testasm2_o

linkrll : testasm1_o libasm2_rll
    ld -ms -s- -o testasm1 testasm1_o -rasm2

testlink : testlink_o
    $(CC) -ms -o testlink testlink$(S)o

${S}hdr${S}h :
    packhdr $*${S}hdr $*${S}h

$(S)c$(S)o :
    $(CC) -c -O $(CFLAGS) $(DEFINES) $<

${S}c${S}ln:
    lint ${CFLAGS} -x ${DEFINES} $< >>ld${S}lint 2>&1

lint :lint2 ${LOBJ}
    lint ${LOBJ} ${LINTFLAGS} >>ld$Slint 2>&1

clean:
    ${RM} -f *$S$O *$Si *$Ss *$Sln

clobber: clean
    ${RM} -f ld


zip:    zipsrc  zipqdos

zipsrc:
    @rm -f ldsrc$(S)zip
    zip ldsrc   $(DOCS) $(MISC)  $(SRC) $(HDR)

zipqdos:
    @rm -f ldqdos$(S)zip
    zip ldqdos  ld
#    zip ldqdos  $(DOCS) ld

#---------------------- Header file dependencies --------------------------

ld${S}h  :  ${I}ctype${S}h    ${I}errno${S}h \
            ${I}fcntl${S}h \
            ${I}stdlib${S}h   ${I}stdio${S}h \
            ${I}string$(S)h \
            ${I}qdos$(S)h     $(I)things$(S)h \
            ldmsgs${S}h       ldconfig$(S)h
    touch ld$Sh

${OBJS} :  ld$(S)h
${LOBJ} :  ld$(S)h
ldrll$(S)$(O) : ${I}rll$(S)h
