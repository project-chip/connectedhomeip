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

import logging
import re
from collections.abc import Iterable
from dataclasses import dataclass, field
from enum import StrEnum
from io import StringIO
from typing import Any

import yaml

LOGGER = logging.getLogger(__name__)


class RunTag(StrEnum):
    """Values for the `tags:` list of a run block."""

    # The block runs only in the nightly job.
    NIGHTLY = "nightly"
    # The block has problems tracked in its `issues:`, such as a failure or probes a deny list skips. The tag doesn't
    # change where the block runs. With nightly, a failure is reported with the issues and doesn't fail the job.
    NEEDS_FOLLOWUP_FIXES = "needs-followup-fixes"
    # The block needs an app and controller built with OpenSSL 3.5, so it runs only in the OpenSSL 3.5 job.
    OPENSSL_3_5 = "openssl-3.5"


# A --tag value with this prefix leaves out the runs that have the tag. No RunTag value may start with it.
SKIP_TAG_PREFIX = "skip-"

# Runs with these tags run only in their own CI job, so a TagFilter leaves them out unless a --tag value names the tag.
_DEDICATED_JOB_TAGS = frozenset({RunTag.NIGHTLY, RunTag.OPENSSL_3_5})

TAG_OPTION_HELP = (
    f"Select runs by tag: '<tag>' keeps only the runs with the tag, '{SKIP_TAG_PREFIX}<tag>' leaves them out. "
    "Repeat to combine; a run must match every value. "
    f"Runs tagged {' or '.join(sorted(_DEDICATED_JOB_TAGS))} are left out unless a value names the tag."
)


@dataclass(frozen=True)
class TagFilter:
    """Selects runs by their tags, as the runners' --tag option does.

    A run must have every required tag and none of the skipped tags. TagFilter() selects the runs PR CI runs.
    """

    required: frozenset[RunTag] = frozenset()
    skipped: frozenset[RunTag] = _DEDICATED_JOB_TAGS

    @classmethod
    def parse(cls, values: Iterable[str]) -> "TagFilter":
        """Return the filter for --tag values: '<tag>' requires the tag, 'skip-<tag>' leaves out the runs with it.

        Runs with a dedicated job's tag stay left out unless a value names that tag. Raises ValueError for an unknown tag.
        """
        known_tags = [tag.value for tag in RunTag]
        required = set()
        skipped = set()
        for value in values:
            name = value.removeprefix(SKIP_TAG_PREFIX)
            if name not in known_tags:
                raise ValueError(f"unknown tag in {value!r}, expected <tag> or {SKIP_TAG_PREFIX}<tag> with a tag from {known_tags}")
            (skipped if value.startswith(SKIP_TAG_PREFIX) else required).add(RunTag(name))
        return cls(frozenset(required), frozenset((_DEDICATED_JOB_TAGS - required) | skipped))

    def matches(self, tags: list[RunTag]) -> bool:
        """Return whether the filter selects a run with these tags."""
        return self.required.issubset(tags) and self.skipped.isdisjoint(tags)

    def matches_script(self, runs: dict[str, dict[str, Any]]) -> bool:
        """Return whether a runner calls a script with these runs, as returned by extract_runs_args().

        A runner calls a script that has a matching run. It also calls a script with no runs when a run without
        tags would match, so that run_python_test.py reports the missing CI test arguments.
        """
        if not runs:
            return self.matches([])
        return any(self.matches(args["tags"]) for args in runs.values())

    def __str__(self) -> str:
        return " ".join(sorted([*self.required, *(f"{SKIP_TAG_PREFIX}{tag}" for tag in self.skipped)]))


_ISSUE_URL = re.compile(r"https://github\.com/[\w.-]+/[\w.-]+/issues/\d+")


class RunArgumentsError(ValueError):
    """A run block's CI test arguments are not valid."""


@dataclass
class Metadata:
    py_script_path: str
    run: str
    app: str = ""
    app_args: str | None = None
    app_ready_pattern: str | None = None
    app_stdin_pipe: str | None = None
    script_args: str | None = None
    factory_reset: bool = False
    factory_reset_app_only: bool = False
    script_gdb: bool = False
    timeout: float | None = None
    quiet: bool = False
    pre_existing_fabric: bool = False
    tags: list[RunTag] = field(default_factory=list)
    issues: list[str] = field(default_factory=list)


def runs_in_pull_requests(tags: list[RunTag]) -> bool:
    """Return whether PR CI runs a block with these tags."""
    return TagFilter().matches(tags)


def script_runs_in_pull_requests(runs: dict[str, dict[str, Any]]) -> bool:
    """Return whether PR CI calls a script with these runs, as returned by extract_runs_args().

    PR CI calls a script that has a run PR CI runs. It also calls a script with no runs, so that
    run_python_test.py reports the missing CI test arguments.
    """
    return TagFilter().matches_script(runs)


def failure_fails_job(tags: list[RunTag]) -> bool:
    """Return whether a failure of a block with these tags fails the job that runs it.

    Only a nightly block with tracked issues can fail without failing its job, so every PR CI failure fails PR CI.
    """
    return not (RunTag.NIGHTLY in tags and RunTag.NEEDS_FOLLOWUP_FIXES in tags)


class NamedStringIO(StringIO):
    def __init__(self, content, name):
        super().__init__(content)
        self.name = name


def _validate_tags_and_issues(py_script_path: str, run: str, args: dict[str, Any]) -> None:
    """Check a block's `tags` and `issues`, and set both in `args` (empty lists when absent)."""

    def error(message: str) -> RunArgumentsError:
        return RunArgumentsError(f"{py_script_path}: run '{run}': {message}")

    tags = args.get("tags", [])
    if not isinstance(tags, list) or not all(isinstance(tag, str) for tag in tags):
        raise error(f"tags must be a list such as [{RunTag.NIGHTLY}], got {tags!r}")
    known_tags = [tag.value for tag in RunTag]
    unknown_tags = [tag for tag in tags if tag not in known_tags]
    if unknown_tags:
        raise error(f"unknown tags {unknown_tags}, expected some of {known_tags}")
    tags = [RunTag(tag) for tag in tags]

    issues = args.get("issues", [])
    if not isinstance(issues, list) or not all(isinstance(issue, str) for issue in issues):
        raise error(f"issues must be a list of issue URLs, one per line, got {issues!r}")
    invalid_issues = [issue for issue in issues if not _ISSUE_URL.fullmatch(issue)]
    if invalid_issues:
        raise error(f"issues must be GitHub issue URLs (https://github.com/<owner>/<repo>/issues/<number>), got {invalid_issues}")
    if RunTag.NEEDS_FOLLOWUP_FIXES in tags and not issues:
        raise error(f"{RunTag.NEEDS_FOLLOWUP_FIXES} needs issues: the issues that track the problems")
    if issues and RunTag.NEEDS_FOLLOWUP_FIXES not in tags:
        raise error(f"issues is only used with the {RunTag.NEEDS_FOLLOWUP_FIXES} tag")

    args["tags"] = tags
    args["issues"] = issues


def extract_runs_args(py_script_path: str) -> dict[str, dict[str, Any]]:
    """Extract the run arguments from the CI test arguments blocks.

    A script without a CI test arguments block has no runs. Every run gets `tags` and `issues`
    lists. Raises RunArgumentsError when a run's tags or issues are not valid.
    """

    found_ci_args_section = False
    runs_arg_lines: dict[str, dict[str, Any]] = {}

    ci_args_section_lines = []
    with open(py_script_path, encoding='utf8') as py_script:
        for line_idx, line in enumerate(py_script.readlines()):
            line = line.strip()

            # Append empty line to the line capture, so during YAML parsing
            # line numbers will match the original file.
            ci_args_section_lines.append("")

            # Detect the single CI args section, to skip the lines otherwise.
            if line.startswith("# === BEGIN CI TEST ARGUMENTS ==="):
                found_ci_args_section = True
                continue
            if line.startswith("# === END CI TEST ARGUMENTS ==="):
                break

            if found_ci_args_section:
                # Update the last line in the line capture.
                ci_args_section_lines[-1] = " " + line.lstrip("#")

    if not runs_arg_lines:
        try:
            runs = yaml.safe_load(NamedStringIO("\n".join(ci_args_section_lines), py_script_path)) or {}
            for run, args in runs.get("test-runner-runs", {}).items():
                runs_arg_lines[run] = {}
                runs_arg_lines[run]['run'] = run
                runs_arg_lines[run].update(args)
                _validate_tags_and_issues(py_script_path, run, runs_arg_lines[run])
        except yaml.YAMLError as e:
            LOGGER.error("Failed to parse CI arguments YAML: %s", e)

    return runs_arg_lines


class MetadataReader:
    """
    A class to parse run arguments from the test scripts and
    resolve them to environment specific values.
    """

    def __init__(self, env_yaml_file_path: str):
        """
        Reads the YAML file and Constructs the environment object

        Parameters:

        env_yaml_file_path:
         Path to the environment file that contains the YAML configuration.
        """
        with open(env_yaml_file_path) as stream:
            self.env: dict[str, str] = yaml.safe_load(stream)

    def __resolve_env_vals__(self, metadata_dict: dict[str, str]) -> None:
        """
        Resolves the argument defined in the test script to environment values.
        For example, if a test script defines "all_clusters" as the value for app
        name, we will check the environment configuration to see what raw value is
        associated with the "all_cluster" variable and set the value for "app" option
        to this raw value.

        Parameter:

        metadata_dict:
         Dictionary where each key represent a particular argument and its value represent
         the value for that argument defined in the test script.
        """
        for arg, arg_val in metadata_dict.items():
            if not isinstance(arg_val, str):
                continue
            # We do not expect to recurse (like ${FOO_${BAR}}) so just expand once
            for name, value in self.env.items():
                arg_val = arg_val.replace(f'${{{name}}}', value)
            metadata_dict[arg] = arg_val.strip()

    def parse_script(self, py_script_path: str) -> list[Metadata]:
        """
        Parses a script and returns a list of metadata object where
        each element of that list representing run arguments associated
        with a particular run.

        Parameter:

        py_script_path:
         path to the python test script

        Return:

        List[Metadata]
         List of Metadata object where each Metadata element represents
         the run arguments associated with a particular run defined in
         the script file.

        Raises RunArgumentsError when a run's tags or issues are not valid.
        """
        runs_metadata: list[Metadata] = []
        runs_args = extract_runs_args(py_script_path)

        for run, attr in runs_args.items():
            self.__resolve_env_vals__(attr)
            runs_metadata.append(Metadata(
                py_script_path=py_script_path,
                run=run,
                app=attr.get("app", ""),
                app_args=attr.get("app-args"),
                app_ready_pattern=attr.get("app-ready-pattern"),
                app_stdin_pipe=attr.get("app-stdin-pipe"),
                script_args=attr.get("script-args"),
                factory_reset=str(attr.get("factory-reset", False)).lower() == 'true',
                timeout=float(attr["timeout"]) if "timeout" in attr else None,
                quiet=str(attr.get("quiet", True)).lower() == 'true',
                pre_existing_fabric=str(attr.get("pre-existing-fabric", False)).lower() == 'true',
                tags=attr["tags"],
                issues=attr["issues"],
            ))

        return runs_metadata
