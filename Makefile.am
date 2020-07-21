#
#    Copyright (c) 2020 Project CHIP Authors
#    Copyright (c) 2014-2018 Nest Labs, Inc.
#    Copyright (c) 2018 Google LLC
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

#
#    Description:
#      This file is the GNU automake template for the CHIP SDK.
#

include $(abs_top_nlbuild_autotools_dir)/automake/pre.am

# To ensure that the .local-version file and subsequent rechecks work
# correctly, don't allow this top-level makefile to run in parallel.

.NOTPARALLEL:

AM_MAKEFLAGS                             = --no-print-directory

SUBDIRS                                  = \
    examples                               \
    third_party                            \
    src                                    \
    docs                                   \
    $(NULL)

PRETTY_SUBDIRS                           = \
    $(filter-out third_party,$(SUBDIRS))   \
    $(NULL)

EXTRA_DIST                               = \
    CODE_OF_CONDUCT.md                     \
    CONTRIBUTING.md                        \
    Makefile-Android                       \
    Makefile-bootstrap                     \
    Makefile-Standalone                    \
    README.md                              \
    .default-version                       \
    bootstrap                              \
    bootstrap-configure                    \
    repos.conf                             \
    .gitmodules                            \
    $(srcdir)/build/autoconf               \
    $(srcdir)/config                       \
    $(srcdir)/config/efr32                 \
    $(srcdir)/config/nrf5                  \
    $(srcdir)/scripts                      \
    $(NULL)

BUILT_SOURCES                            = \
    .local-version                         \
    $(NULL)

dist_doc_DATA                            = \
    LICENSE                                \
    $(NULL)

DISTCLEANFILES                           = \
    .local-version                         \
    .local-version.min                     \
    $(NULL)

# There are no source files to lint or prettify in this subdirectory.

PRETTY_FILES                            := $(NULL)

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
# Override autotool's default notion of the package version variables.
# This ensures that when we create a source distribution the
# version is always the current version, not the package bootstrap
# version.
#
# The include ensures that we update CHIP_VERSION and VERSION once
# we've built .local-version (the source of truth unless CHIP_VERSION
# is specified on the make command line).
#
# CHIP_VERSION can be overridden from the command line, but when the version is NOT
# overridden that we bind the version once and only once across potential
# sub-makes to prevent the version from flapping as VERSION_FILE changes.
#
# .local-version.min sets CHIP_VERSION
-include .local-version.min

PACKAGE_VERSION                    = $(CHIP_VERSION)
VERSION                            = $(PACKAGE_VERSION)


#
# check-file-.local-version
#
# Speculatively regenerate .local-version and check to see if it needs
# to be updated.
#
# If CHIP_VERSION has been supplied anywhere other than in this file
# (which is implicitly the contents of .local-version), then use that;
# otherwise, attempt to generate it from the SCM system.
#
# This is called from $(call check-file,.local-version).
#
define check-file-.local-version
$(if $(filter-out file,$(origin CHIP_VERSION)),\
    echo "$(CHIP_VERSION)" > "$(2)",\
    $(abs_top_nlbuild_autotools_dir)/scripts/mkversion \
        -b "$(CHIP_VERSION)" "$(top_srcdir)"          \
        > "$(2)")
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
cat "$(1)" > "$(2)"
endef

#
# A convenience target to allow package users to easily rerun the
# package configuration according to the current configuration.
#
.PHONY: reconfigure
reconfigure: $(builddir)/config.status
	$(AM_V_at)$(<) --recheck

#
# Version file regeneration rules.
#
.PHONY: force

$(builddir)/.local-version: $(srcdir)/.default-version force

$(distdir)/.dist-version: $(builddir)/.local-version force

$(distdir)/.dist-version $(builddir)/.local-version:
	$(call check-file,$(@F))

$(builddir)/.local-version.min: $(builddir)/.local-version
	(printf "export CHIP_VERSION=" ; cat) < "$(VERSION_FILE)" > $@


#
# When we run 'distcheck' and --with-<any of the third-party packages
# listed in repos.conf>, default to 'internal', the nlbuild-autotools
# infrastructure will attempt to create git paths to manage the
# package repo. Two directories need to be writable to facilitate
# this.
#

DISTCHECK_CONFIGURE_FLAGS = `chmod u+w ../.. ../../third_party`

#
# BlueZ is not well set-up without patching to handle 'make distcheck';
# disable it for that target.
#

DISTCHECK_CONFIGURE_FLAGS += --without-bluez

all-recursive check-recursive coverage-recursive install-recursive pretty-recursive pretty-check-recursive dist distcheck distdir install-headers: $(BUILT_SOURCES)

dist-hook: $(distdir)/.dist-version

# If you are synchronizing a package on which yours depends using 'repos.conf',
# to 'third_party', uncomment and adapt or delete this, as needed. If you choose
# to use this, please also take a look at Makefile.am and third_party/Makefile.am
# and uncomment the appropriate sections there.

#
# Ensure any locally synchronized repositories defined by 'repos.conf'
# are cleaned up.
#
distclean-local:
	$(MAKE) -C $(srcdir) -f Makefile-bootstrap clean-repos

#
# Top-level convenience target for making documentation.
#

.PHONY: doc
doc: $(BUILT_SOURCES)
	$(MAKE) -C docs $(@)

#
# Top-level convenience target for making a documentation-only
# distribution whose results appear at the top level of the build tree
# in the same fashion that the distribution would be for 'make dist'.
#

.PHONY: docdist
docdist: $(BUILT_SOURCES)
	$(MAKE) -C docs docdistdir=$(abs_builddir) $(@)

#
# Top-level convenience target for making a tools-only distribution
# whose resuls appear at the top level of the build tree in the same
# fashion that the distribution would be for 'make dist'.
#

tooldistdir                                     ?= $(abs_builddir)

chip_tooldist_alias                             = \
    $(PACKAGE_TARNAME)-tools

chip_tooldist_name                              = \
    $(chip_tooldist_alias)-$(shell echo $(target) | $(SED) -e 's/[[:digit:].]*$$//g')-$(CHIP_VERSION)

chip_tooldist_archive                           = \
    $(tooldistdir)/$(chip_tooldist_name).tar.gz

CLEANFILES                                       = \
    $(chip_tooldist_archive)                      \
    $(NULL)

$(chip_tooldist_name):
	$(AM_V_at)rm -f -r "$(@)"
	$(call create-directory)
	$(MAKE) DESTDIR=$(tooldistdir)/$(chip_tooldist_name) install
	$(AM_V_at)rm -f -r         \
	    "$(@)/include"         \
	    "$(@)/lib/engines"     \
	    "$(@)/lib/libcrypto.a" \
	    "$(@)/lib/libssl.a"    \
	    "$(@)/lib/pkgconfig"   \
	    "$(@)/openssl"         \
	    "$(@)/share/java"

$(chip_tooldist_archive): $(chip_tooldist_name)
	$(AM_V_at)echo "  TAR      $(@)"
	$(AM_V_at)tardir="$(<)" && $(am__tar) | GZIP=$(GZIP_ENV) gzip -c > "$(@)" && rm -rf "$(<)"

.PHONY: tooldist
tooldist $(chip_tooldist_alias): $(chip_tooldist_archive)

#
# By virtue of the way we allow for the creation of both libChip and
# libInetLayer, using the same makefile fragment included in the
# Makefile.am of each of those libraries, running 'make distclean'
# will fail in src/lib after running in src/inet because the
# dependencies for the latter have already been cleaned for the
# former.
#
# In general, we don't really care about this specifically or if a
# particular step of distclean fails generally, so propagate the
# --ignore-errors and --keep-going flags for make on recursive
# distclean to avoid this.
#

distclean-recursive: AM_MAKEFLAGS += -ik

include $(abs_top_nlbuild_autotools_dir)/automake/post.am
