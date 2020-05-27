#
#    Copyright (c) 2020 Project nlbuild-autotools Authors. All Rights Reserved.
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
#      This file is the make "footer" or post make header for help-related
#      targets.
#

define PrintHelpPreamble
$(NL_V_AT)echo "This makefile supports the following build targets:"
$(NL_V_AT)echo
endef # PrintHelpPreamble

define PrintAllHelp
$(NL_V_AT)echo "  all"
$(NL_V_AT)echo "    Generate all configured build artifacts for this project."
$(NL_V_AT)echo
endef # PrintAllHelp

define PrintCheckHelp
$(NL_V_AT)echo "  check"
$(NL_V_AT)echo "    Generate all configured build artifacts and run all unit"
$(NL_V_AT)echo "    and functional tests for this project."
$(NL_V_AT)echo
endef # PrintCheckHelp

define PrintDistHelp
$(NL_V_AT)echo "  dist"
$(NL_V_AT)echo "    Generate an archive distribution snapshot for this project."
$(NL_V_AT)echo
endef # PrintDistHelp

define PrintDistcheckHelp
$(NL_V_AT)echo "  distcheck"
$(NL_V_AT)echo "    Generate an archive distribution snapshot for this project"
$(NL_V_AT)echo "    and sanity check the resulting distribution by running"
$(NL_V_AT)echo "    'make check' on it for this project."
$(NL_V_AT)echo
endef # PrintDistcheckHelp

define PrintDocHelp
$(NL_V_AT)echo "  doc"
$(NL_V_AT)echo "    Generate documentation for the project."
$(NL_V_AT)echo
endef # PrintDocHelp

define PrintDocdistHelp
$(NL_V_AT)echo "  docdist"
$(NL_V_AT)echo "    Generate an archive distribution of the documentation for"
$(NL_V_AT)echo "    the project."
$(NL_V_AT)echo
endef # PrintDocdistHelp

define PrintHelpHelp
$(NL_V_AT)echo "  help"
$(NL_V_AT)echo "    Display this content."
$(NL_V_AT)echo
endef # PrintHelpHelp

define PrintInstallHelp
$(NL_V_AT)echo "  install"
$(NL_V_AT)echo "    Generate all configured build artifacts for this project"
$(NL_V_AT)echo "    and install athem in DESTDIR on the build host system."
$(NL_V_AT)echo
endef # PrintInstallHelp

define PrintHelp
$(call PrintHelpPreamble)
$(call PrintAllHelp)
$(call PrintCheckHelp)
$(call PrintCoverageHelp)
$(call PrintDistHelp)
$(call PrintDistcheckHelp)
$(call PrintDocHelp)
$(call PrintDocdistHelp)
$(call PrintInstallHelp)
$(call PrintInstallHeadersHelp)
$(call PrintPrettyHelp)
$(call PrintReposHelp)
endef # PrintHelp

.PHONY: help-local
help-local:
	$(call PrintHelp)

.PHONY: help-hook
help-hook: help-local

.PHONY: help
help: help-local help-hook
