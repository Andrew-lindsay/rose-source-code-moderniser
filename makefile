## A sample Makefile to build a ROSE tool.
##
## Important: remember that Makefile recipes must contain tabs:
##
##     <target>: [ <dependency > ]*
##         [ <TAB> <command> <endl> ]+
## So you have to replace spaces with Tabs if you copy&paste this file from a browser!

## ROSE installation contains
##   * libraries, e.g. "librose.la"
##   * headers, e.g. "rose.h"
ROSE_INSTALL=/home/sheep/opt/rose_inst
SRC_DIR=src
BUILD_DIR=build
TESTS_DIR=tests

# Standard C++ compiler stuff (see rose-config --help)
comma   := ,
CXX      = $(shell $(ROSE_INSTALL)/bin/rose-config cxx)
CPPFLAGS = $(shell $(ROSE_INSTALL)/bin/rose-config cppflags) -I.
CXXFLAGS = $(shell $(ROSE_INSTALL)/bin/rose-config cxxflags)
LIBDIRS  = $(shell $(ROSE_INSTALL)/bin/rose-config libdirs)
LDFLAGS  = $(shell $(ROSE_INSTALL)/bin/rose-config ldflags) -L. \
           $(addprefix -Wl$(comma)-rpath -Wl$(comma), $(subst :, , $(LIBDIRS)))

## Your translator
TRANSLATOR=function_insert_test
TRANSLATOR_SOURCE=$(SRC_DIR)/$(TRANSLATOR).cpp

## Input testcode for your translator
TESTCODE=

#-------------------------------------------------------------
# Makefile Targets
#-------------------------------------------------------------

all: $(BUILD_DIR)/$(TRANSLATOR)

# compile the translator and generate an executable
# -g is recommended to be used by default to enable debugging your code
$(BUILD_DIR)/$(TRANSLATOR): $(TRANSLATOR_SOURCE)
	$(CXX) -g $(TRANSLATOR_SOURCE) $(CPPFLAGS) $(LDFLAGS) -o $@

# test the translator
check: $(TRANSLATOR)
	./$(TRANSLATOR) -c -I. -I$(ROSE_INSTALL)/include $(TESTCODE) 

.PHONY: clean-build
.PHONY: clean-tests

clean-tests:
	rm -rf $(TESTS_DIR)/*.o $(TESTS_DIR)/rose_* $(TESTS_DIR)/*.dot $(TEST_DIR)/*.out

clean-build:
	rm -rf $(BUILD_DIR)/$(TRANSLATOR) $(BUILD_DIR)/*.o  
