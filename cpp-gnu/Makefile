#   Makefile for GNU CPP
CC= cc
RM= rm
ZIP=zip
#S=_
S=.

#CFLAGS= -O -maxerr=1 -warn=3 -lang=c90 -DGPLUSPLUS_INCLUDE_DIR="" -DGCC_INCLUDE_DIR="" -DSTDC_VALUE=1
CFLAGS= -O -DGPLUSPLUS_INCLUDE_DIR="" -DGCC_INCLUDE_DIR="" -DSTDC_VALUE=1
LDFLAGS= -bufp100k 

SRCS= alloca$(S)c version$(S)c cexp$(S)c cccp$(S)c
OBJS= alloca$(S)o version$(S)o cexp$(S)o cccp$(S)o

cpp: $(OBJS)
	${CC} -ocpp $(LDFLAGS) $(OBJS)

clean:
	@${RM} -fv ${OBJS}

clobber : clean
	@${RM} -fv cpp

zip:    zipbin   zipsrc

zipbin:
	$(RM) -f qcppbin$(S)zip
	$(ZIP) qcppbin$(S)zip  cpp

zipsrc:
	$(RM) -f qcppsrc$(S)zip
	$(ZIP) qcppsrc$(S)zip Makefile *$(S)h *$(S)c
