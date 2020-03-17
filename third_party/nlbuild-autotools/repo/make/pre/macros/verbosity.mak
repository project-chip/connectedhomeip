#
#    Copyright 2019 Google LLC. All Rights Reserved.
#    Copyright 2017-2018 Nest Labs Inc. All Rights Reserved.
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
#      This file is a make "header" or pre make header that defines
#      make macros for controlling build verbosity.
#

#
# Verbosity
#

# The default build verbosity is 0 or none. Invoking 'make V=0' is
# equivalent to 'make' with this default.

NL_DEFAULT_VERBOSITY             ?= 0

# Alias for the command echo suppression marker.

NL_AT                            := @

# Alias for a conditional command echo suppression marker, conditioned
# on the build verbosity.

NL_V_AT                           = $(NL_V_AT_$(V))
NL_V_AT_                          = $(NL_V_AT_$(NL_DEFAULT_VERBOSITY))
NL_V_AT_0                         = $(NL_AT)
NL_V_AT_1                         = 

#
# Verbosity Build Progress Macros
#

# Macro for emitting "<ACTION> <target | output>" oneline, succinct
# build progress messages.

_NL_PROGRESS                     := printf "  %-13s %s\n"
NL_PROGRESS                      := $(NL_AT)$(_NL_PROGRESS)

# Macro for emitting "<ACTION> <target | output>" oneline, succinct
# build progress messages, conditioned on build verbosity.

NL_V_PROGRESS                     = $(NL_V_PROGRESS_$(V))
NL_V_PROGRESS_                    = $(NL_V_PROGRESS_$(NL_DEFAULT_VERBOSITY))
NL_V_PROGRESS_0                   = $(NL_PROGRESS)
NL_V_PROGRESS_1                   = $(NL_AT)true

# Common convenience progress message macros, conditioned on build
# verbosity.

NL_V_PROGRESS_BOOTSTRAP           = $(NL_V_PROGRESS) "BOOTSTRAP"   "$(1)";
NL_V_PROGRESS_BOOTSTRAP_ALL       = $(NL_V_BOOTSTRAP) "all";
NL_V_PROGRESS_BOOTSTRAP_CONFIG    = $(NL_V_BOOTSTRAP) "config";
NL_V_PROGRESS_BOOTSTRAP_MAKE      = $(NL_V_BOOTSTRAP) "make";

NL_V_PROGRESS_CONFIGURE           = $(NL_V_PROGRESS) "CONFIGURE"

NL_V_PROGRESS_GIT_INIT            = $(NL_V_PROGRESS) "GIT INIT"    "$(@)";

NL_V_PROGRESS_MAKE                = $(NL_V_PROGRESS) "MAKE"        "$(@)";

NL_V_PROGRESS_LN_S                = $(NL_V_PROGRESS) "LN"          "$(@)";

NL_V_PROGRESS_MKDIR               = $(NL_V_PROGRESS) "MKDIR"       "$(1)";
NL_V_PROGRESS_MKDIR_P             = $(NL_V_MKDIR)
NL_V_PROGRESS_RMDIR               = $(NL_V_PROGRESS) "RMDIR"       "$(1)";

NL_V_PROGRESS_GZIP                = $(NL_V_PROGRESS) "GZIP"        "$(@)";
NL_V_PROGRESS_TAR                 = $(NL_V_PROGRESS) "TAR"         "$(@)";
NL_V_PROGRESS_TGZ                 = $(NL_V_PROGRESS) "TGZ"         "$(@)";
NL_V_PROGRESS_TXZ                 = $(NL_V_PROGRESS) "TXZ"         "$(@)";
NL_V_PROGRESS_XZ                  = $(NL_V_PROGRESS) "XZ"          "$(@)";

#
# Verbosity Commands and Flags
#

# This is useful when we do not want submake to be chatty about what
# it doing when verbosity is suppressed. However, when verbosity is
# requested, -s (silent) would otherwise suppress all make verbose
# output. Consequently, suppress -s when verbosity is requested.

NL_V_MAKE_S                       = $(NL_V_MAKE_S_$(V))                   
NL_V_MAKE_S_                      = $(NL_V_MAKE_S_$(NL_DEFAULT_VERBOSITY))
NL_V_MAKE_S_0                     = $(MAKE) -s
NL_V_MAKE_S_1                     = $(MAKE)
