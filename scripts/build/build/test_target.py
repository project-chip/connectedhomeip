#!/usr/bin/env python
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

import os
import sys
import unittest

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
from build.target import BuildTarget, TargetPart  # noqa: E402 isort:skip


class FakeBuilder:
    def __init__(self, **kargs):
        self.kargs = kargs


class TestGlobMatcher(unittest.TestCase):

    def test_one_fixed_target(self):
        t = BuildTarget('fake', FakeBuilder)
        t.AppendFixedTargets([
            TargetPart('foo', foo=1),
            TargetPart('bar', bar=2),
        ])

        self.assertEqual(t.HumanString(), "fake-{foo,bar}")

        self.assertIsNotNone(t.StringIntoTargetParts('fake-foo'))
        self.assertIsNotNone(t.StringIntoTargetParts('fake-bar'))
        self.assertIsNone(t.StringIntoTargetParts('fake-baz'))

    def test_fixed_targets(self):
        t = BuildTarget('fake', FakeBuilder)
        t.AppendFixedTargets([
            TargetPart('foo', foo=1),
            TargetPart('bar', bar=2),
        ])
        t.AppendFixedTargets([
            TargetPart('1', a=1),
            TargetPart('2', a=2),
            TargetPart('3', a=3),
        ])
        t.AppendFixedTargets([
            TargetPart('1', b=1),
            TargetPart('2', b=2),
            TargetPart('3', b=3),
        ])

        self.assertEqual(
            t.HumanString(),
            "fake-{foo,bar}-{1,2,3}-{1,2,3}"
        )

        self.assertIsNotNone(t.StringIntoTargetParts('fake-foo-1-2'))
        self.assertIsNotNone(t.StringIntoTargetParts('fake-foo-2-3'))
        self.assertIsNotNone(t.StringIntoTargetParts('fake-bar-3-1'))
        self.assertIsNone(t.StringIntoTargetParts('fake-bar-1'))
        self.assertIsNone(t.StringIntoTargetParts('fake-baz-1-2'))
        self.assertIsNone(t.StringIntoTargetParts('fake-foo-bar'))
        self.assertIsNone(t.StringIntoTargetParts('fake-1-2'))
        self.assertIsNone(t.StringIntoTargetParts('fake-foo'))
        self.assertIsNone(t.StringIntoTargetParts('fake-1-2-3'))

    def test_modifiers(self):
        t = BuildTarget('fake', FakeBuilder)
        t.AppendFixedTargets([
            TargetPart('foo', foo=1),
            TargetPart('bar', bar=2),
        ])

        t.AppendFixedTargets([
            TargetPart('one', value=1),
            TargetPart('two', value=2),
        ])

        t.AppendModifier('m1', m=1).ExceptIfRe('-m2')
        t.AppendModifier('m2', m=2).ExceptIfRe('-m1')
        t.AppendModifier('x1', x=1)
        t.AppendModifier('y1', x=1).OnlyIfRe('-foo-')

        self.assertEqual(
            t.HumanString(),
            "fake-{foo,bar}-{one,two}[-m1][-m2][-x1][-y1]"
        )

        self.assertEqual(
            set(t.AllVariants()),
            {
                'fake-foo-one',
                'fake-foo-one-m1',
                'fake-foo-one-m1-x1',
                'fake-foo-one-m1-x1-y1',
                'fake-foo-one-m1-y1',
                'fake-foo-one-m2',
                'fake-foo-one-m2-x1',
                'fake-foo-one-m2-x1-y1',
                'fake-foo-one-m2-y1',
                'fake-foo-one-x1',
                'fake-foo-one-x1-y1',
                'fake-foo-one-y1',
                'fake-foo-two',
                'fake-foo-two-m1',
                'fake-foo-two-m1-x1',
                'fake-foo-two-m1-x1-y1',
                'fake-foo-two-m1-y1',
                'fake-foo-two-m2',
                'fake-foo-two-m2-x1',
                'fake-foo-two-m2-x1-y1',
                'fake-foo-two-m2-y1',
                'fake-foo-two-x1',
                'fake-foo-two-x1-y1',
                'fake-foo-two-y1',
                'fake-bar-one',
                'fake-bar-one-m1',
                'fake-bar-one-m1-x1',
                'fake-bar-one-m2',
                'fake-bar-one-m2-x1',
                'fake-bar-one-x1',
                'fake-bar-two',
                'fake-bar-two-m1',
                'fake-bar-two-m1-x1',
                'fake-bar-two-m2',
                'fake-bar-two-m2-x1',
                'fake-bar-two-x1',
            }
        )

        self.assertIsNotNone(t.StringIntoTargetParts('fake-foo-one'))
        self.assertIsNotNone(t.StringIntoTargetParts('fake-bar-one-m1'))
        self.assertIsNotNone(t.StringIntoTargetParts('fake-foo-one-m2'))
        self.assertIsNotNone(t.StringIntoTargetParts('fake-bar-one-x1'))
        self.assertIsNotNone(t.StringIntoTargetParts('fake-foo-one-y1'))
        self.assertIsNotNone(t.StringIntoTargetParts('fake-foo-one-m1-y1'))

        self.assertIsNone(t.StringIntoTargetParts('fake-bar-one-m1-y1'))
        self.assertIsNone(t.StringIntoTargetParts('fake-foo-one-m1-m2'))
        self.assertIsNone(t.StringIntoTargetParts('fake-bar-m1'))
        self.assertIsNone(t.StringIntoTargetParts('fake-foo-x1-y1'))


if __name__ == '__main__':
    unittest.main()
