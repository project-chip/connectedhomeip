#!/usr/bin/env python3
#
# Copyright (c) 2024 Project CHIP Authors
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
import re
from typing import Optional, Set

import click
import coloredlogs
from github import Github

__LOG_LEVELS__ = {
    "debug": logging.DEBUG,
    "info": logging.INFO,
    "warn": logging.WARN,
    "fatal": logging.FATAL,
}

REPOSITORY = "project-chip/connectedhomeip"


class Canceller:
    def __init__(self, token):
        self.api = Github(token)
        self.repo = self.api.get_repo(REPOSITORY)

    def cancel_all_runs(
        self,
        pr_number: int,
        commit_sha: str,
        skip_workflow: Optional[str],
        dry_run: bool,
    ):
        pr = self.repo.get_pull(pr_number)
        logging.info("Examining PR '%s'", pr.title)
        logging.info("Looking to cancel for commit '%s'", commit_sha)
        if skip_workflow is not None:
            logging.info("Will skip workflow named '%s'", skip_workflow)

        stopped: Set[int] = set()

        for commit in pr.get_commits():
            if commit.sha != commit_sha:
                logging.info("Skipping SHA '%s' as it was not selected", commit.sha)
                continue

            for check_suite in commit.get_check_suites():
                for run in check_suite.get_check_runs():
                    if run.status not in {"in_progress", "queued"}:
                        logging.info("Skip over run %s (%s)", run.name, run.status)
                        continue
                    # TODO: I am unclear how to really find the workflow id, however the
                    #       HTML URL is like https://github.com/project-chip/connectedhomeip/actions/runs/11261874698/job/31316278705
                    #       so need whatever is after run.
                    m = re.match(r".*/actions/runs/([\d]+)/job/.*", run.html_url)
                    if not m:
                        logging.error(
                            "Failed to extract workflow number from %s", run.html_url
                        )
                        continue

                    workflow_id = int(m.group(1))
                    workflow = self.repo.get_workflow_run(workflow_id)

                    if workflow_id in stopped:
                        logging.info(
                            "Workflow %s (id %d) already stopped.",
                            workflow.name,
                            workflow_id,
                        )
                        continue

                    stopped.add(workflow_id)

                    if workflow.name == skip_workflow:
                        logging.info("Workflow %s marked as skip.", workflow.name)
                        continue

                    if dry_run:
                        logging.warning("DRY RUN: Will not stop run %s", run.name)
                    else:
                        workflow.cancel()
                        logging.warning("Stopping run %s", run.name)


@click.command()
@click.option(
    "--log-level",
    default="INFO",
    type=click.Choice(list(__LOG_LEVELS__.keys()), case_sensitive=False),
    help="Determines the verbosity of script output.",
)
@click.option("--pull-request", type=int, help="Pull request number to consider")
@click.option("--commit-sha", help="Commit to look at when cancelling pull requests")
@click.option("--gh-api-token", help="Github token to use")
@click.option("--token-file", help="Read github token from the given file")
@click.option("--dry-run", default=False, is_flag=True, help="Actually cancel or not")
@click.option(
    "--skip-workflow", default=None, help="Name of the workflow NOT to cancel"
)
def main(
    log_level,
    pull_request,
    commit_sha,
    gh_api_token,
    token_file,
    dry_run,
    skip_workflow,
):
    coloredlogs.install(
        level=__LOG_LEVELS__[log_level], fmt="%(asctime)s %(levelname)-7s %(message)s"
    )

    if gh_api_token:
        gh_token = gh_api_token
    elif token_file:
        gh_token = open(token_file, "rt").read().strip()
    else:
        raise Exception("Require a --gh-api-token or --token-file to access github")

    Canceller(gh_token).cancel_all_runs(
        pull_request, commit_sha, skip_workflow, dry_run
    )


if __name__ == "__main__":
    main()
