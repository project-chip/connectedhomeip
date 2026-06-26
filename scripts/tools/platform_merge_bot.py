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

import logging
import os
from typing import Iterable, NamedTuple

import click
import coloredlogs
import pathspec
import yaml
from github import Github, GithubException
from github.PullRequest import PullRequest

log = logging.getLogger(__name__)

_LOG_LEVELS = logging.getLevelNamesMapping()

DEFAULT_REPOSITORY = "project-chip/connectedhomeip"
DEFAULT_CONFIG_PATH = ".github/platform_maintainers.yaml"

ELIGIBILITY_COMMENT_MARKER = "<!-- platform-merge-bot-eligibility-marker -->"


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


class PlatformMergeBot:
    """Orchestrates scanning, checking coverage, and auto-merging PRs that affect platform-maintained paths."""

    def __init__(
        self, token: str, repo_name: str, config_path: str, dry_run: bool
    ) -> None:
        self.api = Github(token)
        self.repo = self.api.get_repo(repo_name)
        self.config_path = config_path
        self.dry_run = dry_run
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
                self._bot_username = "platform-merge-bot"
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
        pr_author = getattr(pr.user, "login", None) or "ghost"
        log.info("Checking PR #%d: '%s' (Author: %s)", pr.number, pr.title, pr_author)

        if pr.draft:
            log.info("PR #%d is a draft. Skipping.", pr.number)
            return

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

        if missing_approvals:
            log.info(
                "PR #%d is missing approvals from platform maintainers for groups: %s",
                pr.number,
                list(missing_approvals.keys()),
            )
            self.post_eligibility_comment(pr, active_groups, missing_approvals)
            return

        # If we got here, all active groups have at least one approval. We can merge!
        log.info("PR #%d is fully approved and ready to merge!", pr.number)
        self.merge_pr(pr, valid_approvals_per_group)

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

    def run(self) -> None:
        """Scans all open pull requests and processes them."""
        log.info("Scanning open pull requests...")
        open_prs = self.repo.get_pulls(state="open")
        for pr in open_prs:
            try:
                self.check_and_process_pr(pr)
            except Exception as e:
                log.exception("Error processing PR #%d: %s", pr.number, e)


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
def main(
    log_level: str,
    token_env: str,
    token_file: str | None,
    repo: str,
    config: str,
    dry_run: bool,
) -> None:
    """Platform Merge Bot entry point."""
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

    bot = PlatformMergeBot(gh_token, repo, config, dry_run)
    bot.run()


if __name__ == "__main__":
    main()
