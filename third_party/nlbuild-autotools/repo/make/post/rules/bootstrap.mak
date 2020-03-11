#
#    Copyright 2018 Nest Labs Inc. All Rights Reserved.
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
#      This file is the make footer for nlbuild-autotools bootstrap
#      convenience targets.
#

# The following targets provide some convenience targets for bootstrapping
# the GNU autotools-based build system used by this package.

all check coverage dist distcheck doc docdist install install-headers pretty pretty-check: Makefile
	$(NL_V_PROGRESS_MAKE)
	$(MAKE) -f $(<) --no-print-directory $(@)

Makefile: $(top_srcdir)/Makefile.in $(top_srcdir)/configure
	$(NL_V_PROGRESS_CONFIGURE)
	$(top_srcdir)/configure

$(top_srcdir)/configure: $(top_srcdir)/configure.ac
	$(NL_V_PROGRESS_BOOTSTRAP_CONFIG)
	$(BOOTSTRAP) -w config

Makefile.in: Makefile.am
	$(NL_V_PROGRESS_BOOTSTRAP_MAKE)
	$(BOOTSTRAP) -w make

.PHONY: bootstrap
bootstrap:
	$(NL_V_PROGRESS_BOOTSTRAP_ALL)
	$(BOOTSTRAP) -w all

.PHONY: bootstrap-config
bootstrap-config:
	$(NL_V_PROGRESS_BOOTSTRAP_CONFIG)
	$(BOOTSTRAP) -w config

.PHONY: bootstrap-make
bootstrap-make:
	$(NL_V_PROGRESS_BOOTSTRAP_MAKE)
	$(BOOTSTRAP) -w make

define PrintBootstrapHelp
$(NL_V_AT)echo "  all"
$(NL_V_AT)echo "    Generate all configured build artifacts for this project."
$(NL_V_AT)echo
$(NL_V_AT)echo "  bootstrap"
$(NL_V_AT)echo "    (Re-)generate all build infrastructure for the project, "
$(NL_V_AT)echo "    including both build configuration scripts and makefiles."
$(NL_V_AT)echo
$(NL_V_AT)echo "  bootstrap-all"
$(NL_V_AT)echo "    (Re-)generate all build infrastructure for the project, "
$(NL_V_AT)echo "    including both build configuration scripts and makefiles."
$(NL_V_AT)echo
$(NL_V_AT)echo "  bootstrap-config"
$(NL_V_AT)echo "    (Re-)generate build configuration scripts for the project."
$(NL_V_AT)echo
$(NL_V_AT)echo "  bootstrap-make"
$(NL_V_AT)echo "    (Re-)generate build makefiles for the project."
$(NL_V_AT)echo
$(NL_V_AT)echo "  check"
$(NL_V_AT)echo "    Generate all configured build artifacts and run all unit "
$(NL_V_AT)echo "    and functional tests for this project."
$(NL_V_AT)echo
$(NL_V_AT)echo "  coverage"
$(NL_V_AT)echo "    Generate all configured build artifacts, run all unit "
$(NL_V_AT)echo "    and functional tests, and generate code coverage results "
$(NL_V_AT)echo "    for this project."
$(NL_V_AT)echo
$(NL_V_AT)echo "  dist"
$(NL_V_AT)echo "    Generate an archive distribution snapshot for this project."
$(NL_V_AT)echo
$(NL_V_AT)echo "  distcheck"
$(NL_V_AT)echo "    Generate an archive distribution snapshot for this project "
$(NL_V_AT)echo "    and sanity check the resulting distribution by running "
$(NL_V_AT)echo "    'make check' on it for this project."
$(NL_V_AT)echo
$(NL_V_AT)echo "  doc"
$(NL_V_AT)echo "    Generate documentation for the project."
$(NL_V_AT)echo
$(NL_V_AT)echo "  docdist"
$(NL_V_AT)echo "    Generate an archive distribution of the documentation for "
$(NL_V_AT)echo "    the project."
$(NL_V_AT)echo
$(NL_V_AT)echo "  install"
$(NL_V_AT)echo "    Generate all configured build artifacts for this project "
$(NL_V_AT)echo "    and install them in DESTDIR on the build host system."
$(NL_V_AT)echo
$(NL_V_AT)echo "  install-headers"
$(NL_V_AT)echo "    Generate all configured public header artifacts for this "
$(NL_V_AT)echo "    project and install them in DESTDIR on the build host "
$(NL_V_AT)echo "    system."
$(NL_V_AT)echo
$(NL_V_AT)echo "  pretty"
$(NL_V_AT)echo "    (Re-)format a collection of project source files."
$(NL_V_AT)echo
$(NL_V_AT)echo "  pretty-check"
$(NL_V_AT)echo "    Check but do not (re-)format a collection of project "
$(NL_V_AT)echo "    source files."
$(NL_V_AT)echo
$(NL_V_AT)echo "  Makefile"
$(NL_V_AT)echo "    Run 'configure' for this project and generate the"
$(NL_V_AT)echo "    host-specific makefile."
$(NL_V_AT)echo
endef # PrintBootstrapHelp

.PHONY: help-bootstrap-local
help-bootstrap-local:
	$(call PrintBootstrapHelp)

.PHONY: help-bootstrap-hook
help-bootstrap-hook: help-bootstrap-local

.PHONY: help-bootstrap
help-bootstrap: help-bootstrap-local help-bootstrap-hook

.PHONY: help
help: help-bootstrap

