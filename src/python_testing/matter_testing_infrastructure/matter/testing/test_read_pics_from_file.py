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

"""Unit tests for read_pics_from_file (XML directory layouts and zip archives)."""

import os
import tempfile
import unittest
import zipfile

from matter.testing.pics import read_pics_from_file

_MINIMAL_CLUSTER_PICS = """<?xml version='1.0' encoding='utf-8'?>
<clusterPICS xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance">
\t<usage>
{items}
\t</usage>
</clusterPICS>
"""

_PICS_ITEM = """\t\t<picsItem>
\t\t\t<itemNumber>{name}</itemNumber>
\t\t\t<support>{support}</support>
\t\t</picsItem>
"""


def _xml_with_items(*, items: list[tuple[str, str]]) -> str:
    body = "\n".join(_PICS_ITEM.format(name=n, support=s) for n, s in items)
    return _MINIMAL_CLUSTER_PICS.format(items=body)


class TestReadPicsFromFile(unittest.TestCase):
    def test_ci_text_file(self):
        with tempfile.NamedTemporaryFile(mode="w", suffix=".txt", delete=False) as f:
            f.write("EP0TEST.S=1\nEP0TEST.S.A0001=0\n")
            path = f.name
        try:
            pics = read_pics_from_file(path)
            self.assertTrue(pics["EP0TEST.S"])
            self.assertFalse(pics["EP0TEST.S.A0001"])
        finally:
            os.unlink(path)

    def test_flat_xml_directory(self):
        with tempfile.TemporaryDirectory() as d:
            with open(os.path.join(d, "a.xml"), "w", encoding="utf-8") as f:
                f.write(_xml_with_items(items=[("FLATONE.S", "true")]))
            with open(os.path.join(d, "b.xml"), "w", encoding="utf-8") as f:
                f.write(_xml_with_items(items=[("FLATTWO.S", "false")]))
            pics = read_pics_from_file(d)
            self.assertTrue(pics["FLATONE.S"])
            self.assertFalse(pics["FLATTWO.S"])

    def test_endpoint_subdirectories_merged(self):
        with tempfile.TemporaryDirectory() as d:
            ep0 = os.path.join(d, "0")
            ep1 = os.path.join(d, "1")
            os.makedirs(ep0)
            os.makedirs(ep1)
            with open(os.path.join(ep0, "cluster.xml"), "w", encoding="utf-8") as f:
                f.write(_xml_with_items(items=[("EPZERO.S", "true")]))
            nested = os.path.join(ep1, "nested")
            os.makedirs(nested)
            with open(os.path.join(nested, "deep.xml"), "w", encoding="utf-8") as f:
                f.write(_xml_with_items(items=[("EPONE.S", "true")]))
            pics = read_pics_from_file(d)
            self.assertTrue(pics["EPZERO.S"])
            self.assertTrue(pics["EPONE.S"])

    def test_zip_merges_xml_entries(self):
        with tempfile.TemporaryDirectory() as d:
            zpath = os.path.join(d, "pics_export.zip")
            with zipfile.ZipFile(zpath, "w") as zf:
                zf.writestr(
                    "0/first.xml",
                    _xml_with_items(items=[("ZIPZERO.S", "true")]),
                )
                zf.writestr(
                    "1/second.xml",
                    _xml_with_items(items=[("ZIPONE.S", "false")]),
                )
                zf.writestr(
                    "__MACOSX/ignored",
                    "not xml",
                )
            pics = read_pics_from_file(zpath)
            self.assertTrue(pics["ZIPZERO.S"])
            self.assertFalse(pics["ZIPONE.S"])

    def test_flat_directory_plus_endpoint_dirs(self):
        with tempfile.TemporaryDirectory() as d:
            with open(os.path.join(d, "root.xml"), "w", encoding="utf-8") as f:
                f.write(_xml_with_items(items=[("ROOTONLY.S", "true")]))
            ep0 = os.path.join(d, "0")
            os.makedirs(ep0)
            with open(os.path.join(ep0, "ep.xml"), "w", encoding="utf-8") as f:
                f.write(_xml_with_items(items=[("EPMERGE.S", "true")]))
            pics = read_pics_from_file(d)
            self.assertTrue(pics["ROOTONLY.S"])
            self.assertTrue(pics["EPMERGE.S"])


if __name__ == "__main__":
    unittest.main()
