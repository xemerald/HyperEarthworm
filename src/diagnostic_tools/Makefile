#
#       makefile for Earthworm
#             Lucky Vidmar Mon Jan 11 21:31:51 MST 1999
#
#
# This makefile contains all the directives necessary to compile
# earthworm modules in a directory under Solaris and NT, as well as
# clean up the Earthworm source and binary file tree.
#
# 1. To build (compile) the Earthworm modules on a Sun:
#        make solaris
#
# 2. To build (compile) the Earthworm modules on an NT PC:
#        nmake nt
#
# 3. To clean up the source tree, i.e. remove all *.o, *.obj, *~, *%,
#    and core files from all modules, on a Sun:
#        make clean_solaris
#
# 4. To clean up the source tree, i.e. remove all *.o, *.obj, *~, *%,
#    and core files from all modules, on a PC running NT:
#        nmake clean_nt
#
# 5. To clean up the bin directory, i.e. remove all Earthworm programs,
#    on a Sun:
#        make clean_bin_solaris
#
# 6. To clean up the bin directory, i.e. remove all Earthworm programs,
#    on a PC running NT:
#        nmake clean_bin_nt

# Whenever a new module is added to the Earthworm distribution, it should be
# added to the list of ALL_MODULES, and to the appropriate lists of
# SOLARIS_MODULES, UNIX_MODULES, and NT_MODULES.  The reverse procedure should
# be followed when a module is removed from the Earthworm distribution.

# Note: Neither Sun make nor Microsoft nmake support the GNU gmake .PHONY
# pseudo-target.  Makefiles should use a phony target (the name does not matter)
# with no dependencies and no rules, then add the phony dependency to force a
# recipe to run.  This means there can never be a file with the phony target
# name (case insensitive).  Earthworm makefiles use PHONY for this purpose.

# Note: Windows nmake does not pass $(MAKEFLAGS) as part of $(MAKE) for
# recursive makes; use $(MAKE) /$(MAKEFLAGS).

#
# List all modules (for cleaning purposes)
#

ALL_MODULES = \
	buildinfo \
	dumpwave \
	file2ring \
	findwave \
	gaplist \
	getmenu \
	getstation \
	maketrace \
	putpick \
	sniffring \
	sniffrings \
	sniffwave \
	time_converter \
	wsv_test

#
# List all modules to be compiled into the Unix/Linux/Mac OS X distribution
#

UNIX_MODULES = \
	dumpwave \
	file2ring \
	findwave \
	gaplist \
	getmenu \
	getstation \
	maketrace \
	putpick \
	sniffring \
	sniffrings \
	sniffwave \
	time_converter


#####################
# Top level targets #
#####################

# There is no default target; print usage info
usage: PHONY
	@echo "Usage: make unix"

unix:    unix_modules

###############################
# Unix/Linux/Mac OS X targets #
###############################

unix_modules: PHONY
	@for x in $(UNIX_MODULES) ; \
	do ( \
		cd $$x && \
			echo ---------- ; \
			echo Making $@ in: `pwd` ; \
			$(MAKE) -f makefile.unix \
	) ; done

clean_unix: PHONY
	-@for x in $(ALL_MODULES) ; \
	do ( \
		cd $$x && \
			echo Cleaning in: `pwd` ; \
			$(MAKE) -f makefile.unix clean \
	) ; done

clean_bin_unix: PHONY
	-@for x in $(ALL_MODULES) ; \
	do ( \
		cd $$x && \
			echo Cleaning binaries in: `pwd` ; \
			$(MAKE) -f makefile.unix clean_bin \
	) ; done

PHONY:
