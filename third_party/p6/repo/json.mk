
##############################################################
# The following make variables may be set on the command-line
##############################################################

# Build configuraton to convery to JSON.
# - Debug or Release 
# - Optional, default value is Debug.
#
# CONFIG

# Selected toolchain
# - GCC_ARM, ARM, IAR, A_Clang
# - Optional, default is GCC_ARM
#
# TOOLCHAIN

# Selected target BSP.
# - Any legal TARGET bundled with the BSP.
# - Required (must be set on command-line when invoking "make chip_json TARGET=..."
#
# TARGET 

# Name of file to generate
# - Based on TARGET, TOOLCHAIN, and CONFIG.
# - $(CY_BUILD_LOC)/$(TARGET)/$(CONFIG)/$(TOOLCHAIN).json
# - E.g., build/CY8CPROTO-062-WIFI-BT/Debug/GCC_ARM.json
# May be set on 
#
MTB_CHIP_JSON_FILE?=$(CY_CONFIG_DIR)/$(TOOLCHAIN).json


##############################################################
# Internal state
##############################################################

_MTB_CHIP_JSON_ALL_C   =$(filter %.$(TOOLCHAIN_SUFFIX_C),$(SOURCE)) $(CY_SEARCH_AVAILABLE_C_SOURCES)
_MTB_CHIP_JSON_ALL_CPP =$(filter %.$(TOOLCHAIN_SUFFIX_CPP),$(SOURCE)) $(CY_SEARCH_AVAILABLE_CPP_SOURCES)
_MTB_CHIP_JSON_ALL_ASM =$(filter %.$(TOOLCHAIN_SUFFIX_S),$(SOURCE)) $(CY_SEARCH_AVAILABLE_S_SOURCES)
_MTB_CHIP_JSON_ALL_ASM+=$(filter %.$(TOOLCHAIN_SUFFIX_s),$(SOURCE)) $(CY_SEARCH_AVAILABLE_s_SOURCES)
_MTB_CHIP_JSON_ALL_LIB =$(filter %.$(TOOLCHAIN_SUFFIX_A),$(SOURCE)) $(CY_SEARCH_AVAILABLE_A_LIBS)
_MTB_CHIP_JSON_ALL_OBJ+=$(filter %.$(TOOLCHAIN_SUFFIX_O),$(SOURCE)) $(CY_SEARCH_AVAILABLE_O_SOURCES)

COMMA=,
INDENT=$(CY_SPACE)$(CY_SPACE)$(CY_SPACE)$(CY_SPACE)
SEP=$(COMMA)$(CY_NEWLINE)$(INDENT)


##############################################################
# Implemetation
##############################################################

# Autodiscovery of source code runs as part of the second stage build.
# This ensures prebuilds/generated source exist before we look for code.
ifneq ($(CY_SECOND_STAGE),true)
chip_json:
	$(MAKE) -f $(firstword $(MAKEFILE_LIST)) $(MAKECMDGOALS) TARGET=$(TARGET) CONFIG=$(CONFIG) TOOLCHAIN=$(TOOLCHAIN) CY_SECOND_STAGE=true chip_json
else
include $(CY_BASELIB_CORE_PATH)/make/core/search.mk
chip_json: $(MTB_CHIP_JSON_FILE)
endif

# core-make provides CY_MACRO_FILE_WRITE, but it doesn't support append.
ifeq ($(CY_FILE_TYPE),file)

# Escape ' and " as required for CY_MACRO_FILE_APPEND (some platforms require it, others do not).
CY_MACRO_FILE_ESC_SQUOTE=$1
CY_MACRO_FILE_ESC_DQUOTE=$1

# Always escape " as required by JSON.
_MTB_CHIP_JSON_ESC_DQUOTE=$(subst ",\",$1)

CY_MACRO_FILE_APPEND=$(file >>$1,$2)
else

# Escape ' and " as required for CY_MACRO_FILE_APPEND (some platforms require it, others do not).
CY_MACRO_FILE_ESC_SQUOTE=$(subst ',\',$1)
CY_MACRO_FILE_ESC_DQUOTE=$(subst ",\",$1)

# Inject newline via $(shell echo) w/o the $(shell) swallowing a litteral newline.
CY_MACRO_ESC_NEWLINE=$(subst $(CY_NEWLINE),"$$'\n'",$1)

# Always escape " as required by JSON.
_MTB_CHIP_JSON_ESC_DQUOTE=$(subst ",\\",$1)


# N.B., CY_MACRO_FILE_WRITE doesn't properly escape ". We don't need it for
# overwrite, but we do for append.
CY_MACRO_FILE_APPEND=$(shell echo "$(call CY_MACRO_ESC_NEWLINE,$(call CY_MACRO_FILE_ESC_DQUOTE,$2))" >>$1)
endif

$(MTB_CHIP_JSON_FILE): chip_mkdirs .FORCE
	$(info Generating '$(MTB_CHIP_JSON_FILE)')
	$(call CY_MACRO_FILE_WRITE,$(MTB_CHIP_JSON_FILE),{)
	$(call CY_MACRO_FILE_APPEND,$(MTB_CHIP_JSON_FILE),  "cflags":     $(call _chip_json_get_array,$(CY_RECIPE_CFLAGS))$(COMMA))
	$(call CY_MACRO_FILE_APPEND,$(MTB_CHIP_JSON_FILE),  "cxxflags":   $(call _chip_json_get_array,$(CY_RECIPE_CXXFLAGS))$(COMMA))
	$(call CY_MACRO_FILE_APPEND,$(MTB_CHIP_JSON_FILE),  "asflags":    $(call _chip_json_get_array,$(CY_RECIPE_ASFLAGS))$(COMMA))
	$(call CY_MACRO_FILE_APPEND,$(MTB_CHIP_JSON_FILE),  "ldflags":    $(call _chip_json_get_array,$(CY_RECIPE_LDFLAGS))$(COMMA))
	$(call CY_MACRO_FILE_APPEND,$(MTB_CHIP_JSON_FILE),  "includes":   $(call _chip_json_get_array,$(CY_RECIPE_INCLUDES))$(COMMA))
	$(call CY_MACRO_FILE_APPEND,$(MTB_CHIP_JSON_FILE),  "defines":    $(call _chip_json_get_array,$(CY_RECIPE_DEFINES))$(COMMA))
	$(call CY_MACRO_FILE_APPEND,$(MTB_CHIP_JSON_FILE),  "c_source":   $(call _chip_json_get_array,$(_MTB_CHIP_JSON_ALL_C))$(COMMA))
	$(call CY_MACRO_FILE_APPEND,$(MTB_CHIP_JSON_FILE),  "cxx_source": $(call _chip_json_get_array,$(_MTB_CHIP_JSON_ALL_CPP))$(COMMA))
	$(call CY_MACRO_FILE_APPEND,$(MTB_CHIP_JSON_FILE),  "asm_source": $(call _chip_json_get_array,$(_MTB_CHIP_JSON_ALL_ASM))$(COMMA))
	$(call CY_MACRO_FILE_APPEND,$(MTB_CHIP_JSON_FILE),  "libs":       $(call _chip_json_get_array,$(_MTB_CHIP_JSON_ALL_LIB))$(COMMA))
	$(call CY_MACRO_FILE_APPEND,$(MTB_CHIP_JSON_FILE),  "objs":       $(call _chip_json_get_array,$(_MTB_CHIP_JSON_ALL_OBJ)))
	$(call CY_MACRO_FILE_APPEND,$(MTB_CHIP_JSON_FILE),})

chip_mkdirs:
	@mkdir -p $(CY_CONFIG_DIR)

# Print JSON form of make list variable data.
# - This directly handles empty lists.
# - Dispatches to _chip_json_get_array_single for lists of size 1.
# - Dispatches to _chip_json_get_array_multi for lists of size > 1.
# - This always generates the opening and closing [].
_chip_json_get_array=$(if $(filter 0,$(words $1)),[ ],$(if $(filter 1,$(words $1)),$(call _chip_json_get_array_single,$1),[$(CY_NEWLINE)$(INDENT)$(call _chip_json_get_array_multi,$1)]))

# Handle list of size 1
_chip_json_get_array_single=[ "$(call _MTB_CHIP_JSON_ESC_DQUOTE,$1)" ]

# Handle lists of size > 1
# - pre-condition: any leading whitespace/positionig has already been handled for the first element.
# - need to escape any " found in any values we're printing.
# - line N is responsible for trailing ,<newline><indend> required to setup the next entry.
# - final entry is responsible for trailing <newline>indent> to setup the closing ]
# - opening and closing [] are handled by caller.
_chip_json_get_array_multi=$(if $(filter 1,$(words $1)),"$(call _MTB_CHIP_JSON_ESC_DQUOTE,$1)"$(CY_NEWLINE)$(INDENT),"$(firstword $(call _MTB_CHIP_JSON_ESC_DQUOTE,$1))"$(SEP)$(call _chip_json_get_array_multi,$(wordlist 2,$(words $1),$1)))

# Anything target that depends on this will always run.
.FORCE:

