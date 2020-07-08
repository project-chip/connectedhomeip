#
#    Copyright (c) 2020 Project CHIP Authors
#    Copyright (c) 2014-2017 Nest Labs, Inc.
#    All rights reserved.
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
#    Description:
#      This file is the GNU automake template for the CHIP
#      API and stack documentation.
#

include $(abs_top_nlbuild_autotools_dir)/automake/pre.am

EXTRA_DIST                                               = \
    $(srcdir)/Doxyfile.in                                  \
    $(srcdir)/ChipDoxygenLayout.xml                        \
    $(top_srcdir)/scripts/helpers/doxygen-format-check.sh  \
    $(srcdir)/namespaces.dox                               \
    $(NULL)

#
# Override autotool's default notion of the package version variables.
# This ensures that when we create a doc distribution that the
# version is always the current version, not the version when the
# package was bootstrapped.
#
PACKAGE_VERSION                                 = $(shell cat $(top_builddir)/.local-version)
VERSION                                         = $(PACKAGE_VERSION)

EXTRA_DOCS                                      = \
    $(top_srcdir)/LICENSE                         \
    $(NULL)

docdistdir                                     ?= .

chip_docdist_alias                             = \
    $(PACKAGE_TARNAME)-docs

chip_docdist_name                              = \
    $(chip_docdist_alias)-$(VERSION)

chip_docdist_archive                           = \
    $(docdistdir)/$(chip_docdist_name).tar.gz

CLEANFILES                                      = \
    Doxyfile                                      \
    $(chip_docdist_archive)                      \
    $(NULL)

.PHONY: doc docdist

if CHIP_BUILD_DOCS

all-local doc: html/index.html

#
# We choose to manually transform Doxyfile.in into Doxyfile here in
# the makefile rather than in the configure script so that we can take
# advantage of live, at build time (rather than at configure time),
# updating of the package version number.
#

Doxyfile: $(srcdir)/Doxyfile.in Makefile
	$(AM_V_GEN)$(SED)                                     \
	    -e "s,\@DOXYGEN_USE_DOT\@,$(DOXYGEN_USE_DOT),g"   \
	    -e "s,\@PACKAGE_NAME\@,$(PACKAGE_NAME),g"         \
	    -e "s,\@PACKAGE_VERSION\@,$(PACKAGE_VERSION),g"   \
	    -e "s,\@PERL\@,$(PERL),g"                         \
	    -e "s,\@abs_builddir\@,$(abs_builddir),g"         \
	    -e "s,\@abs_srcdir\@,$(abs_srcdir),g"             \
	    -e "s,\@abs_top_builddir\@,$(abs_top_builddir),g" \
	    -e "s,\@abs_top_srcdir\@,$(abs_top_srcdir),g"     \
	    < "$(srcdir)/Doxyfile.in" > "$(@)"

html/index.html: Doxyfile
	$(AM_V_GEN)$(DOXYGEN) $(<)

#
# Addition rules and commands to create a documentation-only
# distribution of Chip
#

$(chip_docdist_name): $(EXTRA_DOCS) html/index.html
	$(AM_V_at)rm -f -r $(@)
	$(call create-directory)
	$(AM_V_at)cp $(EXTRA_DOCS) $(@)
	$(AM_V_at)cp -R html $(@)

$(chip_docdist_archive): $(chip_docdist_name)
	$(AM_V_at)echo "  TAR      $(@)"
	$(AM_V_at)tardir="$(<)" && $(am__tar) | GZIP=$(GZIP_ENV) gzip -c > "$(@)" && rm -rf $(<)

docdist $(chip_docdist_alias): $(chip_docdist_archive)

clean-local:
	$(AM_V_at)rm -f -r html

else

doc docdist:
	$(error "Documentation support is disabled. Please explicitly enable documentation with `--enable-docs` via `configure` and/or ensure that Doxygen is installed")

endif # CHIP_BUILD_DOCS

include $(abs_top_nlbuild_autotools_dir)/automake/post.am
