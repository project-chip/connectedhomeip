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


class ProjectArgProcessor:
    # Prefixes to try when mapping parameters to GN arguments
    BOOL_ARG_PREFIXES = ('is_', 'enable_', 'use_',
                         'chip_', 'chip_enable_', 'chip_use_', 'chip_config_',
                         'matter_', 'matter_enable_', 'matter_use_', 'matter_config_')
    GENERIC_ARG_PREFIXES = ('chip_', 'chip_config_', 'matter_', 'matter_config_')

    gn_args = {}  # GN arg -> type ('s'tring, 'b'ool, 'i'integer, '[' list, '{' struct)
    args = collections.OrderedDict()  # collected arguments

    def __init__(self, gn_args_json_file):
        # Parse `gn args --list --json` output and derive arg types from default values
        argtype = str.maketrans('"tf0123456789', 'sbbiiiiiiiiii')
        with open(gn_args_json_file) as fh:
            for arg in json.load(fh):
                self.gn_args[arg['name']] = arg['default']['value'][0].translate(argtype)

    def process_env(self):
        self.add_env_arg('chip_code_pre_generated_directory', 'CHIP_PREGEN_DIR')

        self.add_default('custom_toolchain', 'custom')
        self.add_env_arg('target_cc', 'CC', 'cc')
        self.add_env_arg('target_cxx', 'CXX', 'cxx')
        self.add_env_arg('target_ar', 'AR', 'ar')
        self.add_env_arg('target_cflags', 'CPPFLAGS', list=True)
        self.add_env_arg('target_cflags_c', 'CFLAGS', list=True)
        self.add_env_arg('target_cflags_cc', 'CXXFLAGS', list=True)
        self.add_env_arg('target_cflags_objc', 'OBJCFLAGS', list=True)
        self.add_env_arg('target_ldflags', 'LDFLAGS', list=True)

    def add_arg(self, arg, value):
        # format strings and booleans as JSON, otherwise pass through as is
        self.args[arg] = (json.dumps(value) if self.gn_args.get(arg, 's') in 'sb' else value)

    def add_default(self, arg, value):
        """Add an argument, if supported by the GN build"""
        if arg in self.gn_args:
            self.add_arg(arg, value)

    def add_env_arg(self, arg, envvar, default=None, list=False):
        """Add an argument from an environment variable"""
        value = os.environ.get(envvar, default)
        if not value:
            return
        type = self.gn_args.get(arg, None)
        if not type:
            info("Warning: Not propagating %s, project has no build arg '%s'" % (envvar, arg))
            return
        self.args[arg] = json.dumps(value if type != '[' else value.split() if list else [value])

    def gn_arg(self, name, prefixes=(), type=None):
        """Finds the GN argument corresponding to a parameter name"""
        arg = name.translate(str.maketrans('-', '_'))
        candidates = [p + arg for p in (('',) + prefixes) if (p + arg) in self.gn_args]
        preferred = [c for c in candidates if self.gn_args[c] == type] if type else []
        match = next(iter(preferred + candidates), None)
        if not match:
            info("Warning: Project has no build arg '%s'" % arg)
        return match

    def process_triplet_parameter(self, name, value):
        if value is None:
            fail("Project option --%s requires an argument" % name)
        triplet = value.split('-')
        if len(triplet) not in (2, 3, 4):
            fail("Project option --%s argument must be a cpu-vendor-os[-abi] triplet" % name)
        prefix = 'host_' if name == 'build' else 'target_'  # "host" has different meanings in GNU and GN!
        self.add_arg(prefix + 'cpu', triplet[0])
        self.add_arg(prefix + 'os', triplet[1 if len(triplet) == 2 else 2])

    def process_enable_parameter(self, name, value):
        arg = self.gn_arg(name[len('enable-'):], self.BOOL_ARG_PREFIXES, 'b')
        if not arg:
            return
        if self.gn_args[arg] != 'b':
            fail("Project build arg '%s' is not a boolean" % arg)
        if value != 'no' and value is not None:
            fail("Invalid argument for --%s, must be absent or 'no'" % name)
        self.add_arg(arg, value is None)

    def process_generic_parameter(self, name, value):
        arg = self.gn_arg(name, self.GENERIC_ARG_PREFIXES)
        if not arg:
            return
        if self.gn_args[arg] == 'b':
            fail("Project build arg '%s' is a boolean, use --enable-..." % arg)
        if value is None:
            fail("Project option --%s requires an argument" % name)
        self.add_arg(arg, value)

    def process_parameter(self, name, value):
        if name in ('build', 'host', 'target'):
            self.process_triplet_parameter(name, value)
        elif name.startswith('enable-'):
            self.process_enable_parameter(name, value)
        else:
            self.process_generic_parameter(name, value)

    def process_parameters(self, args):
        """Process GNU-style configure command line parameters"""
        for arg in args:
            match = re.fullmatch(r'--([a-z][a-z0-9-]*)(?:=(.*))?', arg)
            if not match:
                fail("Invalid argument: '%s'" % arg)
            self.process_parameter(match.group(1), match.group(2))


def info(message):
    print(message, file=sys.stderr)


def fail(message):
    info("Error: " + message)
    sys.exit(1)


# `configure` invokes the top-level functions in this file by
# passing the function name and arguments on the command line.
[_, func, *args] = sys.argv
globals()[func](*args)
