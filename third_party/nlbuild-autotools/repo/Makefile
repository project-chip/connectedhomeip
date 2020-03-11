#
#    Copyright (c) 2017-2018 Nest Labs Inc. All Rights Reserved.
#    Copyright (c) 2018 Google LLC. All Rights Reserved.
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
#      This make file supports generating distributions of
#      nlbuild-autotools.
#

include Common.mak

#
# Build directories
#

builddir         := .
top_builddir     := .
abs_builddir     := $(CURDIR)
abs_top_builddir := $(CURDIR)

#
# Source directories
#

srcdir           := .
top_srcdir       := .
abs_srcdir       := $(CURDIR)
abs_top_srcdir   := $(CURDIR)

distdir           = $(PACKAGE)-$(VERSION)

dist_tgz_TARGETS  = $(distdir)$(TGZ_EXTENSION)
dist_txz_TARGETS  = $(distdir)$(TXZ_EXTENSION)

DISTFILES        := $(shell $(CAT) MANIFEST)

#
# Package version files:
#
# .default-version - The default package version. This file is ALWAYS checked
#                    in and should always represent the current baseline
#                    version of the package.
#
# .dist-version    - The distributed package version. This file is NEVER
#                    checked in within the upstream repository, is auto-
#                    generated, and is only found in the package distribution.
#
# .local-version   - The current source code controlled package version. This
#                    file is NEVER checked in within the upstream repository,
#                    is auto-generated, and can always be found in both the
#                    build tree and distribution.
#
# When present, the .local-version file is preferred first, the
# .dist-version second, and the .default-version last.
#

# VERSION_FILE should be and is intentionally an immediate (:=) rather
# than a deferred (=) variable to ensure the value binds once and only once
# for a given MAKELEVEL even as .local-version and .dist-version are created
# during makefile execution.

VERSION_FILE                      := $(if $(wildcard $(builddir)/.local-version),$(builddir)/.local-version,$(if $(wildcard $(srcdir)/.dist-version),$(srcdir)/.dist-version,$(srcdir)/.default-version))

#
# The two-level variables and the check against MAKELEVEL ensures that
# not only can the package version be overridden from the command line
# but also when the version is NOT overridden that we bind the version
# once and only once across potential sub-makes to prevent the version
# from flapping as VERSION_FILE changes.
#

export MAYBE_PACKAGE_VERSION      := $(if $(filter 0,$(MAKELEVEL)),$(shell cat $(VERSION_FILE) 2> /dev/null),$(MAYBE_PACKAGE_VERSION))

PACKAGE_VERSION                   ?= $(MAYBE_PACKAGE_VERSION)

VERSION                            = $(PACKAGE_VERSION)

#
# Verbosity
#
_NL_V_COPY                         = $(_NL_V_COPY_$(V))
_NL_V_COPY_                        = $(_NL_V_COPY_$(NL_DEFAULT_VERBOSITY))
_NL_V_COPY_0                       = @for file in $(DISTFILES); do echo "  COPY     $${file}"; done;
_NL_V_COPY_1                       = 

_NL_V_MAKE                         = $(_NL_V_MAKE_$(V))
_NL_V_MAKE_                        = $(_NL_V_MAKE_$(NL_DEFAULT_VERBOSITY))
_NL_V_MAKE_0                       = @echo "  MAKE     dist-hook";
_NL_V_MAKE_1                       = 

#
# check-file <macro suffix>
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
define check-file
$(NL_V_AT)set -e;                                   \
echo '  CHECK    $(@)';                             \
$(MKDIR) -p $(dir $(@));                            \
$(call check-file-$(1),$(<),$(@).N);                \
if [ -r "$(@)" ] && $(CMP) -s "$(@)" "$(@).N"; then \
    $(RM) -f "$(@).N";                              \
else                                                \
    echo '  GEN      $(@)';                         \
    $(MV) -f "$(@).N" "$(@)";                       \
fi
endef # check-file

#
# check-file-.local-version
#
# Speculatively regenerate .local-version and check to see if it needs
# to be updated.
#
# If PACKAGE_VERSION has been supplied anywhere other than in this file
# (which is implicitly the contents of .local-version), then use that;
# otherwise, attempt to generate it from the SCM system.
#
# This is called from $(call check-file,.local-version).
#
define check-file-.local-version
if [ "$(origin PACKAGE_VERSION)" != "file" ]; then     \
    echo "$(PACKAGE_VERSION)" > "$(2)";                \
else                                                   \
    $(abs_top_srcdir)/scripts/mkversion                \
        -b "$(PACKAGE_VERSION)" "$(top_srcdir)"        \
        > "$(2)";                                      \
fi
endef

#
# check-file-.dist-version
#
# Speculatively regenerate .dist-version and check to see if it needs
# to be updated.
#
# This is called from $(call check-file,.dist-version).
#
define check-file-.dist-version
$(CAT) "$(1)" > "$(2)"
endef

#
# Version file regeneration rules.
#
.PHONY: force

$(builddir)/.local-version: $(srcdir)/.default-version force

$(distdir)/.dist-version: $(builddir)/.local-version force

$(distdir)/.dist-version $(builddir)/.local-version:
	$(call check-file,$(@F))

all: .local-version

dist-hook: $(distdir)/.dist-version

#
# Stage the distribution files to a distribution directory
#
stage: $(DISTFILES) .local-version
	$(call nl-remove-dir,$(distdir))
	$(call nl-create-dir,$(distdir))
	$(_NL_V_MAKE)$(MAKE) -s distdir="$(distdir)" dist-hook
	$(_NL_V_COPY)(cd $(abs_top_srcdir); $(dist_tar_ARCHIVE) $(DISTFILES) | (cd $(abs_builddir)/$(distdir); $(TAR) xfBp -))

#
# Produce an architecture-independent distribution using a tar archive
# with gzip compression
#
$(dist_tgz_TARGETS): stage
	$(NL_V_TGZ)$(dist_tgz_ARCHIVE) $(distdir) | $(dist_tgz_COMPRESS) > "$(@)"

#
# Produce an architecture-independent distribution using a tar archive
# with xz compression
#
$(dist_txz_TARGETS): stage
	$(NL_V_TXZ)$(dist_txz_ARCHIVE) $(distdir) | $(dist_txz_COMPRESS) > "$(@)"

#
# Produce an architecture-independent distribution of the
# nlbuild-autotools core.
#
dist: $(DIST_TARGETS) .local-version
	$(call nl-remove-dir,$(distdir))

dist-tgz: $(dist_tgz_TARGETS)

dist-txz: $(dist_txz_TARGETS)

#
# Produce prebuilt GNU autotools binaries for the architecture of the
# CURRENT build machine and install them in THIS nlbuild-autotools
# package.
#
.PHONY: tools
tools:
	$(NL_V_MAKE)$(MAKE) -C tools $(@)

#
# Produce prebuilt GNU autotools architecture-dependent and -independent
# binaries for the architecture of the CURRENT build machine and PACKAGE
# them up for EXTERNAL distribution.
#
toolsdist: .local-version
	$(NL_V_MAKE)$(MAKE) -C tools $(@)

clean: clean-local
	$(NL_V_MAKE)$(MAKE) -C tools $(@)
