#!/usr/bin/env python3

# Copyright (c) 2022 Project CHIP Authors
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

import os
import sys
import unittest
from difflib import unified_diff
from typing import List, Optional

try:
    from matter_idl.matter_idl_parser import CreateParser
except ImportError:

    sys.path.append(os.path.abspath(
        os.path.join(os.path.dirname(__file__), '..')))
    from matter_idl.matter_idl_parser import CreateParser

from matter_idl.generators import GeneratorStorage
from matter_idl.generators.idl import IdlGenerator
from matter_idl.matter_idl_types import Idl


class TestCaseStorage(GeneratorStorage):
    def __init__(self):
        super().__init__()
        self.content: Optional[str] = None

    def get_existing_data(self, relative_path: str):
        # Force re-generation each time
        return None

    def write_new_data(self, relative_path: str, content: str):
        if self.content:
            raise Exception(
                "Unexpected extra data: single file generation expected")
        self.content = content


def ReadMatterIdl(repo_path: str) -> str:
    path = os.path.join(os.path.dirname(__file__), "../../..", repo_path)
    with open(path, "rt") as stream:
        return stream.read()


def ParseMatterIdl(repo_path: str, skip_meta: bool, merge_globals: bool) -> Idl:
    return CreateParser(skip_meta=skip_meta, merge_globals=merge_globals).parse(ReadMatterIdl(repo_path))


def RenderAsIdlTxt(idl: Idl) -> str:
    storage = TestCaseStorage()
    IdlGenerator(storage=storage, idl=idl).render(dry_run=False)
    return storage.content or ""


def SkipLeadingComments(txt: str, also_strip: List[str] = list()) -> str:
    """Skips leading lines starting with // in a file. """
    lines = txt.split("\n")
    idx = 0
    while lines[idx].startswith("//") or not lines[idx]:
        idx = idx + 1

    result = "\n".join(lines[idx:])

    for s in also_strip:
        result = result.replace(s, "")

    return result


class TestIdlRendering(unittest.TestCase):
    def assertTextEqual(self, a: str, b: str):
        if a == b:
            # seems the same. This will just pass
            self.assertEqual(a, b)
            return

        delta = unified_diff(a.splitlines(keepends=True),
                             b.splitlines(keepends=True),
                             fromfile='actual.matter',
                             tofile='expected.matter',
                             )
        self.assertEqual(a, b, '\n' + ''.join(delta))

    def test_client_clusters(self):
        # IDL renderer was updated to have IDENTICAL output for client side
        # cluster rendering, so this diff will be verbatim
        #
        # Comparison made text-mode so that meta-data is read and doc-comments are
        # available

        path = "src/controller/data_model/controller-clusters.matter"

        # Files MUST be identical except the header comments which are different
        original = SkipLeadingComments(ReadMatterIdl(path), also_strip=[
                                       " // NOTE: Default/not specifically set"])
        # Do not merge globals because ZAP generated matter files do not contain
        # the merge global data (will not render global reference comments).
        generated = SkipLeadingComments(RenderAsIdlTxt(
            ParseMatterIdl(path, skip_meta=False, merge_globals=False)))

        self.assertTextEqual(original, generated)

    def test_app_rendering(self):
        # When endpoints are involved, default value formatting is lost
        # (e.g. "0x0000" becomes "0") and ordering of emitted events is not preserved
        # because the events are a

        # as such, this test validates that parsing + generating + re-parsing results
        # in the same data being parsed
        test_paths = [
            "examples/lock-app/lock-common/lock-app.matter",
            "examples/lighting-app/lighting-common/lighting-app.matter",
            "examples/all-clusters-app/all-clusters-common/all-clusters-app.matter",
            "examples/thermostat/thermostat-common/thermostat.matter",
        ]

        for path in test_paths:
            idl = ParseMatterIdl(path, skip_meta=True, merge_globals=True)
            txt = RenderAsIdlTxt(idl)
            idl2 = CreateParser(skip_meta=True, merge_globals=True).parse(txt)

            # checks that data types and content is the same
            self.assertEqual(idl, idl2)


if __name__ == '__main__':
    unittest.main()
