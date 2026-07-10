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
from typing import Iterable, NamedTuple

import click
import coloredlogs
import pathspec
import yaml
from github import Github, GithubException
from github.Commit import Commit
from github.PullRequest import PullRequest

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


class PrCheckerBot:
    """Orchestrates scanning, checking coverage, and auto-merging PRs that affect platform-maintained paths."""

    def __init__(
        self,
        token: str,
        repo_name: str,
        config_path: str,
        dry_run: bool,
        skip_checks: list[str] | None = None,
    ) -> None:
        self.token = token
        self.repo_name = repo_name
        self.api = Github(token)
        self.repo = self.api.get_repo(repo_name)
        self.config_path = config_path
        self.dry_run = dry_run
        self.skip_checks = {ValidationCheck(c) for c in (skip_checks or [])}
        self.single_pr_mode = False
        self.groups: dict[str, PlatformGroup] = {}
        self._bot_username = None
        self.load_config()

    @property
    def bot_username(self) -> str:
        """Retrieves the username of the authenticated bot, falling back to a default on failure."""
        if self._bot_username is None:
            try:
                self._bot_username = self.api.get_user().login.lower()
            except GithubException:
                self._bot_username = "pr-checker-bot"
        return self._bot_username

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

    def get_pr_review_states(self, pr: PullRequest) -> tuple[set[str], set[str]]:
        """Returns the set of users who have currently approved and those who have requested changes."""
        user_reviews = {}
        # get_reviews() returns reviews chronologically
        for review in pr.get_reviews():
            review_user = getattr(review.user, "login", None)
            if not review_user:
                continue
            user = review_user.lower()
            if review.state in ("APPROVED", "CHANGES_REQUESTED", "DISMISSED"):
                user_reviews[user] = review.state

        approvers = {
            user for user, state in user_reviews.items() if state == "APPROVED"
        }
        # Exclude author from self-approval
        pr_author = getattr(pr.user, "login", None)
        author = pr_author.lower() if pr_author else ""
        approvers.discard(author)
        change_requesters = {
            user for user, state in user_reviews.items() if state == "CHANGES_REQUESTED"
        }
        return approvers, change_requesters

    def analyze_pr_files(self, pr: PullRequest) -> tuple[dict[str, set[str]], set[str]]:
        """Analyzes the files changed in the PR.

        Returns:
          - matched_files_per_group: Map of group_name -> set of files matching it.
          - uncovered_files: Set of files that matched no group.
        """
        matched_files_per_group: dict[str, set[str]] = {
            name: set() for name in self.groups
        }
        uncovered_files = set()

        # pr.get_files() returns PaginatedList of File objects
        for pr_file in pr.get_files():
            filepaths_to_check = [pr_file.filename]
            prev_filepath = getattr(pr_file, "previous_filename", None)
            if prev_filepath:
                filepaths_to_check.append(prev_filepath)

            for filepath in filepaths_to_check:
                matched_any = False
                for group_name, group in self.groups.items():
                    if group.matches_file(filepath):
                        matched_files_per_group[group_name].add(filepath)
                        matched_any = True

                if not matched_any:
                    uncovered_files.add(filepath)

        return matched_files_per_group, uncovered_files

    def check_and_process_pr(self, pr: PullRequest) -> None:
        """Checks the coverage and approvals for a single PR, and merges if eligible."""
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

        # Perform file analysis first (saves API calls for ineligible PRs)
        matched_files, uncovered_files = self.analyze_pr_files(pr)

        if uncovered_files:
            log.info(
                "PR #%d contains files outside the platform-maintained scope. Skipping. Uncovered files: %s",
                pr.number,
                list(uncovered_files)[:5],
            )
            self.remove_eligibility_comment(pr)
            return

        # Determine which groups are active (i.e. have changed files)
        active_groups = {name: files for name, files in matched_files.items() if files}
        if not active_groups:
            log.info("PR #%d has no changed files? Skipping.", pr.number)
            self.remove_eligibility_comment(pr)
            return

        log.info(
            "PR #%d is fully covered by platform groups: %s",
            pr.number,
            list(active_groups.keys()),
        )

        # Get the commit object once for subsequent status/CI checks
        commit = self.repo.get_commit(sha=pr.head.sha)

        if (
            ValidationCheck.PULLAPPROVE not in self.skip_checks
            and self._is_pullapprove_green(commit)
        ):
            log.info(
                "PR #%d has a successful pullapprove check. Skipping bot merge (standard flow applies).",
                pr.number,
            )
            self.remove_eligibility_comment(pr)
            return

        # Get current approvals and change requests
        approvers, change_requesters = self.get_pr_review_states(pr)
        log.debug(
            "PR #%d current approvers: %s, change requesters: %s",
            pr.number,
            list(approvers),
            list(change_requesters),
        )

        if change_requesters:
            log.info(
                "PR #%d has active changes requested by: %s. Skipping.",
                pr.number,
                list(change_requesters),
            )
            return

        # Check approvals for each active group
        missing_approvals: dict[str, PlatformGroup] = {}
        valid_approvals_per_group: dict[str, GroupApproval] = {}

        for group_name, files in active_groups.items():
            group = self.groups[group_name]
            group_approvers = approvers.intersection(group.maintainers)
            if not group_approvers:
                missing_approvals[group_name] = group
            else:
                # Pick the first matched approver for documentation
                valid_approvals_per_group[group_name] = GroupApproval(
                    sorted(group_approvers)[0], files
                )

        unresolved_threads = []
        if ValidationCheck.COMMENTS not in self.skip_checks:
            unresolved_threads = self._get_unresolved_threads(pr)

        ci_passed = True
        if ValidationCheck.CI not in self.skip_checks:
            ci_passed = self._has_ci_passed(pr, commit)

        is_mergeable = pr.mergeable
        is_ready = (
            not missing_approvals
            and not unresolved_threads
            and ci_passed
            and is_mergeable is True
        )

        if not is_ready:
            log.info(
                "PR #%d is eligible but not ready to merge. Updating status comment.",
                pr.number,
            )
            self.post_eligibility_comment(
                pr,
                active_groups,
                missing_approvals,
                unresolved_threads,
                ci_passed,
                is_mergeable,
            )
            return

        log.info(
            "PR #%d is fully approved, CI passed, no unresolved comments, and ready to merge!",
            pr.number,
        )
        self.merge_pr(pr, valid_approvals_per_group)

    def _is_pullapprove_green(self, commit: Commit) -> bool:
        """Returns True if the pullapprove check exists and is in the 'success' state."""
        combined_status = commit.get_combined_status()
        for status in combined_status.statuses:
            if status.context == "pullapprove":
                return status.state == "success"
        return False

    def _get_unresolved_threads(self, pr: PullRequest) -> list[dict]:
        """Queries GitHub GraphQL API to find active unresolved review threads on the PR."""
        owner, repo_name = self.repo_name.split("/")
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
        variables = {"owner": owner, "name": repo_name, "number": pr.number}

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
                        pr.number,
                    )
                    unresolved.append(
                        {
                            "author": "system",
                            "body_preview": "Too many review threads (>100). Please resolve or clean up threads.",
                            "url": pr.html_url + "/files",
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
                            body_preview = " ".join(
                                first_comment["body"].split()
                            )
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
                pr.number,
                e,
            )
            raise

        return unresolved

    def _has_ci_passed(self, pr: PullRequest, commit: Commit) -> bool:
        """Checks if all CI checks (combined status and check runs) have passed on the PR's latest commit."""
        combined_status = commit.get_combined_status()
        check_suites = list(commit.get_check_suites())

        # Guard against empty checks / premature success when commit is fresh.
        # A normal PR run has at least 10 combined statuses and check suites.
        total_checks = len(combined_status.statuses) + len(check_suites)
        if total_checks < 10:
            log.info(
                "PR #%d HEAD commit %s has only %d CI checks registered (expected >= 10). Treating as pending.",
                pr.number,
                commit.sha[:8],
                total_checks,
            )
            return False

        # pullapprove is ignored because it delegates normal PR approvals. Since this
        # bot bypasses standard reviews for platform-restricted changes, PullApprove
        # will remain pending forever.
        ignored_contexts = {"pullapprove"}
        for status in combined_status.statuses:
            if status.context in ignored_contexts:
                continue
            if status.state != "success":
                log.info(
                    "PR #%d HEAD commit %s status '%s' is '%s' (%s)",
                    pr.number,
                    pr.head.sha[:8],
                    status.context,
                    status.state,
                    status.description,
                )
                return False

        for suite in check_suites:
            if suite.status != "completed":
                log.info(
                    "PR #%d HEAD commit %s check suite '%s' is not completed (status: '%s')",
                    pr.number,
                    pr.head.sha[:8],
                    suite.id,
                    suite.status,
                )
                return False
            if suite.conclusion not in ("success", "neutral", "skipped"):
                log.info(
                    "PR #%d HEAD commit %s check suite '%s' failed (conclusion: '%s')",
                    pr.number,
                    pr.head.sha[:8],
                    suite.id,
                    suite.conclusion,
                )
                return False

        return True

    def _find_bot_comments(self, pr: PullRequest) -> list:
        """Finds comments posted by this bot on the PR."""
        bot_comments = []
        for comment in pr.get_issue_comments():
            comment_user = getattr(comment.user, "login", None)
            if comment_user and comment_user.lower() == self.bot_username:
                if comment.body and ELIGIBILITY_COMMENT_MARKER in comment.body:
                    bot_comments.append(comment)
        return bot_comments

    def post_eligibility_comment(
        self,
        pr: PullRequest,
        active_groups: dict[str, set[str]],
        missing_approvals: dict[str, PlatformGroup],
        unresolved_threads: list[dict],
        ci_passed: bool,
        mergeable: bool | None,
    ) -> None:
        """Posts or updates a comment stating the auto-merge status of the PR."""
        # Generate comment body first so we can compare it
        comment_body = f"{ELIGIBILITY_COMMENT_MARKER}\n"
        comment_body += "### Platform Maintainers Auto-Merge Info\n"
        comment_body += "This PR is restricted to platform-maintained paths and is eligible for auto-merging upon approval from the designated maintainers.\n\n"
        comment_body += (
            "To merge, we require at least one approval from each of these groups:\n"
        )

        for group_name, files in active_groups.items():
            group = self.groups[group_name]
            maintainer_mentions = ", ".join([f"@{m}" for m in group.maintainers])
            status = (
                "❌ Needs approval"
                if group_name in missing_approvals
                else "✅ Approved"
            )
            comment_body += f"- **{group_name}**: {maintainer_mentions} ({status})\n"
            comment_body += "  *Paths matched:*\n"
            for glob in group.get_matched_globs(files):
                comment_body += f"    * `{glob}`\n"

        comment_body += "\n### Merge Requirements Status\n"
        if not missing_approvals and not unresolved_threads and ci_passed and mergeable is True:
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
                    comment_preview = f': *"{thread["body_preview"]}"*' if thread["body_preview"] else ""
                    comment_body += f"  * Unresolved thread by @{thread['author']}{link_part}{comment_preview}\n"
            else:
                comment_body += "- ✅ All review conversations resolved.\n"

            if ci_passed:
                comment_body += "- ✅ All CI status and check suites passed.\n"
            else:
                comment_body += "- ❌ CI checks are pending or failed.\n"

            if mergeable is True:
                comment_body += "- ✅ No merge conflicts.\n"
            elif mergeable is False:
                comment_body += "- ❌ PR has merge conflicts (resolve conflicts before merge).\n"
            else:
                comment_body += "- ⚠️ Mergeability state is computing on GitHub.\n"

        bot_comments = self._find_bot_comments(pr)
        if bot_comments:
            main_comment = bot_comments[0]
            if main_comment.body.strip() != comment_body.strip():
                if self.dry_run:
                    log.info(
                        "[Dry Run] Would update eligibility comment on PR #%d",
                        pr.number,
                    )
                else:
                    log.info("Updating eligibility comment on PR #%d", pr.number)
                    main_comment.edit(comment_body)
            else:
                log.debug(
                    "PR #%d already has an up-to-date eligibility comment.",
                    pr.number,
                )

            for duplicate in bot_comments[1:]:
                if self.dry_run:
                    log.info(
                        "[Dry Run] Would delete duplicate eligibility comment on PR #%d",
                        pr.number,
                    )
                else:
                    log.info(
                        "Deleting duplicate eligibility comment on PR #%d",
                        pr.number,
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
            if self.dry_run:
                log.info(
                    "[Dry Run] Would post eligibility comment to PR #%d:\n%s",
                    pr.number,
                    comment_body,
                )
            else:
                log.info("Posting eligibility comment to PR #%d", pr.number)
                pr.create_issue_comment(comment_body)

    def remove_eligibility_comment(self, pr: PullRequest) -> None:
        """Removes the eligibility comment if it exists on the PR."""
        if pr.comments == 0:
            return
        for comment in self._find_bot_comments(pr):
            if self.dry_run:
                log.info(
                    "[Dry Run] Would delete stale eligibility comment on PR #%d",
                    pr.number,
                )
            else:
                log.info(
                    "Deleting stale eligibility comment on PR #%d",
                    pr.number,
                )
                try:
                    comment.delete()
                except GithubException as e:
                    log.error("Failed to delete stale comment #%d: %s", comment.id, e)

    def merge_pr(
        self, pr: PullRequest, valid_approvals_per_group: dict[str, GroupApproval]
    ) -> None:
        """Merges the PR and posts a comment explaining the approvals."""
        # Generate merge comment explaining reasons
        merge_reason_comment = "### Platform Maintainers Auto-Merge Executed\n"
        merge_reason_comment += "This PR has been automatically merged. It contains changes restricted to platform-maintained paths and received the required maintainer approvals:\n\n"

        for group_name, approval in valid_approvals_per_group.items():
            group = self.groups[group_name]
            matched_globs = group.get_matched_globs(approval.files)
            globs_str = ", ".join([f"`{g}`" for g in matched_globs])
            merge_reason_comment += f"- **{group_name}** changes (matching {globs_str}) approved by @{approval.approver}\n"

        if self.dry_run:
            log.info(
                "[Dry Run] Would post merge comment to PR #%d:\n%s",
                pr.number,
                merge_reason_comment,
            )
            log.info("[Dry Run] Would merge PR #%d (method: squash)", pr.number)
        else:
            log.info("Merging PR #%d", pr.number)
            # Use squash merge
            pr.merge(
                merge_method="squash",
                commit_title=f"{pr.title} (Auto-merged by platform-bot)",
                sha=pr.head.sha,
            )

            log.info("Posting merge explanation comment to PR #%d", pr.number)
            try:
                pr.create_issue_comment(merge_reason_comment)
            except GithubException as e:
                log.error(
                    "Failed to post merge explanation comment to PR #%d: %s",
                    pr.number,
                    e,
                )

    def run(self, pr_number: int | None = None) -> None:
        """Runs the bot, either scanning all open PRs or processing a single PR."""
        self.single_pr_mode = pr_number is not None
        has_errors = False
        if self.single_pr_mode:
            log.info("Processing single PR #%d...", pr_number)
            try:
                pr = self.repo.get_pull(pr_number)
                self.check_and_process_pr(pr)
            except Exception as e:
                log.exception("Error processing PR #%d: %s", pr_number, e)
                has_errors = True
        else:
            log.info("Scanning open pull requests...")
            open_prs = self.repo.get_pulls(state="open")
            for pr in open_prs:
                try:
                    self.check_and_process_pr(pr)
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
def main(
    log_level: str,
    token_env: str,
    token_file: str | None,
    repo: str,
    config: str,
    dry_run: bool,
    pr: int | None,
    skip_check: tuple[str, ...],
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
        gh_token, repo, config, dry_run, skip_checks=list(skip_check)
    )
    bot.run(pr_number=pr)


if __name__ == "__main__":
    main()
