#!/usr/bin/python3
# Dummy Password Manager for Tizen Studio CLI

import os
import pickle
import sys
from argparse import ArgumentParser


class Secrets:

    def __init__(self, filename: str):
        self.filename = filename
        self.secrets = {}

    @staticmethod
    def _build_key(label: str, **kw):
        return label + ":" + str(tuple(sorted(kw.items())))

    def load(self):
        try:
            with open(self.filename, "rb") as f:
                self.secrets = pickle.load(f)
        except (IOError, ValueError) as e:
            print("ERROR: " + str(e), file=sys.stderr)

    def save(self):
        try:
            with open(self.filename, "wb") as f:
                pickle.dump(self.secrets, f)
        except IOError as e:
            print("ERROR: " + str(e), file=sys.stderr)

    def clear(self, label: str, **kw):
        key = self._build_key(label, **kw)
        self.secrets.pop(key, None)

    def store(self, label: str, password: str, **kw):
        key = self._build_key(label, **kw)
        self.secrets[key] = password

    def lookup(self, label: str, **kw):
        key = self._build_key(label, **kw)
        return self.secrets.get(key, "")


parser = ArgumentParser()
subparsers = parser.add_subparsers(dest='command', required=True)

parser_clear = subparsers.add_parser("clear")
parser_clear.add_argument("-l", "--label", action='store', required=True)
parser_clear.add_argument("kw", nargs='*')

parser_store = subparsers.add_parser("store")
parser_store.add_argument("-l", "--label", action='store', required=True)
parser_store.add_argument("-p", "--password", action='store', required=True)
parser_store.add_argument("kw", nargs='*')

parser_lookup = subparsers.add_parser("lookup")
parser_lookup.add_argument("-l", "--label", action='store', required=True)
parser_lookup.add_argument("kw", nargs='*')

args = parser.parse_args()
kw = dict(zip(args.kw[:: 2], args.kw[1:: 2]))

secrets = Secrets(os.path.expanduser("~/.secretsdb"))
secrets.load()

if args.command == "clear":
    secrets.clear(args.label, **kw)
    secrets.save()
elif args.command == "store":
    secrets.store(args.label, args.password, **kw)
    secrets.save()
elif args.command == "lookup":
    password = secrets.lookup(args.label, **kw)
    print(password)
