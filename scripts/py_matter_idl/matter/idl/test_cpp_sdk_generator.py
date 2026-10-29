#!/usr/bin/env python3
# Copyright (c) 2026 Project CHIP Authors
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

import unittest

from matter.idl.generators.cpp.sdk.sdk_generator import extract_attribute_quality_flags
from matter.idl.matter_idl_parser import CreateParser
from matter.idl.matter_idl_types import Attribute


def ParseAttribute(attribute_idl: str) -> Attribute:
    idl = CreateParser(skip_meta=True).parse(f"server cluster TestCluster = 1 {{ {attribute_idl} }}")
    return idl.clusters[0].attributes[0]


class TestAttributeQualityFlags(unittest.TestCase):
    def test_no_reporting_qualities(self):
        self.assertEqual(extract_attribute_quality_flags(ParseAttribute("readonly attribute int8u a = 1;")), [])

    def test_changes_omitted(self):
        self.assertEqual(extract_attribute_quality_flags(ParseAttribute("readonly changes_omitted attribute int8u a = 1;")),
                         ['DataModel::AttributeQualityFlags::kChangesOmitted'])

    def test_fixed(self):
        self.assertEqual(extract_attribute_quality_flags(ParseAttribute("readonly fixed attribute int8u a = 1;")),
                         ['DataModel::AttributeQualityFlags::kFixed'])

    def test_with_other_qualities(self):
        flags = extract_attribute_quality_flags(ParseAttribute("timedwrite changes_omitted fixed attribute int8u a[] = 1;"))
        self.assertCountEqual(flags, ['DataModel::AttributeQualityFlags::kTimed',
                                      'DataModel::AttributeQualityFlags::kListAttribute',
                                      'DataModel::AttributeQualityFlags::kChangesOmitted',
                                      'DataModel::AttributeQualityFlags::kFixed'])


if __name__ == '__main__':
    unittest.main()
