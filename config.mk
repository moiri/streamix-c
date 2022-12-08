###############################################################################
# CUTOMIZE THIS FILE TO SUIT YOUR NEEDS FOR THE BUILD PROCESS                 #
# This file is included into the following files (refer to them for more      #
# information on the build process):                                          #
#  - Makefile                                                                 #
#  - test/Makefile                                                            #
###############################################################################

# The version number of the app ($(VMAJ).$(VMIN).$(VREV))
VMAJ = 0
VMIN = 6
VREV = 0
VDEB = 1

# the utility library
LIB_SMXUTILS = -lsmxutils-0.4

# the include path of the smxutlis header files
INC_SMXUTILS = -I/usr/include/smx/libsmxutils-0.4

# the name of the application
APPNAME = smxc
