# Copyright (c) 2022 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Build targets are generally of the form `target + modifiers`
#  - `target` defines the platform-specific application to build. It is often
#    in the form of `platform-board-app` but may be `platform-app` as well
#
#  - `modifiers` are additional compilation options like disabling BLE, enabling
#    coverage or other options that are generally passed into gn configurations
#
# Examples:
#  - linux-x64-chip-tool:  this is a 'chip-tool' build for a 64-bit linux
#  - linux-x64-chip-tool-noble-coverage: a chip tool build with modifiers attached of
#    "noble" and "coverage"
#  - qpg-light: a 'light' app built for qpg
#  - imx-thermostat-release: a 'thermostat` build for imx, with a `release` modifier applied
#
# Interpretation of a specific string of `a-b-c-d`  is left up to the Builder/platform
# as there is no direct convention at this time that an application/variant may not contain
# a `-`. So `a-b-c-d` may for example mean any of:
#   - platform 'a', application 'b', modifiers 'c' and 'd'
#   - platform 'a', board 'b', application 'c-d'
#   - platform 'a', application 'b', modifier 'c-d'
#   - platform 'a', application 'b-c', modifier 'd'
#
# The only requirement of the build system is that a single string corresponds to a single
# combination of platform/board/app/modifier(s). This requirement is unenfornced in code
# but easy enough to follow when defining names for things: just don't reuse names between '-'

import itertools
import logging
import os
import re
from dataclasses import dataclass
from typing import Any, Dict, Iterable, List, Optional

from builders.builder import BuilderOptions

report_rejected_parts = True


@dataclass(init=False)
class TargetPart:
    # SubTarget/Modifier name
    name: str

    # The build arguments to apply to a builder if this part is active
    build_arguments: Dict[str, Any]

    # Part should be included if and only if the final string MATCHES the
    # given regular expression
    only_if_re: Optional[re.Pattern] = None

    # Part should be included if and only if the final string DOES NOT match
    # given regular expression
    except_if_re: Optional[re.Pattern] = None

    def __init__(self, name, **kargs):
        self.name = name.lower()
        self.build_arguments = kargs

    def OnlyIfRe(self, expr: str):
        self.only_if_re = re.compile(expr)
        return self

    def ExceptIfRe(self, expr: str):
        self.except_if_re = re.compile(expr)
        return self

    def Accept(self, full_input: str):
        if self.except_if_re:
            if self.except_if_re.search(full_input):
                if report_rejected_parts:
                    # likely nothing will match when we get such an error
                    logging.error(f"'{self.name}' does not support '{full_input}' due to rule EXCEPT IF '{self.except_if_re.pattern}'")
                return False

        if self.only_if_re:
            if not self.only_if_re.search(full_input):
                if report_rejected_parts:
                    # likely nothing will match when we get such an error
                    logging.error(f"'{self.name}' does not support '{full_input}' due to rule ONLY IF '{self.only_if_re.pattern}'")
                return False

        return True

    def ToDict(self):
        """Converts a TargetPart into a dictionary
        """

        result: Dict[str, str] = {}
        result['name'] = self.name

        build_arguments: Dict[str, str] = {}
        for key, value in self.build_arguments.items():
            build_arguments[key] = str(value)

        result['build_arguments'] = build_arguments

        if self.only_if_re is not None:
            result['only_if_re'] = str(self.only_if_re.pattern)

        if self.except_if_re is not None:
            result['except_if_re'] = str(self.except_if_re.pattern)

        return result


def _HasVariantPrefix(value: str, prefix: str):
    """Checks if the given value is <prefix> or starts with "<prefix>-".

    This is useful when considering '-'-delimited strings, where a specific
    prefix may either be the last element in the list of items or some first element
    out of several.

    Returns:
        None if no match or the remaining value if there is a match.

    Examples:
        _HasVariantPrefix('foo', 'foo')         # -> ''
        _HasVariantPrefix('foo', 'bar')         # -> None
        _HasVariantPrefix('foo-bar', 'foo')     # -> 'bar'
        _HasVariantPrefix('foo-bar', 'bar')     # -> None
        _HasVariantPrefix('foo-bar-baz', 'foo') # -> 'bar-baz'
        _HasVariantPrefix('foo-bar-baz', 'bar') # -> None
    """
    if value == prefix:
        return ''

    if value.startswith(prefix + '-'):
        return value[len(prefix)+1:]


def _StringIntoParts(full_input: str, remaining_input: str, fixed_targets: List[List[TargetPart]], modifiers: List[TargetPart]):
    """Given an input string, process through all the input rules and return
       the underlying list of target parts for the input.

       Parameters:
          full_input: the full input string, used for validity matching (except/only_if)
          remaining_input: the remaining input to parse
          fixed_targets: the remaining fixed targets left to match
          modifiers: the modifiers left to match
    """
    if not remaining_input:
        if fixed_targets:
            # String was not fully matched. Fixed thargets are required
            return None

        # Fully parsed
        return []

    if fixed_targets:
        # If fixed targets remain, we MUST match one of them
        for target in fixed_targets[0]:
            suffix = _HasVariantPrefix(remaining_input, target.name)
            if suffix is None:
                continue

            # see if match should be rejected. Done AFTER variant prefix detection so we
            # can log if there are issues
            if not target.Accept(full_input):
                continue

            result = _StringIntoParts(full_input, suffix, fixed_targets[1:], modifiers)
            if result is not None:
                return [target] + result

        # None of the variants matched
        return None

    # Only modifiers left to process
    # Process the modifiers one by one
    for modifier in modifiers:
        suffix = _HasVariantPrefix(remaining_input, modifier.name)
        if suffix is None:
            continue

        # see if match should be rejected. Done AFTER variant prefix detection so we
        # can log if there are issues
        if not modifier.Accept(full_input):
            continue

        result = _StringIntoParts(full_input, suffix, fixed_targets[1:], [x for x in modifiers if x != modifier])
        if result is not None:
            return [modifier] + result

    # Remaining input is not empty and we failed to match it
    return None


class BuildTarget:

    def __init__(self, name, builder_class, **kwargs):
        """ Sets up a new build tareget starting with the given builder class
            and initial arguments
        """
        self.name = name.lower()
        self.builder_class = builder_class
        self.create_kw_args = kwargs

        # a list of sub_targets for this builder
        # sub-targets MUST be selected in some way. For example for esp32, we may
        # have a format of esp32-{devkitc, m5stack}-{light,lock}:
        #   - esp32-m5stack-lock is OK
        #   - esp32-devkitc-light is OK
        #   - esp32-light is NOT ok
        #   - esp32-m5stack is NOT ok
        self.fixed_targets: List[List[TargetPart]] = []

        # a list of all available modifiers for this build target
        # Modifiers can be combined in any way
        self.modifiers: List[TargetPart] = []

    def AppendFixedTargets(self, parts: List[TargetPart]):
        """Append a list of potential targets/variants.

        Example:

            target = BuildTarget('linux', LinuxBuilder)
            target.AppendFixedTargets([
                TargetPart(name='x64', board=HostBoard.X64),
                TargetPart(name='x86', board=HostBoard.X86),
                TargetPart(name='arm64', board=HostBoard.ARM64),
            ])

            target.AppendFixedTargets([
                TargetPart(name='light', app=HostApp.LIGHT),
                TargetPart(name='lock', app=HostApp.LIGHT).ExceptIfRe("-arm64-"),
                TargetPart(name='shell', app=HostApp.LIGHT).OnlyIfRe("-(x64|arm64)-"),
            ])

        The above will accept:
           linux-x64-light
           linux-x86-light
           linux-arm64-light
           linux-x64-lock
           linux-x86-lock
           linux-x64-shell
           linux-arm64-shell
        """
        self.fixed_targets.append(parts)

    def AppendModifier(self, name: str, **kargs):
        """Appends a specific modifier to a build target. For example:

        target.AppendModifier(name='release', release=True)
        target.AppendModifier(name='clang', use_clang=True)
        target.AppendModifier(name='coverage', coverage=True).OnlyIfRe('-clang')

        """
        part = TargetPart(name, **kargs)

        self.modifiers.append(part)

        return part

    def HumanString(self):
        """Prints out the human-readable string of the available variants and modifiers:

           like:

           foo-{bar,baz}[-modifier1][modifier2][modifier3]
           foo-bar-{a,b,c}[-m1][-m2]
        """
        result = self.name
        for fixed in self.fixed_targets:
            if len(fixed) > 1:
                result += '-{' + ",".join(map(lambda x: x.name, fixed)) + '}'
            else:
                result += '-' + fixed[0].name

        for modifier in self.modifiers:
            result += f"[-{modifier.name}]"

        return result

    def ToDict(self):
        """Outputs a parseable description of the available variants
        and modifiers:

            like:

            {
                "name": "foo"
                "shorthand": "foo-bar-baz[-m1]"
                "parts": [
                    {
                        "name": "foo",
                        "build_arguments": {
                            "board": "bar"
                        }
                    }
                    {
                        "name": "baz",
                        "build_arguments": {
                            "app": "foo.baz"
                        }
                    }
                ],
                "modifiers": [
                    {
                        "name": "modifier1",
                        "m1": "True"
                    }
                ]
            }
        """
        return {
            'name': self.name,
            'shorthand': self.HumanString(),
            'parts': [[part.ToDict() for part in target] for target in self.fixed_targets],
            'modifiers': [part.ToDict() for part in self.modifiers]
        }

    def AllVariants(self) -> Iterable[str]:
        """Returns all possible accepted variants by this target.

           For example name-{a,b}-{c,d}[-1][-2]  could return (there may be Only/ExceptIfRe rules):

              name-a-c
              name-a-c-1
              name-a-c-2
              name-a-c-1-2
              name-a-d
              name-a-d-1
              ...
              name-b-d-2
              name-b-d-1-2

           Notice that this DOES increase exponentially and is potentially a very long list
        """

        # Output is made out of 2 separate parts:
        #   - a valid combination of "fixed parts"
        #   - a combination of modifiers

        fixed_indices = [0]*len(self.fixed_targets)

        while True:

            prefix = "-".join(map(
                lambda p: self.fixed_targets[p[0]][p[1]].name, enumerate(fixed_indices)
            ))

            for n in range(len(self.modifiers) + 1):
                for c in itertools.combinations(self.modifiers, n):
                    suffix = ""
                    for m in c:
                        suffix += "-" + m.name
                    option = f"{self.name}-{prefix}{suffix}"

                    if self.StringIntoTargetParts(option) is not None:
                        yield option

            # Move to the next index in fixed_indices or exit loop if we cannot move
            move_idx = len(fixed_indices) - 1
            while move_idx >= 0:
                if fixed_indices[move_idx] + 1 < len(self.fixed_targets[move_idx]):
                    fixed_indices[move_idx] += 1
                    break

                # need to move the previous value
                fixed_indices[move_idx] = 0
                move_idx -= 1

            if move_idx < 0:
                # done iterating through all
                return

    def StringIntoTargetParts(self, value: str):
        """Given an input string, process through all the input rules and return
           the underlying list of target parts for the input.
        """
        suffix = _HasVariantPrefix(value, self.name)
        if not suffix:
            return None

        return _StringIntoParts(value, suffix, self.fixed_targets, self.modifiers)

    def Create(self, name: str, runner, repository_path: str, output_prefix: str,
               verbose: bool, ninja_jobs: int, builder_options: BuilderOptions):

        parts = self.StringIntoTargetParts(name)

        if not parts:
            return None

        kargs = {}
        for part in parts:
            kargs.update(part.build_arguments)

        logging.info("Preparing builder '%s'" % (name,))

        builder = self.builder_class(repository_path, runner=runner, **kargs)
        builder.target = self
        builder.identifier = name
        builder.output_dir = os.path.join(output_prefix, name)
        builder.verbose = verbose
        builder.ninja_jobs = ninja_jobs
        builder.chip_dir = os.path.abspath(repository_path)
        builder.options = builder_options

        return builder
