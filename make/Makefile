# Makefile for make version 2

#   ... using C68 compiler
CC = cc
CFLAGS = -DQDOS -Dunix -O 
RM = rm

S=_

O = $(S)o
LD = ld
LDFLAGS = -ms

SRC     =   check$(S)c  input$(S)c  macro$(S)c \
            main$(S)c   make$(S)c   reader$(S)c \
            rules$(S)c  make$(S)h \
            Makefile    make$(S)doc

OBJS    =   check$(S)o  input$(S)o  macro$(S)o \
            main$(S)o   make$(S)o   reader$(S)o \
            rules$(S)o

make:   $(OBJS)
    $(LD) -omake $(LDFLAGS) $(OBJS)

_c_o :
    $(CC) -c $(CFLAGS) $<


clean:
    @$(RM) -fv *$(S)o *$(S)i *$(S)s

clobber: clean
    @$(RM) -fv make *_map

zip: zipbin zipsrc

zipbin:
    @rm -fv makebin$(S)zip
    @zip makebin$(S)zip make

zipsrc:
    @rm -fv makesrc$(S)zip
    @zip makesrc$(S)zip $(SRC)

#--------------------------- Dependencies ----------------------------

I = $PINCLUDE_

make$(S)h    :  $(I)sys_types$(S)h  $(I)sys_stat$(S)h  \
                $(I)errno$(S)h      $(I)time$(S)h       $(I)qdos$(S)h  \
                $(I)string$(S)h     $(I)ctype$(S)h      $(I)stdio$(S)h
#check$(S)o   :  make$(S)h
#input$(S)o   :  make$(S)h
#macro$(S)o   :  make$(S)h
#main$(S)o    :  make$(S)h
#make$(S)o    :  make$(S)h
#reader$(S)o  :  make$(S)h
#rules$(S)o   :  make$(S)h   $(I)stdlib$(S)h

