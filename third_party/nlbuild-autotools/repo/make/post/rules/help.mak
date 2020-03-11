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
#      This file is the make "footer" or post make header for help-related
#      targets.
#

define PrintHelp
$(NL_V_AT)echo "This makefile supports the following build targets:"
$(NL_V_AT)echo
$(NL_V_AT)echo "  help"
$(NL_V_AT)echo "    Display this content."
$(NL_V_AT)echo
endef # PrintHelp

.PHONY: help-local
help-local:
	$(call PrintHelp)

.PHONY: help-hook
help-hook: help-local

.PHONY: help
help: help-local help-hook
