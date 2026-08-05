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
from collections.abc import Callable, Iterable
from dataclasses import dataclass, field
from datetime import UTC, datetime, timedelta
from enum import Enum
from functools import cached_property
from typing import Any, NamedTuple, cast

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

# Timeout after which uncompleted check suites from known non-critical external apps
# on Dependabot PRs are considered stale/indefinitely queued.
DEPENDABOT_STALE_SUITE_TIMEOUT = timedelta(hours=6)

# Third-party integrations / GitHub apps that register check suites on all PRs but
# never complete or execute builds for Dependabot PRs
IGNORED_STALE_SUITE_APPS = {
    "GitHub Pages",
    "Codecov",
    "Testspace.com",
    "SonarQubeCloud",
    "BuildJet",
}


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


@dataclass
class FileAnalysisResult:
    matched_files_per_group: dict[str, set[str]]
    uncovered_files: set[str]


@dataclass
class ReviewStates:
    approvers: set[str]
    change_requesters: set[str]


@dataclass
class ApprovalsAnalysisResult:
    missing_approvals: dict[str, PlatformGroup]
    valid_approvals: dict[str, GroupApproval]


@dataclass
class UnresolvedThread:
    author: str
    body_preview: str
    url: str


class PRContext:
    """State-holder for the PR being processed. Lazily loads and caches API data."""

    def __init__(
        self,
        api: Github,
        token: str | None,
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

    @cached_property
    def bot_username(self) -> str:
        try:
            return self.api.get_user().login.lower()
        except GithubException:
            return "pr-checker-bot"

    @cached_property
    def is_dependabot(self) -> bool:
        if self.pr.user and getattr(self.pr.user, "login", None):
            return self.pr.user.login.lower() == "dependabot[bot]"
        return False

    @cached_property
    def commit(self) -> Commit:
        return self.repo.get_commit(sha=self.pr.head.sha)

    @cached_property
    def changed_files(self) -> set[str]:
        files = set()
        for pr_file in self.pr.get_files():
            files.add(pr_file.filename)
            if getattr(pr_file, "previous_filename", None):
                files.add(pr_file.previous_filename)
        return files

    @cached_property
    def file_analysis(self) -> FileAnalysisResult:
        matched_files_per_group: dict[str, set[str]] = {}
        uncovered_files = set()
        for filepath in self.changed_files:
            matched_any = False
            for group_name, group in self.groups.items():
                if group.matches_file(filepath):
                    matched_files_per_group.setdefault(group_name, set()).add(filepath)
                    matched_any = True
            if not matched_any:
                uncovered_files.add(filepath)
        return FileAnalysisResult(matched_files_per_group, uncovered_files)

    @property
    def active_groups(self) -> dict[str, set[str]]:
        return self.file_analysis.matched_files_per_group

    @property
    def uncovered_files(self) -> set[str]:
        return self.file_analysis.uncovered_files

    @property
    def is_platform_eligible(self) -> bool:
        return bool(self.active_groups) and not self.uncovered_files

    @cached_property
    def review_states(self) -> ReviewStates:
        approvers = set()
        change_requesters = set()
        # self.pr.get_reviews() returns reviews in chronological order from GitHub (ordered by review ID ascending).
        # Iterating in order ensures that newer review states override earlier ones per user
        # (e.g. APPROVED after CHANGES_REQUESTED removes the user from change_requesters).
        for review in self.pr.get_reviews():
            # review.user can be None if the user account was deleted.
            review_user = getattr(review.user, "login", None)
            if not review_user:
                continue
            user = review_user.lower()
            if review.state == "APPROVED":
                approvers.add(user)
                change_requesters.discard(user)
            elif review.state == "CHANGES_REQUESTED":
                change_requesters.add(user)
                approvers.discard(user)
            elif review.state == "DISMISSED":
                approvers.discard(user)
                change_requesters.discard(user)

        pr_author = getattr(self.pr.user, "login", None)
        author = pr_author.lower() if pr_author else ""
        approvers.discard(author)

        return ReviewStates(approvers, change_requesters)

    @property
    def approvers(self) -> set[str]:
        return self.review_states.approvers

    @property
    def change_requesters(self) -> set[str]:
        return self.review_states.change_requesters

    @cached_property
    def unresolved_threads(self) -> list[UnresolvedThread]:
        return self._load_unresolved_threads()

    def _load_unresolved_threads(self) -> list[UnresolvedThread]:
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

        if not self.token:
            if self.dry_run:
                log.warning(
                    "No GitHub token available for GraphQL query; skipping unresolved threads check in dry-run."
                )
                return []
            raise RuntimeError(
                "GitHub token is required for GraphQL unresolved threads query."
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
                        UnresolvedThread(
                            author="system",
                            body_preview="Too many review threads (>100). Please resolve or clean up threads.",
                            url=self.pr.html_url + "/files",
                        )
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
                            UnresolvedThread(
                                author=author,
                                body_preview=body_preview,
                                url=url,
                            )
                        )
        except Exception as e:
            log.error(
                "Failed to query unresolved threads for PR #%d: %s",
                self.pr.number,
                e,
            )
            raise

        return unresolved

    @cached_property
    def ci_passed(self) -> bool:
        return self._check_ci_passed()

    def _check_ci_passed(self) -> bool:
        commit = self.commit
        combined_status = commit.get_combined_status()
        check_suites = list(commit.get_check_suites())

        # Guard against empty checks / premature success when commit is fresh.
        # A normal PR run has at least 10 combined statuses and check suites.
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

        now = datetime.now(UTC)
        for suite in check_suites:
            app_name = suite.app.name if suite.app else ""
            if suite.status != "completed":
                created_at = suite.created_at
                if created_at.tzinfo is None:
                    created_at = created_at.replace(tzinfo=UTC)

                age = now - created_at
                # Some external integrations automatically create a CheckSuite on every commit but never trigger or complete
                # builds for Dependabot PRs. To prevent Dependabot PRs from hanging, we ignore uncompleted check suites ONLY IF:
                # 1. The PR is authored by Dependabot
                # 2. The check suite belongs to a known external app in `IGNORED_STALE_SUITE_APPS`.
                # 3. The check suite has been queued/pending for longer than `DEPENDABOT_STALE_SUITE_TIMEOUT`.
                if (
                    self.is_dependabot
                    and app_name in IGNORED_STALE_SUITE_APPS
                    and age > DEPENDABOT_STALE_SUITE_TIMEOUT
                ):
                    log.info(
                        "PR #%d HEAD commit %s check suite '%s' (%s) is pending but ignored (queued for %s > %s threshold)",
                        self.pr.number,
                        commit.sha[:8],
                        suite.id,
                        app_name,
                        age,
                        DEPENDABOT_STALE_SUITE_TIMEOUT,
                    )
                    continue

                log.info(
                    "PR #%d HEAD commit %s check suite '%s' (%s) is not completed (status: '%s')",
                    self.pr.number,
                    commit.sha[:8],
                    suite.id,
                    app_name,
                    suite.status,
                )
                return False
            if suite.conclusion not in ("success", "neutral", "skipped"):
                log.info(
                    "PR #%d HEAD commit %s check suite '%s' (%s) failed (conclusion: '%s')",
                    self.pr.number,
                    commit.sha[:8],
                    suite.id,
                    app_name,
                    suite.conclusion,
                )
                return False

        return True

    @cached_property
    def is_pullapprove_green(self) -> bool:
        return self._check_pullapprove_green()

    def _check_pullapprove_green(self) -> bool:
        combined_status = self.commit.get_combined_status()
        for status in combined_status.statuses:
            if status.context == "pullapprove":
                return status.state == "success"
        return False

    @property
    def mergeable(self) -> bool | None:
        return self.pr.mergeable

    @cached_property
    def bot_comments(self) -> list:
        return self._find_bot_comments()

    def _find_bot_comments(self) -> list:
        bot_comments = []
        for comment in self.pr.get_issue_comments():
            comment_user = getattr(comment.user, "login", None)
            if comment_user and comment_user.lower() == self.bot_username:
                if comment.body and ELIGIBILITY_COMMENT_MARKER in comment.body:
                    bot_comments.append(comment)
        return bot_comments

    @cached_property
    def approvals_analysis(self) -> ApprovalsAnalysisResult:
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
        return ApprovalsAnalysisResult(missing_approvals, valid_approvals)

    @property
    def missing_approvals(self) -> dict[str, PlatformGroup]:
        return self.approvals_analysis.missing_approvals

    @property
    def valid_approvals(self) -> dict[str, GroupApproval]:
        return self.approvals_analysis.valid_approvals


@dataclass
class CheckResult:
    """Represents the outcome of a workflow validation check."""

    passed: bool
    message: str = ""
    details: dict[str, Any] = field(default_factory=dict)


class Gate:
    """Represents a gating condition that must be met before checks are run.

    A gate wraps a function that evaluates the PR context and returns a boolean.
    If the gate fails (returns False), an optional `on_fail` callback can be executed
    to perform cleanup or status updates (e.g. removing comments).
    """

    def __init__(
        self,
        fn: Callable[[PRContext], bool],
        on_fail: Callable[[PRContext], None] | None = None,
    ) -> None:
        self.fn = fn
        self.on_fail = on_fail

    def accept(self, context: PRContext) -> bool:
        passed = self.fn(context)
        if not passed and self.on_fail:
            self.on_fail(context)
        return passed


class Workflow:
    """Defines a sequence of gates and checks to process a PR.

    A workflow has a name, a list of gates that must all pass to proceed,
    a list of checks that determine readiness, and callbacks for success (e.g. merge)
    and failure (e.g. post status comment).
    """

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


def gate_is_dependabot(context: PRContext) -> bool:
    """Gate condition: ensures the PR author is dependabot[bot]."""
    if context.is_dependabot:
        return True
    log.info("PR #%d author is not dependabot[bot]. Skipping.", context.pr.number)
    return False


def gate_no_change_requests(context: PRContext) -> bool:
    """Gate condition: ensures the PR has no active changes requested."""
    if not context.change_requesters:
        return True
    log.info(
        "PR #%d has active changes requested by: %s. Skipping.",
        context.pr.number,
        list(context.change_requesters),
    )
    return False


def gate_no_uncovered_files(context: PRContext) -> bool:
    """Gate condition: ensures all changed files belong to platform maintainer groups."""
    if not context.uncovered_files:
        return True
    log.info(
        "PR #%d contains files outside the platform-maintained scope. Skipping. Uncovered files: %s",
        context.pr.number,
        list(context.uncovered_files)[:5],
    )
    return False


def gate_has_active_groups(context: PRContext) -> bool:
    """Gate condition: ensures the PR touches at least one platform group."""
    if context.active_groups:
        return True
    log.info("PR #%d has no changed files in platform groups.", context.pr.number)
    return False


def gate_pullapprove_pending(context: PRContext) -> bool:
    """Gate condition: ensures PullApprove status is not already green (which follows standard flow)."""
    if ValidationCheck.PULLAPPROVE in context.skip_checks:
        return True
    if not context.is_pullapprove_green:
        return True
    log.info(
        "PR #%d has a successful pullapprove check. Skipping bot merge (standard flow applies).",
        context.pr.number,
    )
    return False


# --- Check Functions ---


def check_ci(context: PRContext) -> CheckResult:
    """Check: evaluates whether all CI checks have passed."""
    if ValidationCheck.CI in context.skip_checks:
        return CheckResult(passed=True, message="CI check skipped")
    if context.ci_passed:
        return CheckResult(passed=True)
    return CheckResult(passed=False, message="CI checks are pending or failed.")


def check_unresolved_comments(context: PRContext) -> CheckResult:
    """Check: evaluates whether all review comment threads are resolved."""
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
    """Check: evaluates whether the PR is mergeable cleanly."""
    mergeable = context.mergeable
    if mergeable is True:
        return CheckResult(passed=True)
    if mergeable is False:
        return CheckResult(passed=False, message="PR has merge conflicts.")
    return CheckResult(
        passed=False,
        message="Mergeability state is computing on GitHub.",
        details={"computing": True},
    )


def check_platform_approvals(context: PRContext) -> CheckResult:
    """Check: evaluates whether all required platform maintainer approvals are present."""
    if context.missing_approvals:
        return CheckResult(passed=False, message="Needs platform maintainer approvals.")
    return CheckResult(passed=True)


# --- Action Functions ---


def action_noop(context: PRContext) -> None:
    """No-op action callback."""
    pass


def action_remove_eligibility_comment(context: PRContext) -> None:
    """Deletes any stale eligibility comments posted by the bot on the PR."""
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
    """Merges the PR and posts a status explanation comment."""
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


def action_merge_dependabot(context: PRContext) -> None:
    merge_reason_comment = "### Dependabot Auto-Merge Executed\n"
    merge_reason_comment += (
        "This PR has been automatically merged because it passed all CI checks."
    )

    if context.dry_run:
        log.info(
            "[Dry Run] Would post merge comment to PR #%d:\n%s",
            context.pr.number,
            merge_reason_comment,
        )
        log.info("[Dry Run] Would merge PR #%d (method: squash)", context.pr.number)
    else:
        log.info("Merging PR #%d (Dependabot)", context.pr.number)
        context.pr.merge(
            merge_method="squash",
            commit_title=f"{context.pr.title} (Auto-merged by bot)",
            sha=context.pr.head.sha,
        )
        log.info("Posting merge explanation comment to PR #%d", context.pr.number)
        try:
            context.pr.create_issue_comment(merge_reason_comment)
        except GithubException as e:
            log.error("Failed to post merge comment: %s", e)


def action_post_platform_eligibility(context: PRContext) -> None:
    """Posts or updates a status comment detailing why an eligible PR is not yet ready to merge.

    This function is executed as the `on_failure` action when one or more workflow checks fail.
    """
    comment_body = f"{ELIGIBILITY_COMMENT_MARKER}\n"
    comment_body += "### Platform Maintainers Auto-Merge Info\n"
    comment_body += "This PR is restricted to platform-maintained paths and is eligible for auto-merging upon approval from the designated maintainers.\n\n"
    comment_body += (
        "To merge, we require at least one approval from each of these groups:\n"
    )

    active_groups = context.active_groups
    missing_approvals = context.missing_approvals
    comments_skipped = ValidationCheck.COMMENTS in context.skip_checks
    unresolved_threads = [] if comments_skipped else context.unresolved_threads

    ci_skipped = ValidationCheck.CI in context.skip_checks
    ci_passed = False if ci_skipped else context.ci_passed
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
    comment_body += "⚠️ **PR is not ready to merge yet:**\n"

    if missing_approvals:
        comment_body += "- ❌ Needs platform maintainer approvals (see above).\n"
    else:
        comment_body += "- ✅ Has all platform maintainer approvals.\n"

    if unresolved_threads:
        comment_body += "- ❌ Has unresolved review conversations:\n"
        for thread in unresolved_threads:
            link_part = f" ([Link]({thread.url}))" if thread.url else ""
            comment_preview = (
                f': *"{thread.body_preview}"*' if thread.body_preview else ""
            )
            comment_body += f"  * Unresolved thread by @{thread.author}{link_part}{comment_preview}\n"
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

# PLATFORM_MERGE_WORKFLOW handles PRs that only touch platform-maintained code paths.
PLATFORM_MERGE_WORKFLOW = Workflow(
    name="platform_merge",
    gates=[
        Gate(gate_no_uncovered_files, on_fail=action_remove_eligibility_comment),
        Gate(gate_has_active_groups, on_fail=action_remove_eligibility_comment),
        Gate(gate_pullapprove_pending, on_fail=action_remove_eligibility_comment),
        Gate(gate_no_change_requests),
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

# DEPENDABOT_MERGE_WORKFLOW handles automated dependency update PRs created by Dependabot.
DEPENDABOT_MERGE_WORKFLOW = Workflow(
    name="dependabot_merge",
    gates=[
        Gate(gate_is_dependabot),
        Gate(gate_no_change_requests),
    ],
    checks=[
        check_ci,
        check_unresolved_comments,
        check_mergeable,
    ],
    on_success=action_merge_dependabot,
    on_failure=action_noop,
)

WORKFLOWS = {
    "platform_merge": PLATFORM_MERGE_WORKFLOW,
    "dependabot_merge": DEPENDABOT_MERGE_WORKFLOW,
}


def select_workflow(context: PRContext) -> Workflow | None:
    """Selects the matching workflow to execute based on PR context."""
    if context.is_dependabot:
        return DEPENDABOT_MERGE_WORKFLOW
    if context.active_groups or context.bot_comments:
        return PLATFORM_MERGE_WORKFLOW
    return None


class PrCheckerBot:
    """Orchestrates scanning, checking coverage, and auto-merging PRs using configurable workflows."""

    def __init__(
        self,
        token: str | None,
        repo_name: str,
        config_path: str,
        dry_run: bool,
        skip_checks: list[str] | None = None,
        workflow_name: str | None = None,
    ) -> None:
        self.token = token
        self.repo_name = repo_name
        self.api = Github(token) if token else Github()
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
            if not gate.accept(context):
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
            elif res.message:
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
            if not dry_run:
                raise click.ClickException(
                    f"Require a token. Set environment variable '{token_env}' (or 'GITHUB_TOKEN') or provide --token-file"
                )
            log.warning(
                "No GitHub token provided. Running in unauthenticated dry-run mode."
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
