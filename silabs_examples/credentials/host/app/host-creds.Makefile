####################################################################
# User Makefile for Host Systems                                   #
# This will not be overwritten. Edit as desired.                   #
####################################################################
.SUFFIXES:				# ignore builtin rules
.PHONY: all debug release clean

# Default goal
all: debug

####################################################################
# Definitions                                                      #
####################################################################

# Values set by the initial generation
PROJECTNAME = host-creds

####################################################################
# Definitions of toolchain.                                        #
# You might need to do changes to match your system setup          #
####################################################################

AR      = "ar"
CC      = "gcc"
CXX     = "g++"
LD      = "gcc"
RM      = "rm"

# Command output is hidden by default, it can be enabled by
# setting VERBOSE=true on the commandline.
ifeq ($(VERBOSE),)
  ECHO = @
endif

# Build Directories
BUILD_DIR = build
LST_DIR = lst

ifneq ($(filter $(MAKECMDGOALS),release),)
  OUTPUT_DIR = $(BUILD_DIR)/release
else
  OUTPUT_DIR = $(BUILD_DIR)/debug
endif

# Values that should be appended by the sub-makefiles
C_SOURCE_FILES   = 
CXX_SOURCE_FILES = 
ASM_SOURCE_FILES = 

LIBS = 

C_DEFS   = 
ASM_DEFS = 

INCLUDES = 

C_FLAGS           = 
C_FLAGS_DEBUG     = 
C_FLAGS_RELEASE   = 
CXX_FLAGS         = 
CXX_FLAGS_DEBUG   = 
CXX_FLAGS_RELEASE = 
ASM_FLAGS         = 
ASM_FLAGS_DEBUG   = 
ASM_FLAGS_RELEASE = 
LD_FLAGS          = 

OBJS =

####################################################################
# Include sub-makefiles                                            #
# Define a makefile here to add files/settings to the build.       #
####################################################################
-include host-creds.project.mak


####################################################################
# Rules                                                            #
####################################################################

# -MMD : Don't generate dependencies on system header files.
# -MP  : Add phony targets, useful when a h-file is removed from a project.
# -MF  : Specify a file to write the dependencies to.
DEPFLAGS = -MMD -MP -MF $(@:.o=.d)

CSOURCES       = $(notdir $(C_SOURCE_FILES))
CXXSOURCES     = $(notdir $(filter %.cpp, $(CXX_SOURCE_FILES)))
CCSOURCES      = $(notdir $(filter %.cc, $(CXX_SOURCE_FILES)))
ASMSOURCES_s   = $(notdir $(filter %.s, $(ASM_SOURCE_FILES)))
ASMSOURCES_S   = $(notdir $(filter %.S, $(ASM_SOURCE_FILES)))

COBJS       = $(addprefix $(OUTPUT_DIR)/,$(CSOURCES:.c=.o))
CXXOBJS     = $(addprefix $(OUTPUT_DIR)/,$(CXXSOURCES:.cpp=.o))
CCOBJS      = $(addprefix $(OUTPUT_DIR)/,$(CCSOURCES:.cc=.o))
ASMOBJS_s   = $(addprefix $(OUTPUT_DIR)/,$(ASMSOURCES_s:.s=.o))
ASMOBJS_S   = $(addprefix $(OUTPUT_DIR)/,$(ASMSOURCES_S:.S=.o))
OBJS        += $(COBJS) $(CXXOBJS) $(CCOBJS) $(ASMOBJS_s) $(ASMOBJS_S)

CDEPS       += $(addprefix $(OUTPUT_DIR)/,$(CSOURCES:.c=.d))
CXXDEPS     += $(addprefix $(OUTPUT_DIR)/,$(CXXSOURCES:.cpp=.d))
CXXDEPS     += $(addprefix $(OUTPUT_DIR)/,$(CCSOURCES:.cc=.d))
ASMDEPS_s   += $(addprefix $(OUTPUT_DIR)/,$(ASMSOURCES_s:.s=.d))
ASMDEPS_S   += $(addprefix $(OUTPUT_DIR)/,$(ASMSOURCES_S:.S=.d))

C_PATHS   = $(subst \,/,$(sort $(dir $(C_SOURCE_FILES))))
CXX_PATHS = $(subst \,/,$(sort $(dir $(CXX_SOURCE_FILES))))
ASM_PATHS = $(subst \,/,$(sort $(dir $(ASM_SOURCE_FILES))))

vpath %.c $(C_PATHS)
vpath %.cpp $(CXX_PATHS)
vpath %.cc $(CXX_PATHS)
vpath %.s $(ASM_PATHS)
vpath %.S $(ASM_PATHS)

override CFLAGS = $(C_FLAGS) $(C_DEFS) $(INCLUDES) $(DEPFLAGS)
override CXXFLAGS = $(CXX_FLAGS) $(C_DEFS) $(INCLUDES) $(DEPFLAGS)
override ASMFLAGS = $(ASM_FLAGS) $(ASM_DEFS) $(INCLUDES) $(DEPFLAGS)

debug: C_FLAGS += $(C_FLAGS_DEBUG) 
debug: CXX_FLAGS += $(CXX_FLAGS_DEBUG)
debug: ASM_FLAGS += $(ASM_FLAGS_DEBUG)
debug: $(OUTPUT_DIR)/$(PROJECTNAME).out

release: C_FLAGS += $(C_FLAGS_RELEASE) 
release: CXX_FLAGS += $(CXX_FLAGS_RELEASE)
release: ASM_FLAGS += $(ASM_FLAGS_RELEASE)
release: $(OUTPUT_DIR)/$(PROJECTNAME).out

# include auto-generated dependency files (explicit rules)
ifneq (clean,$(findstring clean, $(MAKECMDGOALS)))
-include $(CDEPS)
-include $(CXXDEPS)
-include $(ASMDEPS_s)
-include $(ASMDEPS_S)
endif

$(OUTPUT_DIR)/$(PROJECTNAME).out: $(OBJS) $(LIB_FILES)
	@echo 'Linking $(OUTPUT_DIR)/$(PROJECTNAME)'
	$(ECHO)$(LD) $(LD_FLAGS) $(OBJS) $(LIBS) -o $(OUTPUT_DIR)/$(PROJECTNAME)
	@echo 'Done.'

$(OBJS):

$(OUTPUT_DIR)/%.o: %.c
	@echo 'Building $<'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(CFLAGS) -c -o $@ $<

$(OUTPUT_DIR)/%.o: %.cpp
	@echo 'Building $<'
	@mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OUTPUT_DIR)/%.o: %.cc
	@echo 'Building $<'
	@mkdir -p $(@D)
	$(ECHO)$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OUTPUT_DIR)/%.o: %.s
	@echo 'Building $<'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(ASMFLAGS) -c -o $@ $<

$(OUTPUT_DIR)/%.o: %.S
	@echo 'Building $<'
	@mkdir -p $(@D)
	$(ECHO)$(CC) $(ASMFLAGS) -c -o $@ $<

clean:
	$(RM) -rf $(BUILD_DIR)
