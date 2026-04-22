# Copyright (c) 2023 Project CHIP Authors
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

# This file contains private utilities for use by the `configure` script.
# It is self-contained and depends only on the Python 3 standard library.
#
# Some unit tests that can be run manually are in configure.impl.test.py.

import collections
import json
import os
import re
import sys
import urllib.request
import zipfile


def download(url, dest):
    urllib.request.urlretrieve(url, dest)


def download_and_extract_zip(url, dest_dir, *member_names):
    file, *_ = urllib.request.urlretrieve(url)
    with zipfile.ZipFile(file) as zip:
        for member in zip.infolist():
            if member.filename in member_names:
                zip.extract(member, dest_dir)


def process_project_args(gn_args_json_file, *params):
    processor = ProjectArgProcessor(gn_args_json_file)
    processor.process_env()
    processor.process_parameters(params)
    for arg, value in processor.args.items():
        info(" - %s = %s" % (arg, value))
        print("%s = %s" % (arg, value))


def describe_args(gn_args_json_file):
    processor = ProjectArgProcessor(gn_args_json_file)

    # Header / Built-ins handled by process_triplet_parameter
    print("Available configure options and default values:\n"
          "\n"
          "### Target Options\n"
          "\n"
          "--build=...\n"
          "  # host_cpu, host_os: GN built-in\n"
          "  # The system on which the software is being built, as cpu-os or cpu-vendor-os[-abi]\n"
          "\n"
          "--host=...\n"
          "--target=...\n"
          "  # target_cpu, target_os: GN built-in\n"
          "  # The system on which the software will run, as cpu-os or cpu-vendor-os[-abi]\n"
          )

    def describe_group(group, heading):
        if not group:
            return
        print(f"### {heading}\n")
        for arg in sorted(group, key=lambda a: a.alias):
            if arg.type == 'b':
                print(f"--enable-{arg.alias}{'=no' if arg.default == 'false' else ''}")
            else:
                # Abbreviate long list / structs
                default = arg.default if len(arg.default) <= 40 \
                    else '[...]' if arg.type == '[' \
                    else '{...}' if arg.type == '{' \
                    else arg.default
                print(f"--{arg.alias}={default}")
            if arg.source:
                print(f"  # {arg.name}: {arg.source}")
            if arg.comment:
                print("  #", arg.comment.replace("\n", "\n  # "))
            print()

    project, sdk, third_party, overrides = [], [], [], []
    for arg in processor.gn_args.values():
        if (source := arg.source) and arg.alias:
            group = third_party if source.startswith('//third_party/connectedhomeip/third_party/') \
                else sdk if source.startswith('//third_party/connectedhomeip/') \
                else third_party if source.startswith('//third_party/') \
                else overrides if source.startswith('//build_overrides/') \
                else project
            group.append(arg)

    describe_group(project, "Project Options")
    describe_group(sdk, "Matter SDK Options")
    describe_group(overrides, "Build Override Paths")
    describe_group(third_party, "Third-Party Options")


class ProjectArgProcessor:

    class GnArg:
        # Prefixes for mapping between configure parameters and GN arguments (longer prefixes first)
        BOOL_ARG_PREFIXES = ('is_', 'enable_', 'use_',
                             'chip_enable_', 'chip_use_', 'chip_config_', 'chip_',
                             'matter_enable_', 'matter_use_', 'matter_config_', 'matter_', '')
        GENERIC_ARG_PREFIXES = ('chip_config_', 'chip_', 'matter_config_', 'matter_', '')

        ARG_TYPE_TRANS = str.maketrans('"tf0123456789-', 'sbbiiiiiiiiiii')
        DASH_TRANS = str.maketrans('_', '-')

        def __init__(self, data):
            self.name = str(data['name'])
            self.alias = None  # will be set below
            self.default = str(data['default']['value'])
            self.type = self.default[0].translate(self.ARG_TYPE_TRANS)  # 's'tring, 'b'ool, 'i'integer, '[' list, '{' struct
            file = data['default'].get('file', None)
            self.source = '%s:%d' % (file, data['default']['line']) if file else None
            self.comment = str(data.get('comment', '')).strip()

        def alias_candidates(self):
            prefixes = self.BOOL_ARG_PREFIXES if self.type == 'b' else self.GENERIC_ARG_PREFIXES
            return (self.name.removeprefix(p).lower().translate(self.DASH_TRANS) for p in prefixes if self.name.startswith(p))

        def __str__(self):
            return self.name

    args: collections.OrderedDict  # collected arguments
    gn_args: dict[str, GnArg]  # by GN name
    options: dict[str, GnArg]  # by all unique option aliases

    def __init__(self, gn_args_json_file):
        self.args = collections.OrderedDict()

        # Parse `gn args --list --json` output into gn_args
        with open(gn_args_json_file) as fh:
            self.gn_args = {arg.name: arg for arg in (self.GnArg(rec) for rec in json.load(fh))}

        # Count collisions first then index options by all unique aliases
        counts = collections.defaultdict(int)
        for arg in self.gn_args.values():
            for alias in arg.alias_candidates():
                counts[alias] += 1
        self.options = {}
        for arg in self.gn_args.values():
            for alias in arg.alias_candidates():
                if counts[alias] <= 1:
                    self.options[alias] = arg
                    arg.alias = arg.alias or alias  # canonical (shortest unique) alias

    def process_env(self):
        self.add_env_arg('chip_code_pre_generated_directory', 'CHIP_PREGEN_DIR')

        self.add_arg('custom_toolchain', 'custom', required=False)
        self.add_env_arg('target_cc', 'CC', 'cc')
        self.add_env_arg('target_cxx', 'CXX', 'cxx')
        self.add_env_arg('target_ar', 'AR', 'ar')
        self.add_env_arg('target_cflags', 'CPPFLAGS', list=True)
        self.add_env_arg('target_cflags_c', 'CFLAGS', list=True)
        self.add_env_arg('target_cflags_cc', 'CXXFLAGS', list=True)
        self.add_env_arg('target_cflags_objc', 'OBJCFLAGS', list=True)
        self.add_env_arg('target_ldflags', 'LDFLAGS', list=True)

    def add_arg(self, nameOrArg, value, required=True):
        if arg := self.gn_args.get(nameOrArg, None) if isinstance(nameOrArg, str) else nameOrArg:
            # format strings and booleans as JSON, otherwise pass through as is
            self.args[arg.name] = (json.dumps(value) if arg.type in 'sb' else value)
        elif required:
            fail("Project has no build arg '%s'" % nameOrArg)

    def add_env_arg(self, name, envvar, default=None, list=False):
        """Add an argument from an environment variable"""
        if not (value := os.environ.get(envvar, default)):
            return
        if not (arg := self.gn_args.get(name, None)):
            info("Warning: Not propagating %s, project has no build arg '%s'" % (envvar, name))
            return
        # bypass add_arg() to handle list splitting / formatting directly
        self.args[arg.name] = json.dumps(value if arg.type != '[' else value.split() if list else [value])

    def process_triplet_parameter(self, name, value):
        if value is None:
            fail("Project option --%s requires an argument" % name)
        triplet = value.split('-')
        if len(triplet) not in (2, 3, 4):
            fail("Project option --%s argument must be a cpu-os pair or cpu-vendor-os[-abi] triplet" % name)
        prefix = 'host_' if name == 'build' else 'target_'  # "host" has different meanings in GNU and GN!
        self.add_arg(prefix + 'cpu', triplet[0])
        self.add_arg(prefix + 'os', triplet[1 if len(triplet) == 2 else 2])

    def option(self, option):
        """Returns the GnArg for the given option name, or None"""
        if not (arg := self.options.get(option, None)):
            info("Warning: Project has no build option '%s'" % option)
        return arg

    def process_enable_parameter(self, option, value):
        if not (arg := self.option(option.removeprefix('enable-'))):
            return
        if arg.type != 'b':
            fail("Project build arg '%s' is not a boolean" % arg)
        if value != 'no' and value is not None:
            fail("Invalid argument for --%s, must be absent or 'no'" % option)
        self.add_arg(arg, value is None)

    def process_generic_parameter(self, option, value):
        if not (arg := self.option(option)):
            return
        if arg.type == 'b':
            fail("Project build arg '%s' is a boolean, use --enable-..." % arg)
        if value is None:
            fail("Project option --%s requires an argument" % option)
        self.add_arg(arg, value)

    def process_parameter(self, option, value):
        if option in ('build', 'host', 'target'):
            self.process_triplet_parameter(option, value)
        elif option.startswith('enable-'):
            self.process_enable_parameter(option, value)
        else:
            self.process_generic_parameter(option, value)

    def process_parameters(self, params):
        """Process GNU-style configure command line parameters"""
        for param in params:
            match = re.fullmatch(r'--([a-z][a-z0-9-]*)(?:=(.*))?', param)
            if not match:
                fail("Invalid argument: '%s'" % param)
            self.process_parameter(match.group(1), match.group(2))


def info(message):
    print(message, file=sys.stderr)


def fail(message):
    info("Error: " + message)
    sys.exit(1)


if __name__ == "__main__":
    # `configure` invokes the top-level functions in this file by
    # passing the function name and arguments on the command line.
    [_, func, *args] = sys.argv
    globals()[func](*args)
