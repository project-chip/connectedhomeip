#
#    Copyright (c) 2020 Project CHIP Authors
#    Copyright (c) 2019 Google LLC.
#    Copyright (c) 2014-2017 Nest Labs, Inc.
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
#      This file is the GNU automake template for CHIP public
#      headers.
#

include $(abs_top_nlbuild_autotools_dir)/automake/pre.am

#
# Any build configuration *supplied by CHIP* is shipped
#
CHIP_CONFIG_INCLUDES=$(subst $(abspath $(abs_top_srcdir)),$(top_srcdir),$(filter $(abspath $(top_srcdir)/config/%),$(abspath $(CHIP_PROJECT_CONFIG_INCLUDES))))

#
# headers to install
include_HEADERS                            = \
    $(CHIP_CONFIG_INCLUDES)                  \
    CHIPVersion.h                            \
    BuildConfig.h                            \
    $(NULL)

# Auto-generation targets to force the creation and subsequent check
# and re-generation of CHIPVersion.h.

.PHONY: force
CHIPVersion.h: force

define check-file-CHIPVersion.h
$(top_srcdir)/scripts/gen-chip-version.sh -o "$(2)" "`cat $(1)`"
endef # check-file-CHIPVersion.h

CHIPVersion.h: $(top_builddir)/.local-version
	$(call check-file,CHIPVersion.h)

BUILT_SOURCES                                               = \
    CHIPVersion.h                                             \
    $(NULL)

CLEANFILES                                                  = \
    CHIPVersion.h                                             \
    $(NULL)

#
# Foreign make dependencies
#

NLFOREIGN_FILE_DEPENDENCIES                                 = \
   $(top_builddir)/.local-version                             \
   $(NULL)

# Ensure that auto-generated content such as BuildConfig.h and
# CHIPVersion.h, while public API headers, are scrubbed out of
# the source distribution.

dist-hook:
	$(AM_V_at)rm -f $(distdir)/BuildConfig.h
	$(AM_V_at)rm -f $(distdir)/CHIPVersion.h

install-headers: install-data

include $(abs_top_nlbuild_autotools_dir)/automake/post.am
