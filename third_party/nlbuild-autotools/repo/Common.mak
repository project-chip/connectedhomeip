#
#    Copyright 2017 Nest Labs Inc. All Rights Reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

#
#    Description:
#      This make file header defines common variables, rules, and
#      targets for maintaining nlbuild-autotools distributions.
#

.DEFAULT_GOAL         = all

#
# This package
#
PACKAGE              := nlbuild-autotools

#
# Tools
#
CAT                  ?= cat
CHMOD                ?= chmod
CMP                  ?= cmp
FIND                 ?= find
GZIP                 ?= gzip
MKDIR                ?= mkdir
MV                   ?= mv
RM                   ?= rm
SED                  ?= sed
TAR                  ?= tar
XZ                   ?= xz

dist_tar_ARCHIVE      = $(TAR) -chof -

dist_tgz_ARCHIVE      = $(dist_tar_ARCHIVE)
dist_tgz_COMPRESS     = $(GZIP) --best -c

dist_txz_ARCHIVE      = $(dist_tar_ARCHIVE)
dist_txz_COMPRESS     = $(XZ) --extreme -c

TGZ_EXTENSION        := .tar.gz
TXZ_EXTENSION        := .tar.xz

DIST_TARGETS	     ?= dist-tgz dist-txz
DIST_ARCHIVES         = $(dist_tgz_TARGETS) $(dist_txz_TARGETS)

#
# Verbosity
#

NL_DEFAULT_VERBOSITY  = 0

NL_V_AT               = $(NL_V_AT_$(V))
NL_V_AT_              = $(NL_V_AT_$(NL_DEFAULT_VERBOSITY))
NL_V_AT_0             = @
NL_V_AT_1             = 

NL_V_MAKE             = $(NL_V_MAKE_$(V))
NL_V_MAKE_            = $(NL_V_MAKE_$(NL_DEFAULT_VERBOSITY))
NL_V_MAKE_0           = @echo "  MAKE     $(@)";
NL_V_MAKE_1           = 

NL_V_MKDIR_P          = $(NL_V_MKDIR_P_$(V))
NL_V_MKDIR_P_         = $(NL_V_MKDIR_P_$(NL_DEFAULT_VERBOSITY))
NL_V_MKDIR_P_0        = @echo "  MKDIR    $(1)";
NL_V_MKDIR_P_1        = 

NL_V_RMDIR            = $(NL_V_RMDIR_$(V))
NL_V_RMDIR_           = $(NL_V_RMDIR_$(NL_DEFAULT_VERBOSITY))
NL_V_RMDIR_0          = @echo "  RMDIR    $(1)";
NL_V_RMDIR_1          = 

NL_V_TGZ              = $(NL_V_TGZ_$(V))
NL_V_TGZ_             = $(NL_V_TGZ_$(NL_DEFAULT_VERBOSITY))
NL_V_TGZ_0            = @echo "  TGZ      $(@)";
NL_V_TGZ_1            = 

NL_V_TXZ              = $(NL_V_TXZ_$(V))
NL_V_TXZ_             = $(NL_V_TXZ_$(NL_DEFAULT_VERBOSITY))
NL_V_TXZ_0            = @echo "  TXZ      $(@)";
NL_V_TXZ_1            = 

#
# nl-create-dir <directory>
#
# Create the specified directory, including any parent directories
# that may not exist.
#
define nl-create-dir
$(NL_V_AT)echo "  MKDIR    $(1)"; \
$(MKDIR) -p "$(1)"
endef # nl-create-dir

#
# nl-remove-dir <directory>
#
# If the specified directory exists, then ensure all of the
# directories are writable by the current user, and then forcibly
# remove the directory and all of its contents, sleeping for five (5)
# seconds and failure before trying the removal again.
#
define nl-remove-dir
$(NL_V_RMDIR) \
if [ -d "$(1)" ]; then \
    $(FIND) "$(1)" -type d ! -perm -200 -exec $(CHMOD) u+w {} ';' \
    && $(RM) -rf "$(1)" \
    || { sleep 5 && $(RM) -rf "$(1)"; }; \
fi
endef # nl-remove-dir

clean-local:
	$(NL_V_AT)$(RM) -f *~ "#"*

help:
	@echo "This make file shold not be needed for end users and system "
	@echo "integrators of $(PACKAGE). It should only be needed by "
	@echo "maintainers producing distributions of $(PACKAGE)."
