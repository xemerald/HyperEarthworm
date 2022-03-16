########################################################################

# NOTE: This Makefile can build and install
# a. 32-bit libraries
#	make all32
#	make install32
# b. 64-bit libraries
#	make all64
#	make install64
# c. Both 32-bit and 64-bit libraries (if system can build both)
#	make all
#	make installall
#
# Default is "make all" (32-bit and 64-bit)

########################################################################

# Start Configuration Section

# 1. Select installation dir.
# Installation in /share/apps.
#ROOTDIR	= /share/apps
#LEAPSECONDS = $(ROOTDIR)/etc/leapseconds

# Installation in /usr/local.
ROOTDIR = /usr/local
LEAPSECONDS = /usr/local/etc/leapseconds

# 2. Select OS library directory structure.
# Linux:
LIB32DIR= $(ROOTDIR)/lib
LIB64DIR= $(ROOTDIR)/lib64
MANDIR	= $(ROOTDIR)/share/man
MANEXT	= 3

# Solaris:
#LIB32DIR= $(ROOTDIR)/lib
#LIB64DIR= $(ROOTDIR)/lib/64
#MANDIR	= $(ROOTDIR)/man
#MANEXT	= 3

# 3. These should not have to be changed.
INCDIR	= $(ROOTDIR)/include
BINDIR	= $(ROOTDIR)/bin

# 4. Select compiler and options.
CC	= cc
COPT	= -O
CDBG	= -g
CNL	= -DNO_LEAPSECONDS
C64	= -m64
C32	= -m32
DLEAP	= -DLEAPSECONDS=\"$(LEAPSECONDS)\"
CFLAGS_SITE = -Wall

# -Dfortran_suffix is required for fortran Compilers that use the BSD
# convention of appending a "_" suffix to the subroutine or function name.
# -Dqlib2_fortran is required to compile the fortran interface routines.

CFLAGS	= $(CFLAGS_SITE) -Dfortran_suffix -Dqlib2_fortran

# 5.  Select default target (all32, all64, or both)
all:	all32 all64

# End Configuration Section

########################################################################

DIR	= qlib2

SRCS	= drm_utils.c ms_utils.c ms_pack.c ms_pack2.c ms_unpack.c \
	pack.c qda_utils.c qtime.c sdr_utils.c unpack.c qutils.c

HDR =	qlib2.h

HDRS =	qdefines.h msdatatypes.h timedef.h \
	qsteim.h sdr.h qda.h seismo.h data_hdr.h \
	qtime.h qutils.h qda_utils.h drm_utils.h sdr_utils.h \
	ms_utils.h ms_pack.h ms_pack2.h pack.h ms_unpack.h unpack.h

FHDR =	qlib2.inc
F90HDR = qlib2_90.inc

FHDRS =	qdefines.inc data_hdr.inc qfortran.inc
F90HDRS = qdefines.inc data_hdr_90.inc qfortran.inc

OBJS	= $(SRCS:%.c=%.o)

.PRECIOUS:	$(TARGET1) $(TARGET2) $(TARGET3) $(TARGET4) $(TARGET5) $(TARGET6) $(TARGET7) $(TARGET8)

L32DIR	= lib
L64DIR	= lib64
TARGET1	= $(L32DIR)/libqlib2.a
TARGET2	= $(L32DIR)/libqlib2.debug.a
TARGET3	= $(L32DIR)/libqlib2nl.a
TARGET4	= $(L32DIR)/libqlib2nl.debug.a
TARGET5	= $(L64DIR)/libqlib2.a
TARGET6	= $(L64DIR)/libqlib2.debug.a
TARGET7	= $(L64DIR)/libqlib2nl.a
TARGET8	= $(L64DIR)/libqlib2nl.debug.a
LLIBS	= $(L32DIR) $(L64DIR)

ALLHDRS	= $(HDR) $(FHDR) $(F90HDR)
ALLLIBS32 = $(TARGET1) $(TARGET2) $(TARGET3) $(TARGET4)
ALLLIBS64 = $(TARGET5) $(TARGET6) $(TARGET7) $(TARGET8)
ALLLIBS = $(ALLLIBS32) $(ALLLIBS64)
ALL	= $(ALLHDRS) $(ALLLIBS)

########################################################################

all32:	$(ALLHDRS) $(ALLLIBS32)
all64:	$(ALLHDRS) $(ALLLIBS64)

########################################################################

$(HDR):	$(HDRS)
	cat $(HDRS)  | grep -v '#include "'> $@

$(FHDR): $(FHDRS)
	cat $(FHDRS) | grep -v '#include "'> $@

$(F90HDR): $(F90HDRS)
	cat $(F90HDRS) | grep -v '#include "'> $@

$(L32DIR):	
	mkdir -p $(L32DIR)

$(L64DIR):	
	mkdir -p $(L64DIR)

$(TARGET1): $(SRCS) $(HDRS) $(HDR)
	mkdir -p $(L32DIR)
	$(CC) -c $(DLEAP) $(CFLAGS) $(C32) $(COPT) $(SRCS)
	ar r $(TARGET1) $(OBJS)
	-rm -f $(OBJS)

$(TARGET2): $(SRCS) $(HDRS) $(HDR)
	mkdir -p $(L32DIR)
	$(CC) -c $(DLEAP) $(CFLAGS) $(C32) $(CDBG) $(SRCS)
	ar r $(TARGET2) $(OBJS)
	-rm -f $(OBJS)

$(TARGET3): $(SRCS) $(HDRS) $(HDR)
	mkdir -p $(L32DIR)
	$(CC) -c $(DLEAP) $(CFLAGS) $(C32) $(CNL) $(COPT) $(SRCS)
	ar r $(TARGET3) $(OBJS)
	-rm -f $(OBJS)

$(TARGET4): $(SRCS) $(HDRS) $(HDR)
	mkdir -p $(L32DIR)
	$(CC) -c $(DLEAP) $(CFLAGS) $(C32) $(CNL) $(CDBG) $(SRCS)
	ar r $(TARGET4) $(OBJS)
	-rm -f $(OBJS)

$(TARGET5): $(SRCS) $(HDRS) $(HDR)
	mkdir -p $(L64DIR)
	$(CC) -c $(DLEAP) $(CFLAGS) $(COPT) $(C64) $(SRCS)
	ar r $(TARGET5) $(OBJS)
	-rm -f $(OBJS)

$(TARGET6): $(SRCS) $(HDRS) $(HDR)
	mkdir -p $(L64DIR)
	$(CC) -c $(DLEAP) $(CFLAGS) $(CDBG) $(C64) $(SRCS)
	ar r $(TARGET6) $(OBJS)
	-rm -f $(OBJS)

$(TARGET7): $(SRCS) $(HDRS) $(HDR)
	mkdir -p $(L64DIR)
	$(CC) -c $(DLEAP) $(CFLAGS) $(CNL) $(COPT) $(C64) $(SRCS)
	ar r $(TARGET7) $(OBJS)
	-rm -f $(OBJS)

$(TARGET8): $(SRCS) $(HDRS) $(HDR)
	mkdir -p $(L64DIR)
	$(CC) -c $(DLEAP) $(CFLAGS) $(CNL) $(CDBG) $(C64) $(SRCS)
	ar r $(TARGET8) $(OBJS)
	-rm -f $(OBJS)

install:	installall

installall:	install_hdr install_leapseconds install_man install_libs

install32:	install_hdr install_leapseconds install_man install_lib32

install64:	install_hdr install_leapseconds install_man install_lib64

install_hdr:
	cp $(HDR) $(FHDR) $(F90HDR) drm_seismo.h $(INCDIR)/

install_leapseconds:	leapseconds
	cp leapseconds $(LEAPSECONDS)

install_libs:	install_lib32 install_lib64

install_lib32:	$(ALLLIBS32)
	for lib in $(ALLLIBS32) ; do \
		cp $$lib $(LIB32DIR)/ ; \
	done

install_lib64:	$(ALLLIBS64) leapseconds
	for lib in $(ALLLIBS64) ; do \
		cp $$lib $(LIB64DIR)/ ; \
	done

install_man:	FORCE
	cp qlib2.man $(MANDIR)/man$(MANEXT)/qlib2.$(MANEXT)

clean:
	-rm -f *.o $(ALL)

veryclean:	clean

tar:
	(cd .. && tar cf - $(DIR)/*.c $(DIR)/*.h $(DIR)/Makefile \
		$(DIR)/*.inc $(DIR)/*.man \
		$(DIR)/README $(DIR)/CHANGES $(DIR)/leapseconds ) \
	| gzip > /tmp/$(DIR).tar.gz

$(QLIB2):	FORCE
	cd ../qlib2 && $(MAKE)

FORCE:

cleandepend:
	makedepend

depend:	$(HDR) $(FHDR)
	makedepend $(CFLAGS) *.c
	sed -e "s/^[^ 	]*\.o/$(TARGET)(&)/" < Makefile > Makefile.new \
		&& mv Makefile.new Makefile

########################################################################
# DO NOT DELETE THIS LINE -- make depend depends on it.
