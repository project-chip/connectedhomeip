#
#    Copyright (c) 2019 Google LLC. All Rights Reserved.
#    Copyright (c) 2018 Nest Labs Inc. All Rights Reserved.
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
#      This file is the generic "header" or pre make header for all
#      common host-specific (i.e., non-target and -toolchain-specific)
#      tools included in any makefile used in the build tree.
#

CAT                              ?= cat
CHMOD                            ?= chmod
CMP                              ?= cmp
CUT                              ?= cut
DIFF                             ?= diff
FIND                             ?= find
GIT                              ?= git
GREP                             ?= grep
GZIP                             ?= gzip
MKDIR                            ?= mkdir
MKDIR_P                          ?= $(MKDIR) -p                  
MV                               ?= mv

RMFLAGS                           = -f
ifeq ($(V),1)
RMFLAGS                          += -v
endif
RM                               ?= rm $(RMFLAGS)

RMDIR                            ?= rmdir
SED                              ?= sed
SORT                             ?= sort
TAR                              ?= tar
UNIQ                             ?= uniq
XZ                               ?= xz

#
# nl-create-dir <directory>
#
# Create the specified directory, including any parent directories
# that may not exist.
#
define nl-create-dir
$(NL_V_PROGRESS_MKDIR)
$(NL_V_AT)$(MKDIR_P) "$(1)"
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
$(NL_V_PROGRESS_RMDIR)
$(NL_V_AT)if [ -d "$(1)" ]; then \
    $(FIND) "$(1)" -type d ! -perm -200 -exec $(CHMOD) u+w {} ';' \
    && $(RM) -rf "$(1)" \
    || { sleep 5 && $(RM) -rf "$(1)"; }; \
fi
endef # nl-remove-dir

#
# nl-check-file-with-subroutine <subroutine prefix> <macro suffix>
#
# Check whether a file, referenced by the $(@) variable, should be
# updated / regenerated based on its dependencies, referenced by the
# $(<) variable by running the make macro check-file-<macro suffix>.
#
# The $(<) is passed as the first argument if the macro wants to process
# it and the prospective new output file, which the macro MUST
# generate, as the second.
#
# This macro will ensure that any required parent directories are created
# prior to invoking the subroutine <subroutine prefix>-<macro suffix>.
#
# This macro is similar to and inspired by that from Linux Kbuild and
# elsewhere.
#
#   <subroutine prefix> - The subroutine name, prefixed to "-<macro suffix>",
#                         which, together with <macro suffix>, indicates the
#                         make macro to invoke to actually check the file.
#
#   <macro suffix>      - The name, suffixed to "<subroutine prefix>-", which
#                         together with <subroutine prefix>, indicates the
#                         make macro to invoke to actually check the file.
#
#
define nl-check-file-with-subroutine
$(NL_V_AT)set -e;                                   \
$(_NL_PROGRESS) "CHECK" "$(@)";                     \
$(MKDIR_P) $(dir $(@));                             \
$(call $(1)-$(2),$(<),$(@).N);                      \
if [ -r "$(@)" ] && $(CMP) -s "$(@)" "$(@).N"; then \
    rm -f "$(@).N";                                 \
else                                                \
    $(_NL_PROGRESS) "GEN" "$(@)";                   \
    mv -f "$(@).N" "$(@)";                          \
fi
endef # nl-check-file

#
# nl-check-file <macro suffix>
#
# Check whether a file, referenced by the $(@) variable, should be
# updated / regenerated based on its dependencies, referenced by the
# $(<) variable by running the make macro check-file-<macro suffix>.
#
# The $(<) is passed as the first argument if the macro wants to process
# it and the prospective new output file, which the macro MUST
# generate, as the second.
#
# This macro will ensure that any required parent directories are created
# prior to invoking check-file-<macro suffix>.
#
# This macro is similar to and inspired by that from Linux Kbuild and
# elsewhere.
#
#   <macro suffix> - The name, suffixed to "check-file-", which indicates
#                    the make macro to invoke.
#
#
define nl-check-file
$(call nl-check-file-with-subroutine,nl-check-file,$(1))
endef # nl-check-file

#
# nl-create-link
#
# Create the symbolic link with the source of the $(<) variable and
# the destination of the $(@) variable, using the LN_S macro.
#
define nl-create-link
$(NL_V_PROGRESS_LN_S)
$(NL_V_AT)$(LN_S) $(<) $(@)
endef # nl-create-link
