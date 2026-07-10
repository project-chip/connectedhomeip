#!/usr/bin/env python3
#
# Copyright (c) 2026 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

import json
import logging
import os
import urllib.request
from enum import Enum
from typing import Any, Callable, Iterable, NamedTuple, cast

import click
import coloredlogs
import pathspec
import yaml
from github import Github, GithubException
from github.Commit import Commit
from github.PullRequest import PullRequest
from github.Repository import Repository

log = logging.getLogger(__name__)

_LOG_LEVELS = logging.getLevelNamesMapping()

DEFAULT_REPOSITORY = "project-chip/connectedhomeip"
DEFAULT_CONFIG_PATH = ".github/platform_maintainers.yaml"

ELIGIBILITY_COMMENT_MARKER = "<!-- pr-checker-bot-eligibility-marker -->"


class ValidationCheck(Enum):
    CI = "ci"
    PULLAPPROVE = "pullapprove"
    COMMENTS = "comments"


class GroupApproval(NamedTuple):
    """Represents an approval for a platform group."""

    approver: str
    files: set[str]


class PlatformGroup:
    """Represents a platform group configuration."""

    def __init__(self, name: str, maintainers: list[str], paths: list[str]) -> None:
        self.name = name
        self.maintainers = sorted({m.strip().lower() for m in maintainers})
        self.paths = sorted({p.strip() for p in paths})
        self.spec = pathspec.PathSpec.from_lines("gitignore", self.paths)
        self.path_specs = {
            glob: pathspec.PathSpec.from_lines("gitignore", [glob])
            for glob in self.paths
        }

    def matches_file(self, filepath: str) -> bool:
        """Checks if a file path matches this group's configured paths."""
        return self.spec.match_file(filepath)

    def get_matched_globs(self, files: Iterable[str]) -> list[str]:
        """Returns the list of glob patterns in this group that match any of the given files."""
        matched_globs = set()
        for f in files:
            for glob, spec in self.path_specs.items():
                if spec.match_file(f):
                    matched_globs.add(glob)
        return sorted(matched_globs)


class PRContext:
    """State-holder for the PR being processed. Lazily loads and caches API data."""

    def __init__(
        self,
        api: Github,
        token: str,
        repo: Repository,
        pr: PullRequest,
        groups: dict[str, PlatformGroup],
        dry_run: bool,
        skip_checks: set[ValidationCheck],
    ) -> None:
        self.api = api
        self.token = token
        self.repo = repo
        self.pr = pr
        self.groups = groups
        self.dry_run = dry_run
        self.skip_checks = skip_checks

        # Caches
        self._bot_username: str | None = None
        self._commit: Commit | None = None
        self._changed_files: set[str] | None = None
        self._file_analysis: tuple[dict[str, set[str]], set[str]] | None = None
        self._review_states: tuple[set[str], set[str]] | None = None
        self._unresolved_threads: list[dict[str, Any]] | None = None
        self._ci_passed: bool | None = None
        self._is_pullapprove_green: bool | None = None
        self._bot_comments: list[Any] | None = None
        self._approvals_analysis: (
            tuple[dict[str, PlatformGroup], dict[str, GroupApproval]] | None
        ) = None

    @property
    def bot_username(self) -> str:
        if self._bot_username is None:
            try:
                self._bot_username = self.api.get_user().login.lower()
            except GithubException:
                self._bot_username = "pr-checker-bot"
        return self._bot_username

    @property
    def commit(self) -> Commit:
        if self._commit is None:
            self._commit = self.repo.get_commit(sha=self.pr.head.sha)
        return self._commit

    @property
    def changed_files(self) -> set[str]:
        if self._changed_files is None:
            files = set()
            for pr_file in self.pr.get_files():
                files.add(pr_file.filename)
                if getattr(pr_file, "previous_filename", None):
                    files.add(pr_file.previous_filename)
            self._changed_files = files
        return self._changed_files

    @property
    def file_analysis(self) -> tuple[dict[str, set[str]], set[str]]:
        if self._file_analysis is None:
            matched_files_per_group: dict[str, set[str]] = {
                name: set() for name in self.groups
            }
            uncovered_files = set()
            for filepath in self.changed_files:
                matched_any = False
                for group_name, group in self.groups.items():
                    if group.matches_file(filepath):
                        matched_files_per_group[group_name].add(filepath)
                        matched_any = True
                if not matched_any:
                    uncovered_files.add(filepath)
            self._file_analysis = (matched_files_per_group, uncovered_files)
        return self._file_analysis

    @property
    def active_groups(self) -> dict[str, set[str]]:
        matched, _ = self.file_analysis
        return {name: files for name, files in matched.items() if files}

    @property
    def uncovered_files(self) -> set[str]:
        _, uncovered = self.file_analysis
        return uncovered

    @property
    def is_platform_eligible(self) -> bool:
        return bool(self.active_groups) and not self.uncovered_files

    @property
    def review_states(self) -> tuple[set[str], set[str]]:
        if self._review_states is None:
            user_reviews = {}
            for review in self.pr.get_reviews():
                review_user = getattr(review.user, "login", None)
                if not review_user:
                    continue
                user = review_user.lower()
                if review.state in ("APPROVED", "CHANGES_REQUESTED", "DISMISSED"):
                    user_reviews[user] = review.state

            approvers = {
                user for user, state in user_reviews.items() if state == "APPROVED"
            }
            pr_author = getattr(self.pr.user, "login", None)
            author = pr_author.lower() if pr_author else ""
            approvers.discard(author)
            change_requesters = {
                user
                for user, state in user_reviews.items()
                if state == "CHANGES_REQUESTED"
            }
            self._review_states = (approvers, change_requesters)
        return self._review_states

    @property
    def approvers(self) -> set[str]:
        return self.review_states[0]

    @property
    def change_requesters(self) -> set[str]:
        return self.review_states[1]

    @property
    def unresolved_threads(self) -> list[dict]:
        if self._unresolved_threads is None:
            self._unresolved_threads = self._load_unresolved_threads()
        return self._unresolved_threads

    def _load_unresolved_threads(self) -> list[dict]:
        owner, repo_name = self.repo.full_name.split("/")
        query = """
        query($owner: String!, $name: String!, $number: Int!) {
          repository(owner: $owner, name: $name) {
            pullRequest(number: $number) {
              reviewThreads(first: 100) {
                pageInfo {
                  hasNextPage
                }
                nodes {
                  isResolved
                  comments(first: 1) {
                    nodes {
                      author { login }
                      body
                      url
                    }
                  }
                }
              }
            }
          }
        }
        """
        variables = {"owner": owner, "name": repo_name, "number": self.pr.number}

        req = urllib.request.Request(
            "https://api.github.com/graphql",
            data=json.dumps({"query": query, "variables": variables}).encode("utf-8"),
            headers={
                "Authorization": f"Bearer {self.token}",
                "Content-Type": "application/json",
                "User-Agent": "pr-checker-bot",
            },
            method="POST",
        )

        unresolved = []
        try:
            with urllib.request.urlopen(req, timeout=30) as response:
                res_data = json.loads(response.read().decode("utf-8"))
                if "errors" in res_data:
                    raise RuntimeError(
                        f"GraphQL API returned errors: {res_data['errors']}"
                    )

                data = res_data.get("data")
                if (
                    not data
                    or not data.get("repository")
                    or not data["repository"].get("pullRequest")
                ):
                    raise RuntimeError(
                        f"GraphQL response missing PR repository/pullRequest data: {res_data}"
                    )
                threads_data = data["repository"]["pullRequest"]["reviewThreads"]
                if threads_data["pageInfo"]["hasNextPage"]:
                    log.warning(
                        "PR #%d has more than 100 review threads. Gating merge to be safe.",
                        self.pr.number,
                    )
                    unresolved.append(
                        {
                            "author": "system",
                            "body_preview": "Too many review threads (>100). Please resolve or clean up threads.",
                            "url": self.pr.html_url + "/files",
                        }
                    )

                threads = threads_data["nodes"]
                for thread in threads:
                    if not thread["isResolved"]:
                        first_comment = (
                            thread["comments"]["nodes"][0]
                            if thread["comments"]["nodes"]
                            else None
                        )
                        author = (
                            first_comment["author"]["login"]
                            if first_comment and first_comment["author"]
                            else "unknown"
                        )
                        url = first_comment["url"] if first_comment else ""
                        body_preview = ""
                        if first_comment and first_comment.get("body"):
                            body_preview = " ".join(first_comment["body"].split())
                            if len(body_preview) > 40:
                                body_preview = body_preview[:37] + "..."
                        unresolved.append(
                            {
                                "author": author,
                                "body_preview": body_preview,
                                "url": url,
                            }
                        )
        except Exception as e:
            log.error(
                "Failed to query unresolved threads for PR #%d: %s",
                self.pr.number,
                e,
            )
            raise

        return unresolved

    @property
    def ci_passed(self) -> bool:
        if self._ci_passed is None:
            self._ci_passed = self._check_ci_passed()
        return self._ci_passed

    def _check_ci_passed(self) -> bool:
        commit = self.commit
        combined_status = commit.get_combined_status()
        check_suites = list(commit.get_check_suites())

        total_checks = len(combined_status.statuses) + len(check_suites)
        if total_checks < 10:
            log.info(
                "PR #%d HEAD commit %s has only %d CI checks registered (expected >= 10). Treating as pending.",
                self.pr.number,
                commit.sha[:8],
                total_checks,
            )
            return False

        ignored_contexts = {"pullapprove"}
        for status in combined_status.statuses:
            if status.context in ignored_contexts:
                continue
            if status.state != "success":
                log.info(
                    "PR #%d HEAD commit %s status '%s' is '%s' (%s)",
                    self.pr.number,
                    commit.sha[:8],
                    status.context,
                    status.state,
                    status.description,
                )
                return False

        for suite in check_suites:
            if suite.status != "completed":
                log.info(
                    "PR #%d HEAD commit %s check suite '%s' is not completed (status: '%s')",
                    self.pr.number,
                    commit.sha[:8],
                    suite.id,
                    suite.status,
                )
                return False
            if suite.conclusion not in ("success", "neutral", "skipped"):
                log.info(
                    "PR #%d HEAD commit %s check suite '%s' failed (conclusion: '%s')",
                    self.pr.number,
                    commit.sha[:8],
                    suite.id,
                    suite.conclusion,
                )
                return False

        return True

    @property
    def is_pullapprove_green(self) -> bool:
        if self._is_pullapprove_green is None:
            self._is_pullapprove_green = self._check_pullapprove_green()
        return self._is_pullapprove_green

    def _check_pullapprove_green(self) -> bool:
        combined_status = self.commit.get_combined_status()
        for status in combined_status.statuses:
            if status.context == "pullapprove":
                return status.state == "success"
        return False

    @property
    def mergeable(self) -> bool | None:
        return self.pr.mergeable

    @property
    def bot_comments(self) -> list:
        if self._bot_comments is None:
            self._bot_comments = self._find_bot_comments()
        return self._bot_comments

    def _find_bot_comments(self) -> list:
        bot_comments = []
        for comment in self.pr.get_issue_comments():
            comment_user = getattr(comment.user, "login", None)
            if comment_user and comment_user.lower() == self.bot_username:
                if comment.body and ELIGIBILITY_COMMENT_MARKER in comment.body:
                    bot_comments.append(comment)
        return bot_comments

    @property
    def approvals_analysis(
        self,
    ) -> tuple[dict[str, PlatformGroup], dict[str, GroupApproval]]:
        if self._approvals_analysis is None:
            missing_approvals = {}
            valid_approvals = {}
            for group_name, files in self.active_groups.items():
                group = self.groups[group_name]
                group_approvers = self.approvers.intersection(group.maintainers)
                if not group_approvers:
                    missing_approvals[group_name] = group
                else:
                    valid_approvals[group_name] = GroupApproval(
                        sorted(group_approvers)[0], files
                    )
            self._approvals_analysis = (missing_approvals, valid_approvals)
        return self._approvals_analysis

    @property
    def missing_approvals(self) -> dict[str, PlatformGroup]:
        return self.approvals_analysis[0]

    @property
    def valid_approvals(self) -> dict[str, GroupApproval]:
        return self.approvals_analysis[1]


class CheckResult:
    def __init__(
        self, passed: bool, message: str = "", details: dict[str, Any] | None = None
    ) -> None:
        self.passed = passed
        self.message = message
        self.details = details or {}


class Gate:
    def __init__(
        self,
        fn: Callable[[PRContext], bool],
        on_fail: Callable[[PRContext], None] | None = None,
    ) -> None:
        self.fn = fn
        self.on_fail = on_fail

    def run(self, context: PRContext) -> bool:
        passed = self.fn(context)
        if not passed and self.on_fail:
            self.on_fail(context)
        return passed


class Workflow:
    def __init__(
        self,
        name: str,
        gates: list[Gate],
        checks: list[Callable[[PRContext], CheckResult]],
        on_success: Callable[[PRContext], None],
        on_failure: Callable[[PRContext], None],
    ) -> None:
        self.name = name
        self.gates = gates
        self.checks = checks
        self.on_success = on_success
        self.on_failure = on_failure


# --- Gate Functions ---


def gate_no_change_requests(context: PRContext) -> bool:
    if context.change_requesters:
        log.info(
            "PR #%d has active changes requested by: %s. Skipping.",
            context.pr.number,
            list(context.change_requesters),
        )
        return False
    return True


def gate_no_uncovered_files(context: PRContext) -> bool:
    if context.uncovered_files:
        log.info(
            "PR #%d contains files outside the platform-maintained scope. Skipping. Uncovered files: %s",
            context.pr.number,
            list(context.uncovered_files)[:5],
        )
        return False
    return True


def gate_has_active_groups(context: PRContext) -> bool:
    if not context.active_groups:
        log.info("PR #%d has no changed files in platform groups.", context.pr.number)
        return False
    return True


def gate_pullapprove_pending(context: PRContext) -> bool:
    if ValidationCheck.PULLAPPROVE in context.skip_checks:
        return True
    if context.is_pullapprove_green:
        log.info(
            "PR #%d has a successful pullapprove check. Skipping bot merge (standard flow applies).",
            context.pr.number,
        )
        return False
    return True


# --- Check Functions ---


def check_ci(context: PRContext) -> CheckResult:
    if ValidationCheck.CI in context.skip_checks:
        return CheckResult(passed=True, message="CI check skipped")
    if context.ci_passed:
        return CheckResult(passed=True)
    return CheckResult(passed=False, message="CI checks are pending or failed.")


def check_unresolved_comments(context: PRContext) -> CheckResult:
    if ValidationCheck.COMMENTS in context.skip_checks:
        return CheckResult(passed=True, message="Unresolved comments check skipped")
    threads = context.unresolved_threads
    if threads:
        return CheckResult(
            passed=False,
            message="PR has unresolved review conversations.",
            details={"threads": threads},
        )
    return CheckResult(passed=True)


def check_mergeable(context: PRContext) -> CheckResult:
    mergeable = context.mergeable
    if mergeable is True:
        return CheckResult(passed=True)
    elif mergeable is False:
        return CheckResult(passed=False, message="PR has merge conflicts.")
    else:
        return CheckResult(
            passed=False,
            message="Mergeability state is computing on GitHub.",
            details={"computing": True},
        )


def check_platform_approvals(context: PRContext) -> CheckResult:
    if context.missing_approvals:
        return CheckResult(passed=False, message="Needs platform maintainer approvals.")
    return CheckResult(passed=True)


# --- Action Functions ---


def action_noop(context: PRContext) -> None:
    pass


def action_remove_eligibility_comment(context: PRContext) -> None:
    if context.pr.comments == 0:
        return
    for comment in context.bot_comments:
        if context.dry_run:
            log.info(
                "[Dry Run] Would delete stale eligibility comment on PR #%d",
                context.pr.number,
            )
        else:
            log.info("Deleting stale eligibility comment on PR #%d", context.pr.number)
            try:
                comment.delete()
            except GithubException as e:
                log.error("Failed to delete stale comment #%d: %s", comment.id, e)


def action_merge_platform(context: PRContext) -> None:
    valid_approvals_per_group = context.valid_approvals
    merge_reason_comment = "### Platform Maintainers Auto-Merge Executed\n"
    merge_reason_comment += "This PR has been automatically merged. It contains changes restricted to platform-maintained paths and received the required maintainer approvals:\n\n"

    for group_name, approval in valid_approvals_per_group.items():
        group = context.groups[group_name]
        matched_globs = group.get_matched_globs(approval.files)
        globs_str = ", ".join([f"`{g}`" for g in matched_globs])
        merge_reason_comment += f"- **{group_name}** changes (matching {globs_str}) approved by @{approval.approver}\n"

    if context.dry_run:
        log.info(
            "[Dry Run] Would post merge comment to PR #%d:\n%s",
            context.pr.number,
            merge_reason_comment,
        )
        log.info("[Dry Run] Would merge PR #%d (method: squash)", context.pr.number)
    else:
        log.info("Merging PR #%d", context.pr.number)
        context.pr.merge(
            merge_method="squash",
            commit_title=f"{context.pr.title} (Auto-merged by platform-bot)",
            sha=context.pr.head.sha,
        )
        log.info("Posting merge explanation comment to PR #%d", context.pr.number)
        try:
            context.pr.create_issue_comment(merge_reason_comment)
        except GithubException as e:
            log.error(
                "Failed to post merge explanation comment to PR #%d: %s",
                context.pr.number,
                e,
            )


def action_post_platform_eligibility(context: PRContext) -> None:
    comment_body = f"{ELIGIBILITY_COMMENT_MARKER}\n"
    comment_body += "### Platform Maintainers Auto-Merge Info\n"
    comment_body += "This PR is restricted to platform-maintained paths and is eligible for auto-merging upon approval from the designated maintainers.\n\n"
    comment_body += (
        "To merge, we require at least one approval from each of these groups:\n"
    )

    active_groups = context.active_groups
    missing_approvals = context.missing_approvals
    unresolved_threads = context.unresolved_threads

    ci_skipped = ValidationCheck.CI in context.skip_checks
    ci_passed = context.ci_passed
    mergeable = context.mergeable

    for group_name, files in active_groups.items():
        group = context.groups[group_name]
        maintainer_mentions = ", ".join([f"@{m}" for m in group.maintainers])
        status = (
            "❌ Needs approval" if group_name in missing_approvals else "✅ Approved"
        )
        comment_body += f"- **{group_name}**: {maintainer_mentions} ({status})\n"
        comment_body += "  *Paths matched:*\n"
        for glob in group.get_matched_globs(files):
            comment_body += f"    * `{glob}`\n"

    comment_body += "\n### Merge Requirements Status\n"

    is_ready = (
        not missing_approvals
        and not unresolved_threads
        and (ci_passed or ci_skipped)
        and mergeable is True
    )

    if is_ready:
        comment_body += "✅ **All checks passed. PR is ready for merge.**\n"
    else:
        comment_body += "⚠️ **PR is not ready to merge yet:**\n"
        if missing_approvals:
            comment_body += "- ❌ Needs platform maintainer approvals (see above).\n"
        else:
            comment_body += "- ✅ Has all platform maintainer approvals.\n"

        if unresolved_threads:
            comment_body += "- ❌ Has unresolved review conversations:\n"
            for thread in unresolved_threads:
                link_part = f" ([Link]({thread['url']}))" if thread["url"] else ""
                comment_preview = (
                    f': *"{thread["body_preview"]}"*' if thread["body_preview"] else ""
                )
                comment_body += f"  * Unresolved thread by @{thread['author']}{link_part}{comment_preview}\n"
        else:
            comment_body += "- ✅ All review conversations resolved.\n"

        if ci_passed or ci_skipped:
            comment_body += "- ✅ All CI status and check suites passed.\n"
        else:
            comment_body += "- ❌ CI checks are pending or failed.\n"

        if mergeable is True:
            comment_body += "- ✅ No merge conflicts.\n"
        elif mergeable is False:
            comment_body += (
                "- ❌ PR has merge conflicts (resolve conflicts before merge).\n"
            )
        else:
            comment_body += "- ⚠️ Mergeability state is computing on GitHub.\n"

    bot_comments = context.bot_comments
    if bot_comments:
        main_comment = bot_comments[0]
        if main_comment.body.strip() != comment_body.strip():
            if context.dry_run:
                log.info(
                    "[Dry Run] Would update eligibility comment on PR #%d",
                    context.pr.number,
                )
            else:
                log.info("Updating eligibility comment on PR #%d", context.pr.number)
                main_comment.edit(comment_body)
        else:
            log.debug(
                "PR #%d already has an up-to-date eligibility comment.",
                context.pr.number,
            )

        for duplicate in bot_comments[1:]:
            if context.dry_run:
                log.info(
                    "[Dry Run] Would delete duplicate eligibility comment on PR #%d",
                    context.pr.number,
                )
            else:
                log.info(
                    "Deleting duplicate eligibility comment on PR #%d",
                    context.pr.number,
                )
                try:
                    duplicate.delete()
                except GithubException as e:
                    log.error(
                        "Failed to delete duplicate comment #%d: %s",
                        duplicate.id,
                        e,
                    )
    else:
        if context.dry_run:
            log.info(
                "[Dry Run] Would post eligibility comment to PR #%d:\n%s",
                context.pr.number,
                comment_body,
            )
        else:
            log.info("Posting eligibility comment to PR #%d", context.pr.number)
            context.pr.create_issue_comment(comment_body)


# --- Workflows ---

PLATFORM_MERGE_WORKFLOW = Workflow(
    name="platform_merge",
    gates=[
        Gate(gate_no_change_requests),
        Gate(gate_no_uncovered_files, on_fail=action_remove_eligibility_comment),
        Gate(gate_has_active_groups, on_fail=action_remove_eligibility_comment),
        Gate(gate_pullapprove_pending, on_fail=action_remove_eligibility_comment),
    ],
    checks=[
        check_ci,
        check_unresolved_comments,
        check_mergeable,
        check_platform_approvals,
    ],
    on_success=action_merge_platform,
    on_failure=action_post_platform_eligibility,
)

WORKFLOWS = {
    "platform_merge": PLATFORM_MERGE_WORKFLOW,
}


def select_workflow(context: PRContext) -> Workflow | None:
    if context.active_groups or context.bot_comments:
        return PLATFORM_MERGE_WORKFLOW
    return None


class PrCheckerBot:
    """Orchestrates scanning, checking coverage, and auto-merging PRs using configurable workflows."""

    def __init__(
        self,
        token: str,
        repo_name: str,
        config_path: str,
        dry_run: bool,
        skip_checks: list[str] | None = None,
        workflow_name: str | None = None,
    ) -> None:
        self.token = token
        self.repo_name = repo_name
        self.api = Github(token)
        self.repo = self.api.get_repo(repo_name)
        self.config_path = config_path
        self.dry_run = dry_run
        self.skip_checks = {ValidationCheck(c) for c in (skip_checks or [])}
        self.workflow_name = workflow_name
        self.single_pr_mode = False
        self.groups: dict[str, PlatformGroup] = {}
        self.load_config()

    def load_config(self) -> None:
        """Loads and validates the platform groups configuration from the YAML file."""
        if not os.path.exists(self.config_path):
            raise FileNotFoundError(f"Config file not found at {self.config_path}")

        with open(self.config_path, encoding="utf-8") as f:
            content = yaml.safe_load(f)

        if not isinstance(content, dict):
            raise ValueError(
                "Invalid config file format. Expected a YAML dictionary of groups."
            )

        for name, data in content.items():
            if not isinstance(name, str):
                raise ValueError(f"Group name '{name}' must be a string.")
            if not isinstance(data, dict):
                raise ValueError(
                    f"Invalid data format for group '{name}'. Expected a dictionary."
                )

            invalid_keys = set(data.keys()) - {"maintainers", "paths"}
            if invalid_keys:
                raise ValueError(
                    f"Group '{name}' contains unrecognized keys: {list(invalid_keys)}"
                )

            maintainers = data.get("maintainers")
            paths = data.get("paths")

            if not isinstance(maintainers, list) or not maintainers:
                raise ValueError(
                    f"Group '{name}' must contain a non-empty 'maintainers' list."
                )
            if not isinstance(paths, list) or not paths:
                raise ValueError(
                    f"Group '{name}' must contain a non-empty 'paths' list."
                )

            if not all(isinstance(m, str) and m.strip() for m in maintainers):
                raise ValueError(
                    f"Group '{name}' maintainers must be non-empty strings."
                )
            if not all(isinstance(p, str) and p.strip() for p in paths):
                raise ValueError(f"Group '{name}' paths must be non-empty strings.")

            self.groups[name] = PlatformGroup(name, maintainers, paths)

        log.info("Loaded %d platform groups from config.", len(self.groups))

    def check_and_process_pr(
        self, pr: PullRequest, workflow: Workflow | None = None
    ) -> None:
        """Checks the eligibility and readiness of a PR and runs the appropriate workflow."""
        if pr.user is None or not getattr(pr.user, "login", None):
            log.info(
                "PR #%d has no valid author (deleted account). Skipping.",
                pr.number,
            )
            return

        pr_author = pr.user.login
        log.info("Checking PR #%d: '%s' (Author: %s)", pr.number, pr.title, pr_author)

        if pr.state != "open":
            if self.dry_run and self.single_pr_mode:
                log.info(
                    "PR #%d state is '%s' but bypassing open check for testing in dry-run.",
                    pr.number,
                    pr.state,
                )
            else:
                log.info(
                    "PR #%d is not open (state: '%s'). Skipping.",
                    pr.number,
                    pr.state,
                )
                return

        if pr.draft:
            if self.dry_run and self.single_pr_mode:
                log.info(
                    "PR #%d is draft but bypassing draft check for testing in dry-run.",
                    pr.number,
                )
            else:
                log.info("PR #%d is a draft. Skipping.", pr.number)
                return

        context = PRContext(
            api=self.api,
            token=self.token,
            repo=self.repo,
            pr=pr,
            groups=self.groups,
            dry_run=self.dry_run,
            skip_checks=self.skip_checks,
        )

        if workflow is None:
            workflow = select_workflow(context)

        if workflow is None:
            log.info("PR #%d does not match any workflow. Skipping.", pr.number)
            return

        self.run_workflow(workflow, context)

    def run_workflow(self, workflow: Workflow, context: PRContext) -> None:
        log.info("Running workflow '%s' for PR #%d", workflow.name, context.pr.number)

        for gate in workflow.gates:
            if not gate.run(context):
                log.info("Gate '%s' failed. Aborting workflow.", gate.fn.__name__)
                return

        results = []
        all_passed = True
        for check in workflow.checks:
            res = check(context)
            results.append(res)
            if not res.passed:
                all_passed = False
                log.info("Check '%s' failed: %s", check.__name__, res.message)
            else:
                if res.message:
                    log.info("Check '%s' passed: %s", check.__name__, res.message)

        if all_passed:
            log.info("All checks passed. Triggering success action.")
            workflow.on_success(context)
        else:
            log.info("Some checks failed. Triggering failure action.")
            workflow.on_failure(context)

    def run(self, pr_number: int | None = None) -> None:
        """Runs the bot, either scanning all open PRs or processing a single PR."""
        self.single_pr_mode = pr_number is not None
        has_errors = False

        workflow = None
        if self.workflow_name:
            workflow = WORKFLOWS.get(self.workflow_name)
            if not workflow:
                raise ValueError(f"Unknown workflow: {self.workflow_name}")

        if pr_number is not None:
            log.info("Processing single PR #%d...", pr_number)
            try:
                pr = self.repo.get_pull(cast(int, pr_number))
                self.check_and_process_pr(pr, workflow)
            except Exception as e:
                log.exception("Error processing PR #%d: %s", pr_number, e)
                has_errors = True
        else:
            log.info("Scanning open pull requests...")
            open_prs = self.repo.get_pulls(state="open")
            for pr in open_prs:
                try:
                    self.check_and_process_pr(pr, workflow)
                except Exception as e:
                    log.exception("Error processing PR #%d: %s", pr.number, e)
                    has_errors = True
        if has_errors:
            raise RuntimeError("One or more PRs encountered errors during processing.")


@click.command()
@click.option(
    "--log-level",
    default="INFO",
    type=click.Choice(list(_LOG_LEVELS.keys()), case_sensitive=False),
    help="Determines the verbosity of script output.",
)
@click.option(
    "--token-env",
    default="GH_TOKEN",
    help="Environment variable containing the GitHub token",
)
@click.option(
    "--token-file",
    type=click.Path(exists=True, dir_okay=False, readable=True),
    help="Read github token from the given file",
)
@click.option(
    "--repo",
    default=DEFAULT_REPOSITORY,
    help=f"Github repository name (default: {DEFAULT_REPOSITORY})",
)
@click.option(
    "--config",
    default=DEFAULT_CONFIG_PATH,
    help=f"Path to the platform maintainers yaml config (default: {DEFAULT_CONFIG_PATH})",
)
@click.option(
    "--dry-run",
    default=False,
    is_flag=True,
    help="Simulate merging and commenting without executing",
)
@click.option(
    "--pr",
    type=int,
    help="Process only this specific pull request number.",
)
@click.option(
    "--skip-check",
    multiple=True,
    type=click.Choice(["ci", "pullapprove", "comments"]),
    help="Validation check to skip. Can be specified multiple times.",
)
@click.option(
    "--workflow",
    type=click.Choice(list(WORKFLOWS.keys())),
    help="Force a specific workflow to run.",
)
def main(
    log_level: str,
    token_env: str,
    token_file: str | None,
    repo: str,
    config: str,
    dry_run: bool,
    pr: int | None,
    skip_check: tuple[str, ...],
    workflow: str | None,
) -> None:
    """Platform Merge Bot entry point.

    Example Dry-Run and Validation Testing:
    ---------------------------------------
    # Run the bot in dry-run mode on a specific PR (even if closed/merged) to see what it would do:
    GITHUB_TOKEN=$(gh auth token) python3 scripts/tools/pr_checker_bot.py --dry-run --pr 72779

    # Run in dry-run mode, skipping CI and PullApprove status validations:
    GITHUB_TOKEN=$(gh auth token) python3 scripts/tools/pr_checker_bot.py --dry-run --pr 72779 --skip-check ci --skip-check pullapprove
    """
    coloredlogs.install(
        level=_LOG_LEVELS[log_level.upper()],
        fmt="%(asctime)s %(levelname)-7s %(message)s",
    )

    gh_token = None
    if token_file:
        with open(token_file, encoding="utf-8") as f:
            gh_token = f.read().strip()
        if not gh_token:
            raise click.ClickException(f"Token file {token_file} is empty")
    else:
        gh_token = os.environ.get(token_env) or os.environ.get("GITHUB_TOKEN")

        if not gh_token:
            raise click.ClickException(
                f"Require a token. Set environment variable '{token_env}' (or 'GITHUB_TOKEN') or provide --token-file"
            )

    bot = PrCheckerBot(
        gh_token,
        repo,
        config,
        dry_run,
        skip_checks=list(skip_check),
        workflow_name=workflow,
    )
    bot.run(pr_number=pr)


if __name__ == "__main__":
    main()
