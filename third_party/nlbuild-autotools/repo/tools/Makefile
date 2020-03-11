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
#      This make file supports generating prebuilt architecture
#      -independent and -dependent distributions of nlbuild-autotools
#      binaries for the architeture of the current build host.
#

include ../Common.mak

#
# Build directories
#

builddir         := .
top_builddir     := ..
abs_builddir     := $(CURDIR)
abs_top_builddir := $(abspath $(top_builddir))

#
# Source directories
#

srcdir           := .
top_srcdir       := ..
abs_srcdir       := $(CURDIR)
abs_top_srcdir   := $(abspath $(top_srcdir))

# Figure out what sort of build host we are running on, stripping off
# any trailing version number information typically included on Darwin
# / Mac OS X.

host             := $(shell $(top_srcdir)/third_party/autoconf/config.guess | $(SED) -e 's/[[:digit:].]*$$//g')

distdir           = $(PACKAGE)-$(VERSION)

dist_tgz_TARGETS  = ${top_builddir}/${PACKAGE}-common-$(VERSION)$(TGZ_EXTENSION) \
                    ${top_builddir}/${PACKAGE}-${host}-$(VERSION)$(TGZ_EXTENSION)

dist_txz_TARGETS  = ${top_builddir}/${PACKAGE}-common-$(VERSION)$(TXZ_EXTENSION) \
                    ${top_builddir}/${PACKAGE}-${host}-$(VERSION)$(TXZ_EXTENSION)

dist_common_DIRS  = share include
dist_arch_DIRS    = $(host)

PACKAGE_VERSION   = $(shell $(CAT) $(top_builddir)/.local-version)

VERSION           = $(PACKAGE_VERSION)

DIST_TMPDIR      := $(shell mktemp -u -d /tmp/tmp.$(PACKAGE)-$(VERSION)XXXXXX)
TOOLS_TMPDIR     := $(shell mktemp -u -d /tmp/tmp.$(PACKAGE)-$(VERSION)XXXXXX)

#
# Verbosity
#
_NL_V_BUILD       = $(_NL_V_BUILD_$(V))
_NL_V_BUILD_      = $(_NL_V_BUILD_$(NL_DEFAULT_VERBOSITY))
_NL_V_BUILD_0     = @echo "  BUILD";
_NL_V_BUILD_1     = 

dist-tgz: $(dist_tgz_TARGETS)

dist-txz: $(dist_txz_TARGETS)

#
# Produce prebuilt GNU autotools binaries for the architecture of the
# current build machine and install them in this nlbuild-autotools
# package.
#
tools:
	$(call nl-remove-dir,$(TOOLS_TMPDIR))
	$(call nl-create-dir,$(TOOLS_TMPDIR))
	$(_NL_V_BUILD)${srcdir}/packages/build --arch ${host} --srcdir ${abs_srcdir}/packages --builddir "${TOOLS_TMPDIR}" --destdir "${abs_top_srcdir}/tools/host"
	$(call nl-remove-dir,$(TOOLS_TMPDIR))

#
# Produce prebuilt GNU autotools architecture-dependent and -independent
# binaries for the architecture of the current build machine and package
# them up for distribution.
#
dist toolsdist: $(DIST_TARGETS)
	$(call nl-remove-dir,$(DIST_TMPDIR))

#
# Stage the distribution files to a distribution directory
#
stage:
	$(call nl-remove-dir,$(DIST_TMPDIR))
	$(call nl-create-dir,$(DIST_TMPDIR))
	$(_NL_V_BUILD)${srcdir}/packages/build --arch ${host} --srcdir ${abs_srcdir}/packages --builddir "${DIST_TMPDIR}" --destdir "${DIST_TMPDIR}"

#
# Produce prebuilt GNU autotools architecture-independent binaries for
# the architecture of the current build machine and package them up
# for distribution in tar-gzip format.
#
${top_builddir}/${PACKAGE}-common-$(VERSION)$(TGZ_EXTENSION): stage
	$(NL_V_TGZ)$(dist_tgz_ARCHIVE) --directory ${DIST_TMPDIR} $(dist_common_DIRS) | $(dist_tgz_COMPRESS) > "$(@)"

#
# Produce prebuilt GNU autotools architecture-dependent binaries for
# the architecture of the current build machine and package them up
# for distribution in tar-gzip format.
#
${top_builddir}/${PACKAGE}-${host}-$(VERSION)$(TGZ_EXTENSION): stage
	$(NL_V_TGZ)$(dist_tgz_ARCHIVE) --directory ${DIST_TMPDIR} $(dist_arch_DIRS) | $(dist_tgz_COMPRESS) > "$(@)"

#
# Produce prebuilt GNU autotools architecture-independent binaries for
# the architecture of the current build machine and package them up
# for distribution in tar-xz format.
#
${top_builddir}/${PACKAGE}-common-$(VERSION)$(TXZ_EXTENSION): stage
	$(NL_V_TXZ)$(dist_txz_ARCHIVE) --directory ${DIST_TMPDIR} $(dist_common_DIRS) | $(dist_txz_COMPRESS) > "$(@)"

#
# Produce prebuilt GNU autotools architecture-dependent binaries for
# the architecture of the current build machine and package them up
# for distribution in tar-xz format.
#
${top_builddir}/${PACKAGE}-${host}-$(VERSION)$(TXZ_EXTENSION): stage
	$(NL_V_TXZ)$(dist_txz_ARCHIVE) --directory ${DIST_TMPDIR} $(dist_arch_DIRS) | $(dist_txz_COMPRESS) > "$(@)"

clean: clean-local
