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

import unittest

try:
    from build.target import *
except:
    import sys
    import os

    sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
    from build.target import *


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

        self.assertIsNotNone(t.StringIntoTargetParts('foo'))
        self.assertIsNotNone(t.StringIntoTargetParts('bar'))
        self.assertIsNone(t.StringIntoTargetParts('baz'))


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

        self.assertIsNotNone(t.StringIntoTargetParts('foo-1-2'))
        self.assertIsNotNone(t.StringIntoTargetParts('foo-2-3'))
        self.assertIsNotNone(t.StringIntoTargetParts('bar-3-1'))
        self.assertIsNone(t.StringIntoTargetParts('bar-1'))
        self.assertIsNone(t.StringIntoTargetParts('baz-1-2'))
        self.assertIsNone(t.StringIntoTargetParts('foo-bar'))
        self.assertIsNone(t.StringIntoTargetParts('1-2'))
        self.assertIsNone(t.StringIntoTargetParts('foo'))
        self.assertIsNone(t.StringIntoTargetParts('1-2-3'))

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

        t.AppendModifier(TargetPart('m1', m=1).ExceptIfRe('-m2'));
        t.AppendModifier(TargetPart('m2', m=2).ExceptIfRe('-m1'));
        t.AppendModifier(TargetPart('x1', x=1))
        t.AppendModifier(TargetPart('y1', x=1).OnlyIfRe('foo-'))

        self.assertIsNotNone(t.StringIntoTargetParts('foo-one'))
        self.assertIsNotNone(t.StringIntoTargetParts('bar-one-m1'))
        self.assertIsNotNone(t.StringIntoTargetParts('foo-one-m2'))
        self.assertIsNotNone(t.StringIntoTargetParts('bar-one-x1'))
        self.assertIsNotNone(t.StringIntoTargetParts('foo-one-y1'))
        self.assertIsNotNone(t.StringIntoTargetParts('foo-one-m1-y1'))

        self.assertIsNone(t.StringIntoTargetParts('bar-one-m1-y1'))
        self.assertIsNone(t.StringIntoTargetParts('foo-one-m1-m2'))
        self.assertIsNone(t.StringIntoTargetParts('bar-m1'))
        self.assertIsNone(t.StringIntoTargetParts('foo-x1-y1'))



if __name__ == '__main__':
    unittest.main()
