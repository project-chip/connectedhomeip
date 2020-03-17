#
#    Copyright 2017-2018 Nest Labs Inc. All Rights Reserved.
#    Copyright 2018 Google LLC. All Rights Reserved.
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
#      This file is a make "footer" or post make header that defines make
#      convenience targets and templates for interacting with and managing
#      "foreign" (e.g., those outside of this project) git projects in the
#      context of managing project dependencies.
#

ifneq ($(REPOS),)
# Stem for git clones and submodules

__REPOS_GIT_STEM                 := /.git

# Stem for the git configuration file for a git repository.

__REPOS_GIT_CONFIG_STEM          := $(__REPOS_GIT_STEM)/config

# Stem for the git cache directory for a git submodule

__REPOS_GIT_MODULE_CACHE_STEM    := $(__REPOS_GIT_STEM)/modules

# git submodule configuration file and path

__REPOS_GIT_MODULES_FILE         := .gitmodules
__REPOS_GIT_MODULES_PATH         := $(top_srcdir)/$(__REPOS_GIT_MODULES_FILE)

# Stem for the git configuration file for a git clone or submodule.

__REPOS_GIT_CLONE_STEM           := $(__REPOS_GIT_STEM)
__REPOS_GIT_SUBMODULE_STEM       := $(__REPOS_GIT_STEM)

# Git "pull" method to use for retrieving repositories on which this
# package may depend.
#
# This defaults to 'clone' if no configuration value is present.

__REPOS_MAYBE_PULL_METHOD        := $(call nlGitGetMethodForPullFromFile,$(REPOS_CONFIG))
__REPOS_DEFAULT_PULL_METHOD      := clone
REPOS_PULL_METHOD                := $(if $(__REPOS_MAYBE_PULL_METHOD),$(__REPOS_MAYBE_PULL_METHOD),$(__REPOS_DEFAULT_PULL_METHOD))

# Git repository configuration for this package, if it exists.
#
# This value is only relevant when the pull method is 'submodule'.

ifeq ($(REPOS_PULL_METHOD),submodule)
REPOS_PACKAGE_GIT_PATH           := $(top_srcdir)$(__REPOS_GIT_CONFIG_STEM)
else
REPOS_PACKAGE_GIT_PATH           :=
endif

# Sentinel Files

REPOS_GIT_INIT_SENTINEL          := $(top_srcdir)/.repos-git-init-stamp
REPOS_GIT_MODULES_SENTINEL       := $(top_srcdir)/.repos-git-modules-stamp
REPOS_WARNING_SENTINEL           := $(top_builddir)/.repos-warning-stamp

#
# REPOS_template <repo file> <repo name>
#
# This template defines variables and targets used for inlining optional and
# required third-party packages as package-internal copies.
#
#   <repo file>  - Path to the repo configuration file from which to get
#                  values for named repo.
#   <repo name>  - Name of the repository in <repo file> for which to get
#                  values for branch, local path, and URL.
#
define REPOS_template
$(2)_repo_NAME               := $(2)
$(2)_repo_BRANCH             := $$(call nlGitGetBranchForRepoFromNameFromFile,$(1),$(2))
$(2)_repo_COMMIT             := $$(call nlGitGetCommitForRepoFromNameFromFile,$(1),$(2))
$(2)_repo_PATH               := $$(call nlGitGetPathForRepoFromNameFromFile,$(1),$(2))
$(2)_repo_URL                := $$(call nlGitGetURLForRepoFromNameFromFile,$(1),$(2))

$(2)_repo_GIT                := $$(addsuffix $(__REPOS_GIT_STEM),$$($(2)_repo_PATH))
$(2)_repo_CACHE              := $(top_srcdir)$(__REPOS_GIT_MODULE_CACHE_STEM)/$$($(2)_repo_PATH)

REPO_NAMES                   += $$($(2)_repo_NAME)
REPO_GITS                    += $$($(2)_repo_GIT)
REPO_PATHS                   += $$($(2)_repo_PATH)
REPO_URLS                    += $$($(2)_repo_URL)
REPO_CACHES                  += $$($(2)_repo_CACHE)

# Allow a repo to be made with a path target (e.g., third_party/foo/repo) or
# with its actual git target (e.g., third_party/foo/repo/.git).

$$($(2)_repo_PATH): | $$($(2)_repo_GIT)

$$($(2)_repo_GIT): $(REPOS_PACKAGE_GIT_PATH) | repos-warning
	$(NL_V_AT)case "$(REPOS_PULL_METHOD)" in \
	    submodule) echo "  SUBMODULE    $$(subst $(__REPOS_GIT_SUBMODULE_STEM),,$$(@))"; \
		if ! test -f $(__REPOS_GIT_MODULES_PATH); then \
			touch $(REPOS_GIT_MODULES_SENTINEL); \
		fi; \
		$(GIT) -C $(top_srcdir) submodule -q add -f -b $$($(2)_repo_BRANCH) -- $$($(2)_repo_URL) $$($(2)_repo_PATH);; \
            clone) echo "  CLONE        $$(subst $(__REPOS_GIT_CLONE_STEM),,$$(@))"; \
                $(GIT) -C $(top_srcdir) clone -q -b $$($(2)_repo_BRANCH) -- $$($(2)_repo_URL) $$($(2)_repo_PATH);; \
            *) echo "$(REPOS_CONFIG): Unknown or unsupported pull method '$(REPOS_PULL_METHOD)'.";; \
        esac
	$(NL_V_AT)if ! test -z "$$($(2)_repo_COMMIT)"; then \
                $(GIT) -C $$($(2)_repo_PATH) checkout -q $$($(2)_repo_COMMIT); \
        fi
endef # REPOS_template

$(REPOS_PACKAGE_GIT_PATH):
	$(NL_V_PROGRESS_GIT_INIT)
	$(GIT) -C $(top_srcdir) init -q $(top_srcdir)
	$(NL_V_AT)touch $(REPOS_GIT_INIT_SENTINEL)

define PrintReposWarning
$(NL_V_AT)echo "The 'repos' target requires external network connectivity to"
$(NL_V_AT)echo "reach the following upstream GIT repositories:"
$(NL_V_AT)echo ""
$(NL_V_AT)for url in $(REPO_URLS); do echo "    $${url}"; done
$(NL_V_AT)echo ""
$(NL_V_AT)echo "and will fail if external network connectivity is not"
$(NL_V_AT)echo "available. This package may still be buildable without these"
$(NL_V_AT)echo "packages but may require disabling certain features or"
$(NL_V_AT)echo "functionality."
$(NL_V_AT)echo ""
endef # PrintReposWarning

$(REPOS_WARNING_SENTINEL):
	$(NL_V_AT)touch $(@)
	$(call PrintReposWarning)

.PHONY: repos-warning
repos-warning: $(REPOS_WARNING_SENTINEL)

.PHONY: repos-local
repos-local: repos-warning
	$(NL_V_AT)$(MAKE) -f $(firstword $(MAKEFILE_LIST)) --no-print-directory $(REPO_GITS)

.PHONY: repos-hook
repos-hook: repos-local

.PHONY: repos
repos: repos-local repos-hook

.PHONY: clean-repos-hook
clean-repos-hook:

.PHONY: clean-repos-local
clean-repos-local: clean-repos-hook
	@echo "  CLEAN"
	$(NL_V_AT)case "$(REPOS_PULL_METHOD)" in \
	    submodule) $(GIT) -C $(top_srcdir) submodule -q deinit -f -- $(REPO_PATHS) 2> /dev/null || true; \
		if test -f $(REPOS_GIT_MODULES_SENTINEL); then \
		    $(RM) $(REPOS_GIT_MODULES_SENTINEL); \
		    $(GIT) -C $(top_srcdir) rm -f -q $(__REPOS_GIT_MODULES_PATH) 2> /dev/null; \
		fi ; \
		if test -f $(REPOS_GIT_INIT_SENTINEL); then \
		    $(RM) -r $(dir $(REPOS_PACKAGE_GIT_PATH)); \
		    $(RM) $(REPOS_GIT_INIT_SENTINEL); \
		fi; \
		$(RM) $(REPOS_WARNING_SENTINEL); \
		$(GIT) -C $(top_srcdir) rm -rf -q --cached $(REPO_PATHS) 2> /dev/null || true; \
		$(RM) -r $(addprefix $(top_srcdir)/,$(REPO_PATHS)); \
		$(RMDIR) -p $(addprefix $(top_srcdir),$(dir $(REPO_PATHS))) 2> /dev/null || true; \
		$(RM) -r $(REPO_CACHES) 2> /dev/null;; \
	    clone) $(GIT) -C $(top_srcdir) rm -rf -q --cached $(REPO_PATHS) 2> /dev/null || true; \
		$(RM) $(REPOS_WARNING_SENTINEL); \
		$(RM) -r $(addprefix $(top_srcdir)/,$(REPO_PATHS)); \
		$(RMDIR) -p $(addprefix $(top_srcdir),$(dir $(REPO_PATHS))) 2> /dev/null || true;; \
            *) echo "$(REPOS_CONFIG): Unknown or unsupported pull method '$(REPOS_PULL_METHOD)'.";; \
	esac

.PHONY: clean-repos
clean-repos: clean-repos-local

# Invoke the REPOS_template for each defined optionally-inlined package repo

$(foreach repo,$(REPOS),$(eval $(call REPOS_template,$(REPOS_CONFIG),$(repo))))

define MaybePrintReposHelp
$(NL_V_AT)echo "  repos"
$(NL_V_AT)echo "    Clone any upstream, dependent git repositories that this package"
$(NL_V_AT)echo "    regards as required or optional rather than using '--with-<package>'"
$(NL_V_AT)echo "    options to specify external instances of those packages."
$(NL_V_AT)echo
$(NL_V_AT)echo "  clean-repos"
$(NL_V_AT)echo "    This is the opposite of the 'repos' target. This removes, in their"
$(NL_V_AT)echo "    entirety, any clones of any upstream, dependent git repositories that"
$(NL_V_AT)echo "    this package regards as required or optional."
$(NL_V_AT)echo
endef # MaybePrintReposHelp

else

define MaybePrintReposHelp
endef # MaybePrintReposHelp

endif # REPOS

define PrintReposHelp
$(call MaybePrintReposHelp)
endef # PrintReposHelp

.PHONY: help-repos-local
help-repos-local:
	$(call PrintReposHelp)

.PHONY: help-repos-hook
help-repos-hook: help-repos-local

.PHONY: help-repos
help-repos: help-repos-local help-repos-hook

.PHONY: help
help: help-repos
