#
#       Top level makefile for Earthworm
#             Lucky Vidmar Mon Feb 14 21:48:28 GMT 2000
#
#
# This makefile contains all the directives necessary to compile
# earthworm under Solaris and NT, as well as clean up the Earthworm
# source and binary file tree.
#
# 1. To build (compile) the entire Earthworm distribution on a Sun:
#        make solaris
#
# 1a. To compile the Earthworm distrib. on a Linux or other POSIX box:
#        make unix
#
# 2. To build (compile) the entire Earthworm distribution on an NT PC:
#        nmake nt
#
# 3. To clean up the source tree, i.e. remove all *.o, *.obj, *~, *%,
#    and core files from all modules, on a Sun:
#        make clean_solaris
#
# 3a. To clean up the source tree on a POSIX computer:
#        make clean_unix
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
#
#

#
# Starting with Earthworm version 5, the source directory is
# divided into subfolders. Whenever a new module is added to Earthworm,
# it should be added to the Makefile in the appropriate subfolder.
#

# Note: Neither Sun make nor Microsoft nmake support the GNU gmake .PHONY
# pseudo-target.  Makefiles should use a phony target (the name does not matter)
# with no dependencies and no rules, then add the phony dependency to force a
# recipe to run.  This means there can never be a file with the phony target
# name (case insensitive).  Earthworm makefiles use PHONY for this purpose.

# Note: Windows nmake does not pass $(MAKEFLAGS) as part of $(MAKE) for
# recursive makes; use $(MAKE) /$(MAKEFLAGS).

#
# If a new subfolder is created, it should be added to the list of SUBFOLDERs
#

SUBFOLDERS = \
	archiving \
	diagnostic_tools \
	system_control

#####################
# Top level targets #
#####################

# There is no default target; print usage info
usage: PHONY
	@echo "Usage: make unix"

unix:    unix_buildinfo     unix_libs     unix_modules

###############################
# Unix/Linux/Mac OS X targets #
###############################

unix_buildinfo: BUILDINFO_MESSAGE
	cd diagnostic_tools/buildinfo && \
		$(MAKE) -f makefile.unix clean ; \
		$(MAKE) -f makefile.unix clean_bin ; \
		$(MAKE) -f makefile.unix
	buildinfo

unix_libs: CHECK_PLATFORM_UNIX MAKING_C_LIBRARIES_MESSAGE
	@cd libsrc && \
		$(MAKE) -f makefile.unix

unix_modules: CHECK_PLATFORM_UNIX MAKING_MODULES_MESSAGE
	@for x in $(SUBFOLDERS) ; \
	do  ( \
		cd $$x && \
			echo ---------- ; \
			echo Making $@ in: `pwd` ; \
			$(MAKE) unix \
	) ; done

CHECK_PLATFORM_UNIX: PHONY
	@if [ ! "$(PLATFORM)" ] ; then \
		echo " set PLATFORM env var to LINUX for GNU or Mac OS X systems" ; \
		exit 1 ; \
	fi

clean_unix: PHONY
	-@cd diagnostic_tools/buildinfo && \
		echo Cleaning in: `pwd` ; \
		$(MAKE) -f makefile.unix clean
	-@cd libsrc && \
		echo Cleaning in: `pwd` ; \
		$(MAKE) -f makefile.unix clean
	-@for x in $(SUBFOLDERS) ; \
	do ( \
		cd $$x && \
			echo Cleaning in: `pwd` ; \
			$(MAKE) clean_unix \
	) ; done

clean_bin_unix: PHONY
	-@cd diagnostic_tools/buildinfo && \
		echo Cleaning binaries in: `pwd` ; \
		$(MAKE) -f makefile.unix clean_bin
	-@cd libsrc && \
		echo Cleaning binaries in: `pwd` ; \
		$(MAKE) -f makefile.unix clean_bin
	-@for x in $(SUBFOLDERS) ; \
	do ( \
		cd $$x && \
			echo Cleaning binaries in: `pwd` ; \
			$(MAKE) clean_bin_unix \
	) ; done

##################
# Helper targets #
##################

BUILDINFO_MESSAGE: PHONY
	@echo -*-*-*-*-*-*-*-*-*-*
	@echo Earthworm build info
	@echo -*-*-*-*-*-*-*-*-*-*

MAKING_C_LIBRARIES_MESSAGE: PHONY
	@echo -*-*-*-*-*-*-*-*-*
	@echo Making C libraries
	@echo -*-*-*-*-*-*-*-*-*

MAKING_MODULES_MESSAGE: PHONY
	@echo -*-*-*-*-*-*-*-*-*-*-*-*
	@echo Making Earthworm modules
	@echo -*-*-*-*-*-*-*-*-*-*-*-*

PHONY:
