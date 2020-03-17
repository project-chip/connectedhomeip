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
#      This file is the make "header" or pre make header that defines make
#      convenience macros for interacting with git.
#

# nlGitGetConfigFromFileCommand <file>
#
# Command to get a value for a variable set in the specified git config file <file>.

nlGitGetConfigFromFileCommand         = $(GIT) config --file $(1)

# nlGitListConfigFromFileCommand <file>
#
# Command to list all variables set in the specified config file <file>

nlGitListConfigFromFileCommand        = $(call nlGitGetConfigFromFileCommand,$(1)) --list

# nlGitGetConfigFromFile <file>
#
# Get a value for a variable set in the specified git config file <file>.

nlGitGetConfigFromFile                = $(shell $(call nlGitGetConfigFromFileCommand,$(1)))

# nlGitListConfigFromFile <file>
#
# List all variables set in the specified config file <file>

nlGitListConfigFromFile               = $(shell $(call nlGitListConfigFromFileCommand,$(1)))

# nlGitGetValueForRepoFromNameFromFile <file> <repo name> <value>
#
# Get a value for a repo / submodule variable set in specified git config file <file>.

nlGitGetValueForRepoFromNameFromFile  = $(shell $(call nlGitGetConfigFromFileCommand,$(1)) 'submodule.$(2).$(3)')

# nlGitGetBranchForRepoFromNameFromFile <file> <repo name>
#
# Get the remote branch for a repo / submodule variable set in specified git config file <file>.

nlGitGetBranchForRepoFromNameFromFile = $(call nlGitGetValueForRepoFromNameFromFile,$(1),$(2),branch)

# nlGitGetCommitForRepoFromNameFromFile <file> <repo name>
#
# Get the commit for a repo / submodule variable set in specified git config file <file>.

nlGitGetCommitForRepoFromNameFromFile = $(call nlGitGetValueForRepoFromNameFromFile,$(1),$(2),commit)

# nlGitGetURLForRepoFromNameFromFile <file> <repo name>
#
# Get the remote URL for a repo / submodule variable set in specified git config file <file>.

nlGitGetURLForRepoFromNameFromFile    = $(call nlGitGetValueForRepoFromNameFromFile,$(1),$(2),url)

# nlGitGetPathForRepoFromNameFromFile <file> <repo name>
#
# Get the local path for a repo / submodule variable set in specified git config file <file>.

nlGitGetPathForRepoFromNameFromFile   = $(call nlGitGetValueForRepoFromNameFromFile,$(1),$(2),path)

# nlGitGetMethodForPullFromFile <file>
#
# Get the pull method set in specified git config file <file>.

nlGitGetMethodForPullFromFile         = $(shell $(call nlGitGetConfigFromFileCommand,$(1)) 'pull.method')

