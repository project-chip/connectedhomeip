#!/usr/bin/python3
# Dummy Password Manager for Tizen Studio CLI
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

import argparse
import fcntl
import os
import pickle
import sys


class Secrets:

    def __init__(self, filename: str):
        self.filename = filename
        self.dirty = False
        self.secrets = {}
        self.fp = None

    def __enter__(self):
        self.fp = open(self.filename, "a+b")
        fcntl.flock(self.fp.fileno(), fcntl.LOCK_EX)
        self._load()
        return self

    def __exit__(self, _type, value, tb):
        if self.dirty:
            self._save()
        fcntl.flock(self.fp.fileno(), fcntl.LOCK_UN)
        self.fp.close()

    @staticmethod
    def _build_key(label: str, **kw):
        return label + ":" + str(tuple(sorted(kw.items())))

    def _load(self):
        try:
            self.fp.seek(0)
            self.secrets = pickle.load(self.fp)
        except EOFError:
            # Unpickling an empty file is not an error for us
            pass
        except ValueError as e:
            print("ERROR: " + str(e), file=sys.stderr)

    def _save(self):
        try:
            self.fp.seek(0)
            self.fp.truncate()
            pickle.dump(self.secrets, self.fp)
            self.fp.flush()
        except IOError as e:
            print("ERROR: " + str(e), file=sys.stderr)

    def clear(self, label: str, **kw):
        key = self._build_key(label, **kw)
        self.secrets.pop(key, None)
        self.dirty = True

    def store(self, label: str, password: str, **kw):
        key = self._build_key(label, **kw)
        self.secrets[key] = password
        self.dirty = True

    def lookup(self, label: str, **kw):
        key = self._build_key(label, **kw)
        return self.secrets.get(key, "")


parser = argparse.ArgumentParser(
    formatter_class=argparse.RawDescriptionHelpFormatter,
    description="""
Dummy Password Manager for Tizen Studio CLI.

This simple password manager circumvents the requirement of having functional
D-Bus Secrets service (org.freedesktop.secrets) in the Docker container. As a
storage this manager uses plain-text file with pickled data (~/.secretsdb).

Please, DO NOT store real secrets in it!""")
subparsers = parser.add_subparsers(dest='command', required=True)

parser_clear = subparsers.add_parser(
    "clear", help="Remove password associated with given key value pairs")
parser_clear.add_argument("-l", "--label", action='store', required=True,
                          help="label for given key value pairs")
parser_clear.add_argument("kw", nargs='*',
                          help="key value pairs")

parser_store = subparsers.add_parser(
    "store", help="Store password for given key value pairs")
parser_store.add_argument("-l", "--label", action='store', required=True,
                          help="label for given key value pairs")
parser_store.add_argument("-p", "--password", action='store', required=True,
                          help="password for given key value pairs")
parser_store.add_argument("kw", nargs='*',
                          help="key value pairs")

parser_lookup = subparsers.add_parser(
    "lookup", help="Retrieve password associated with given key value pairs")
parser_lookup.add_argument("-l", "--label", action='store', required=True,
                           help="label for given key value pairs")
parser_lookup.add_argument("kw", nargs='*',
                           help="key value pairs")

args = parser.parse_args()
kw = dict(zip(args.kw[:: 2], args.kw[1:: 2]))

with Secrets(os.path.expanduser("~/.secretsdb")) as secrets:
    if args.command == "clear":
        secrets.clear(args.label, **kw)
    elif args.command == "store":
        secrets.store(args.label, args.password, **kw)
    elif args.command == "lookup":
        password = secrets.lookup(args.label, **kw)
        print(password)
