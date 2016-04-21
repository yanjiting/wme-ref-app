#
# modified from ACE_wrappers\include\makeinclude\rules_*.GNU
#

LIB ?=
DLL ?=
CLEANUP ?=

ifdef USER_SRCS
  OBJS += $(addsuffix .$(OBJEXT),$(addprefix $(OBJDIR)/,$(basename $(notdir $(USER_SRCS)))))
  CLEANUP += $(OBJS)
endif # USER_SRCS

ifdef LIB
  VLIB = $(LIB_DIR)/$(LIB)
  CLEANUP += $(VLIB)
endif
ifdef BIN
  VBIN = $(BIN_DIR)/$(BIN)
  CLEANUP += $(VBIN)
endif
ifdef DLL
  VDLL = $(DLL_DIR)/$(DLL)
  CLEANUP += $(VDLL)
endif

# If the client makefile is not called "Makefile", the USER_MAKEFILE_NAME
# variable must be set to its actual name before including this
# file to allow the recursive MAKE to work properly.
ifdef USER_MAKEFILE_NAME
  MAKEFILE = $(USER_MAKEFILE_NAME)
else
  MAKEFILE = Makefile
endif # USER_MAKEFILE_NAME

MULTI_MAKEFILES =
ifdef USER_MULTI_MAKEFILES
  MULTI_MAKEFILES = $(addsuffix .mmakefiles, $(USER_MULTI_MAKEFILES))
endif # USER_MULTI_MAKEFILES

SUB_DIRS =
ifdef USER_SUB_DIRS
  SUB_DIRS = $(addsuffix .subdirs, $(USER_SUB_DIRS))
endif # USER_SUB_DIRS

TARGETS_LOCAL  = all.local clean.local
TARGETS_NESTED = $(TARGETS_LOCAL:.local=.nested)

all: $(SECURITY) $(SERVER_V) all.nested all.local
clean: $(SECURITY_CLEAN) clean.nested clean.local

all.local: check_dir $(VBIN) $(VLIB) $(VDLL) $(OBJS)

CHECK_DIRS += $(OBJDIR) $(LIB_DIR) $(BIN_DIR) $(DLL_DIR)
check_dir: $(CHECK_DIRS)
#	@echo $(VLIB)

$(CHECK_DIRS):
ifndef MULTI_MAKEFILES
	@test -d $@ || mkdir -p $@ 
endif # !MULTI_MAKEFILES

$(CHECK_OBJDIR):
ifndef MULTI_MAKEFILES
	@test -d $(OBJ_DIR) || mkdir -p $(OBJ_DIR)
endif # !MULTI_MAKEFILES

$(CHECK_LIBDIR):
ifndef MULTI_MAKEFILES
	@test -d $(LIB_DIR) || mkdir -p $(LIB_DIR)
endif # !MULTI_MAKEFILES

$(CHECK_BINDIR):
ifndef MULTI_MAKEFILES
	@test -d $(BIN_DIR) || mkdir -p $(BIN_DIR)
endif # !MULTI_MAKEFILES

$(CHECK_DLLDIR):
ifndef MULTI_MAKEFILES
	@test -d $(DLL_DIR) || mkdir -p $(DLL_DIR)
endif # !MULTI_MAKEFILES

$(OBJDIR)/%.$(OBJEXT): $(CHECK_OBJDIR) %.c
	$(COMPILE.cc) $(CC_OUTPUT_FLAG) $@ $<

$(OBJDIR)/%.$(OBJEXT): $(CHECK_OBJDIR) %.cpp
	$(COMPILE.cpp) $(CC_OUTPUT_FLAG) $@ $<

$(OBJDIR)/%.$(OBJEXT): $(CHECK_OBJDIR) %.cxx
	$(COMPILE.cpp) $(CC_OUTPUT_FLAG) $@ $<

$(OBJDIR)/%.$(OBJEXT): $(CHECK_OBJDIR) %.asm
	nasm -f elf $(NASMFLAG) $(CC_OUTPUT_FLAG) $@ $<

# user can ar USER_LINK_LIBS into xxx.a file
$(VLIB): $(CHECK_LIBDIR) $(OBJS) $(USER_LINK_LIBS)
	$(AR) $(ARFLAGS) $@ $(OBJS) $(USER_LINK_LIBS)
	-chmod a+r $@

#Share library,Field,2005-2-24
$(VDLL): $(CHECK_DLLDIR) $(OBJS) $(VLIB) 
	$(CC) -shared -o $@ $(OBJS) $(USER_LINK_LIBS)

# don't depend on $(LIBS) due to -L and -l
$(VBIN): $(CHECK_BINDIR) $(OBJS) $(VLIB) $(LIBS)
	$(LINK.cpp) $(CC_OUTPUT_FLAG) $@ $(OBJS) $(VLIB) $(LIBS) $(DLLS)

clean.local:
ifneq ($(CLEANUP),)
	$(RM) $(CLEANUP)
endif # CLEANUP

%.mmakefiles: %
	 $(MAKE) -s -j4 -f $< $(MMAKEFILES_TARGET)

%.subdirs: %  #Field,2005-6-7
	 $(MAKE) -C $< $(SUBDIRS_TARGET) -f $(MAKEFILE)

$(TARGETS_NESTED):
ifdef MULTI_MAKEFILES
	$(MAKE)  -s  -f $(MAKEFILE) MMAKEFILES_TARGET=$(@:.nested=) $(MULTI_MAKEFILES)
endif # MULTI_MAKEFILES
ifdef SUB_DIRS
	$(MAKE)  -s  -f $(MAKEFILE) SUBDIRS_TARGET=$(@:.nested=) $(SUB_DIRS)
endif # SUB_DIRS
