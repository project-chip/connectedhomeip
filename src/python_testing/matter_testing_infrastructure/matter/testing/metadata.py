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
from io import StringIO
from typing import Any

import yaml

LOGGER = logging.getLogger(__name__)


# A run block's `tags:` list takes any tag name, so a tag that only selects runs with --tag needs no code.
# These tags also change what the runners do with a run.

# The run runs only in the nightly job.
NIGHTLY_TAG = "nightly"
# The run has problems tracked in its `issues:`, such as a failure or probes a deny list skips. The tag doesn't
# change where the run runs. With nightly, a failure is reported with the issues and doesn't fail the job.
NEEDS_FOLLOWUP_FIXES_TAG = "needs-followup-fixes"
# The run needs an app and controller built with OpenSSL 3.5, so it runs only in the OpenSSL 3.5 job.
OPENSSL_3_5_TAG = "openssl-3.5"

# A --tag value with this prefix leaves out the runs that have the tag, so no tag name may start with it.
SKIP_TAG_PREFIX = "skip-"

# Lowercase only, so that a tag differing only in case from nightly can't put a nightly run into PR CI.
_TAG_NAME = re.compile(r"[a-z0-9][a-z0-9.-]*")

# Runs with these tags run only in their own CI job, so a TagFilter leaves them out unless a --tag value names the tag.
_DEDICATED_JOB_TAGS = frozenset({NIGHTLY_TAG, OPENSSL_3_5_TAG})


def is_valid_tag_name(name: str) -> bool:
    """Return whether a run block may use this tag: lowercase letters, digits, '.' and '-', not starting with 'skip-'."""
    return bool(_TAG_NAME.fullmatch(name)) and not name.startswith(SKIP_TAG_PREFIX)


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

    required: frozenset[str] = frozenset()
    skipped: frozenset[str] = _DEDICATED_JOB_TAGS

    @classmethod
    def parse(cls, values: Iterable[str]) -> "TagFilter":
        """Return the filter for --tag values: '<tag>' requires the tag, 'skip-<tag>' leaves out the runs with it.

        Runs with a dedicated job's tag stay left out unless a value names that tag. Raises ValueError for a value
        whose tag isn't a valid tag name. Whether any run has the tag is for the caller to check, with unused_tag_values().
        """
        required: set[str] = set()
        skipped: set[str] = set()
        for value in values:
            name = value.removeprefix(SKIP_TAG_PREFIX)
            if not is_valid_tag_name(name):
                raise ValueError(f"invalid tag in {value!r}, expected <tag> or {SKIP_TAG_PREFIX}<tag> with a lowercase tag name")
            (skipped if value.startswith(SKIP_TAG_PREFIX) else required).add(name)
        return cls(frozenset(required), frozenset((_DEDICATED_JOB_TAGS - required) | skipped))

    def matches(self, tags: list[str]) -> bool:
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


def unused_tag_values(values: Iterable[str], used_tags: set[str]) -> list[str]:
    """Return the --tag values that name a tag no run has, such as a misspelled tag.

    used_tags must be the tags of every run the runner could select, before any other filter, so that a value
    doesn't count as unused only because, for example, a --regex left out the runs with the tag.
    """
    return [value for value in values if value.removeprefix(SKIP_TAG_PREFIX) not in used_tags]


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
    tags: list[str] = field(default_factory=list)
    issues: list[str] = field(default_factory=list)


def runs_in_pull_requests(tags: list[str]) -> bool:
    """Return whether PR CI runs a block with these tags."""
    return TagFilter().matches(tags)


def script_runs_in_pull_requests(runs: dict[str, dict[str, Any]]) -> bool:
    """Return whether PR CI calls a script with these runs, as returned by extract_runs_args().

    PR CI calls a script that has a run PR CI runs. It also calls a script with no runs, so that
    run_python_test.py reports the missing CI test arguments.
    """
    return TagFilter().matches_script(runs)


def failure_fails_job(tags: list[str]) -> bool:
    """Return whether a failure of a block with these tags fails the job that runs it.

    Only a nightly block with tracked issues can fail without failing its job, so every PR CI failure fails PR CI.
    """
    return not (NIGHTLY_TAG in tags and NEEDS_FOLLOWUP_FIXES_TAG in tags)


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
        raise error(f"tags must be a list such as [{NIGHTLY_TAG}], got {tags!r}")
    invalid_tags = [tag for tag in tags if not is_valid_tag_name(tag)]
    if invalid_tags:
        raise error(f"invalid tags {invalid_tags}: a tag is a lowercase name of letters, digits, '.' and '-' "
                    f"that doesn't start with {SKIP_TAG_PREFIX!r}")

    issues = args.get("issues", [])
    if not isinstance(issues, list) or not all(isinstance(issue, str) for issue in issues):
        raise error(f"issues must be a list of issue URLs, one per line, got {issues!r}")
    invalid_issues = [issue for issue in issues if not _ISSUE_URL.fullmatch(issue)]
    if invalid_issues:
        raise error(f"issues must be GitHub issue URLs (https://github.com/<owner>/<repo>/issues/<number>), got {invalid_issues}")
    if NEEDS_FOLLOWUP_FIXES_TAG in tags and not issues:
        raise error(f"{NEEDS_FOLLOWUP_FIXES_TAG} needs issues: the issues that track the problems")
    if issues and NEEDS_FOLLOWUP_FIXES_TAG not in tags:
        raise error(f"issues is only used with the {NEEDS_FOLLOWUP_FIXES_TAG} tag")

    args["tags"] = tags
    args["issues"] = issues


def extract_runs_args(py_script_path: str) -> dict[str, dict[str, Any]]:
    """Extract the run arguments from the CI test arguments blocks.

    A script without a CI test arguments block has no runs. Every run gets `tags` and `issues`
    lists. Raises RunArgumentsError when `test-runner-runs` or a run is empty or not a mapping, or
    when a run's tags or issues are not valid.
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
            test_runner_runs = runs.get("test-runner-runs", {})
            if not isinstance(test_runner_runs, dict):
                raise RunArgumentsError(
                    f"{py_script_path}: test-runner-runs must list the runs with their arguments, got {test_runner_runs!r}")
            for run, args in test_runner_runs.items():
                if not isinstance(args, dict):
                    raise RunArgumentsError(
                        f"{py_script_path}: run '{run}': expected arguments such as app and script-args, got {args!r}")
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

        Raises RunArgumentsError when `test-runner-runs` or a run is empty or not a mapping, or
        when a run's tags or issues are not valid.
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
