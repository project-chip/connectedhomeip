#
# Copyright (c) 2021 Project CHIP Authors
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
"""Configuration utilities for MDF tools"""

import argparse
import ast
import logging
import re
from typing import Any, Dict, List, Mapping, MutableMapping, Optional, Pattern, Sequence, Tuple, Union

import humanfriendly  # type: ignore
import memdf.util.nd as nd
import memdf.util.pretty

# A ConfigDescription is a declarative description of configuration options.
#
# In a description dictionary, (most) keys are configuration keys
# and values are dictionaries that MUST contain at least
#   'help':    help string.
#   'default': default value.
# and may contain:
#   'metavar':  if the command line argument takes a value
#   'choices':  if the argument value must be one of several specific values
#   'argparse': additional argument parsing information; most of this is
#               supplied as keyword arguments to `argparse.add_argument()`,
#               except for:
#                   'alias': list of alternate command line option names
#   'postprocess': a callable invoked after argument parsing with three
#               arguments: the config, the key, and the description entry.
#
# Special keys can be used to control argument parser groups. By default any
# configuration key containing a ‘.’ belongs to a group determined by the
# key prefix (the part before the first ‘.’).
#   Config.group_def(group):
#               the value is supplied as keyword arguments to
#               `argparse.add_argument_group()`
#   Config.group_map(prefix):
#               the value contains a key 'group', whose value is the group
#               to be used for configuration keys with the given prefix.
#
ConfigDescription = Mapping[Union[str, Tuple[int, str]], Mapping[str, Any]]


class Config:
    """Key/value store and argument parsing.

    A configuration key is a string where dots (`.`) separate levels in the
    underlying nested dictionary.

    For functions that take a Config, an empty `Config()` is normally
    acceptable. These functions should always assume reasonable defaults,
    so that they can be used without any particular configuration.
    """

    def __init__(self):
        self.d: MutableMapping = {}
        self.argparse = None
        self.argparse_groups = {}
        self.group_alias = {}
        self.postprocess_args = {}
        self.config_desc: ConfigDescription = None
        self.dest_to_key: MutableMapping = {}
        self.key_to_dest: MutableMapping = {}

    # Basic config access

    def get(self, key: str, default: Any = None) -> Any:
        return self.getl(key.split('.'), default)

    def __getitem__(self, key: str) -> Any:
        """[] syntax for configuration.

        Note that this will return `None` for an unknown key, since the
        absence of a configured value is not considered an error.
        """
        return self.get(key)

    def getl(self, keys: nd.Key, default: Any = None) -> Any:
        return nd.get(self.d, keys, default)

    def put(self, key: str, value: Any) -> None:
        self.putl(key.split('.'), value)

    def __setitem__(self, key: str, value: Any) -> None:
        self.put(key, value)

    def putl(self, keys: nd.Key, value: Any) -> None:
        nd.put(self.d, keys, value)

    def update(self, src: Mapping) -> None:
        nd.update(self.d, src)

    # Command line and config file reading

    _GROUP_DEF = 1
    _GROUP_MAP = 2

    @staticmethod
    def group_def(s: str) -> Tuple[int, str]:
        return (Config._GROUP_DEF, s)

    @staticmethod
    def group_map(s: str) -> Tuple[int, str]:
        return (Config._GROUP_MAP, s)

    def init_config(self, desc: ConfigDescription) -> 'Config':
        """Initialize a configuration from a description dictionary.

        Note that this initializes only the key/value store,
        not anything associated with command line argument parsing.
        """
        self.config_desc = desc
        for key, info in desc.items():
            if isinstance(key, str):
                self.put(key, info['default'])

        return self

    def init_args(self, desc: ConfigDescription, *args, **kwargs) -> 'Config':
        """Initialize command line argument parsing."""
        self.argparse = argparse.ArgumentParser(*args, **kwargs)

        # Groups
        for key, info in desc.items():
            if not isinstance(key, tuple):
                continue
            kind, name = key
            if kind == self._GROUP_MAP:
                self.group_alias[name] = info['group']
            elif kind == self._GROUP_DEF:
                self.argparse_groups[name] = self.argparse.add_argument_group(
                    **info)

        # Arguments
        for key, info in desc.items():
            if not isinstance(key, str):
                continue
            if (arg_info := info.get('argparse', {})) is False:
                continue

            arg_info = arg_info.copy()
            name = arg_info.pop('argument', '--' + key.replace('.', '-'))
            names = [name] + arg_info.pop('alias', [])
            info['names'] = names
            for k in ['metavar', 'choices']:
                if k in info:
                    arg_info[k] = info[k]
            default = info['default']
            if not arg_info.get('action'):
                if isinstance(default, list):
                    arg_info['action'] = 'append'
                elif default is False:
                    arg_info['action'] = 'store_true'
                elif default is True:
                    arg_info['action'] = 'store_false'
                elif isinstance(default, int) and 'metavar' not in info:
                    arg_info['action'] = 'count'
            if postprocess := info.get('postprocess'):
                self.postprocess_args[key] = (postprocess, info)

            group: Optional[str] = info.get('group')
            if group is None and (e := key.find('.')) > 0:
                group = key[0:e]
            group = self.group_alias.get(group, group)
            arg_group = self.argparse_groups.get(group, self.argparse)
            arg = arg_group.add_argument(*names,
                                         help=info['help'],
                                         default=self.get(key, default),
                                         **arg_info)
            self.dest_to_key[arg.dest] = key
            self.key_to_dest[key] = arg.dest

        return self

    def init(self, desc: ConfigDescription, *args, **kwargs) -> 'Config':
        """Intialize configuration from a configuration description."""
        self.init_config(desc)
        self.init_args(desc, *args, **kwargs)
        return self

    def parse(self, argv: Sequence[str]) -> 'Config':
        """Parse command line options into a configuration dictionary."""

        # Read config file(s).
        config_parser = argparse.ArgumentParser(add_help=False,
                                                allow_abbrev=False)
        config_arg: Dict[str, Any] = {
            'metavar': 'FILE',
            'default': [],
            'action': 'append',
            'help': 'Read configuration FILE'
        }
        config_parser.add_argument('--config-file', **config_arg)
        self.argparse.add_argument('--config-file', **config_arg)
        config_args, argv = config_parser.parse_known_args(argv[1:])
        for filename in config_args.config_file:
            self.read_config_file(filename)

        # Update argparser defaults.
        defaults = {}
        for dest, key in self.dest_to_key.items():
            default = self.get(key)
            if default is not None:
                defaults[dest] = default
        self.argparse.set_defaults(**defaults)

        # Parse command line arguments and update config.
        args = self.argparse.parse_args(argv)
        for dest, value in vars(args).items():
            if (key := self.dest_to_key.get(dest)) is None:
                key = 'args.' + dest
            self.put(key, value)

        # Configure logging.
        if self.get('log-level') is None:
            verbose = self.get('verbose', 0)
            self.put('log-level',
                     (logging.DEBUG if verbose > 1 else
                      logging.INFO if verbose else logging.WARNING))
        else:
            self.put('log-level',
                     getattr(logging, self.get('log-level').upper()))
        logging.basicConfig(level=self.get('log-level'),
                            format=self.get('log-format'))

        # Postprocess config.
        for key, postprocess in self.postprocess_args.items():
            action, info = postprocess
            action(self, key, info)

        memdf.util.pretty.debug(self.d)
        return self

    def read_config_file(self, filename: str) -> 'Config':
        """Read a configuration file."""
        with open(filename, 'r') as fp:
            d = ast.literal_eval(fp.read())
            nd.update(self.d, d)
        return self

    @staticmethod
    def transpose_dictlist(src: Dict[str, List[str]]) -> Dict[str, str]:
        d: Dict[str, str] = {}
        for k, vlist in src.items():
            for v in vlist:
                d[v] = k
        return d

    def getl_re(self, key: nd.Key) -> Optional[Pattern]:
        """Get a cached compiled regular expression for a config value list."""
        regex_key: nd.Key = ['cache', 're'] + key
        regex: Optional[Pattern] = self.getl(regex_key)
        if not regex:
            branches: Optional[Sequence[str]] = self.getl(key)
            if branches:
                regex = re.compile('|'.join(branches))
            self.putl(regex_key, regex)
        return regex

    def get_re(self, key: str) -> Optional[Pattern]:
        return self.getl_re(key.split('.'))


# Argument parsing helpers


def parse_size(s: str) -> int:
    return humanfriendly.parse_size(s, binary=True) if s else 0


class ParseSizeAction(argparse.Action):
    """argparse helper for humanfriendly sizes"""

    def __call__(self, parser, namespace, values, option_string=None):
        setattr(namespace, self.dest, parse_size(values))


# Config description of options shared by all tools.
CONFIG: ConfigDescription = {
    'log-level': {
        'help':
            'Set logging level: one of critical, error, warning, info, debug.',
        'default': None,
        'choices': ['critical', 'error', 'warning', 'info', 'debug'],
    },
    'log-format': {
        'help': 'Set logging format',
        'metavar': 'FORMAT',
        'default': '%(message)s',
    },
    'verbose': {
        'help': 'Show informational messages; repeat for debugging messages',
        'default': 0,
        'argparse': {
            'alias': ['-v'],
            'action': 'count',
        },
    },
}
