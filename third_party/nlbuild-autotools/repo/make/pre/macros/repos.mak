#
#    Copyright 2018 Google LLC. All Rights Reserved.
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
#      make convenience macros for interacting with and managing "foreign"
#      (e.g., those outside of this project) git projects in the context of
#      managing project dependencies.
#

# Third-party package repository configuration, if it exists, listing all
# packages and their names, branches, URLs, and local paths that may be
# optionally inlined for this package.
#
# This file is formatted identically to .gitmodules (see gitmodules(5)).

REPOS_CONFIG                     ?= $(top_srcdir)/repos.conf

REPOS                            := $(shell if test -f $(REPOS_CONFIG); then $(call nlGitListConfigFromFileCommand,$(REPOS_CONFIG)) | $(GREP) '^submodule\.' | $(CUT) -d '.' -f 2 | $(SORT) | $(UNIQ); fi)
