#!/usr/bin/env python3

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

import unittest

from glob_matcher import GlobMatcher


class TestGlobMatcher(unittest.TestCase):

    def test_exact_match(self):
        self.assertTrue(GlobMatcher('').matches(''))
        self.assertTrue(GlobMatcher('a').matches('a'))
        self.assertTrue(GlobMatcher('abc').matches('abc'))

        self.assertFalse(GlobMatcher('').matches('a'))
        self.assertFalse(GlobMatcher('abc').matches(''))
        self.assertFalse(GlobMatcher('x').matches('y'))
        self.assertFalse(GlobMatcher('abcd').matches('abcz'))
        self.assertFalse(GlobMatcher('abcdef').matches('abczef'))
        self.assertFalse(GlobMatcher('abc').matches('abcd'))
        self.assertFalse(GlobMatcher('abc').matches('bc'))
        self.assertFalse(GlobMatcher('abcd').matches('abc'))

    def test_questionmark(self):
        self.assertTrue(GlobMatcher('?').matches('a'))
        self.assertTrue(GlobMatcher('?x?z').matches('axyz'))
        self.assertTrue(GlobMatcher('a??').matches('abc'))

        self.assertFalse(GlobMatcher('??').matches('a'))
        self.assertFalse(GlobMatcher('??').matches('abc'))
        self.assertFalse(GlobMatcher('a').matches('?'))

    def test_star(self):
        self.assertTrue(GlobMatcher('*').matches(''))
        self.assertTrue(GlobMatcher('*').matches('a'))
        self.assertTrue(GlobMatcher('*').matches('abc'))
        self.assertTrue(GlobMatcher('*x').matches('abcx'))
        self.assertTrue(GlobMatcher('*x*').matches('x'))
        self.assertTrue(GlobMatcher('*x*').matches('xabc'))
        self.assertTrue(GlobMatcher('*x*').matches('abcx'))
        self.assertTrue(GlobMatcher('*x*').matches('abcxdef'))
        self.assertTrue(GlobMatcher('a*c').matches('abc'))
        self.assertTrue(GlobMatcher('a*c').matches('athisislongc'))
        self.assertTrue(GlobMatcher('123*').matches('123abc'))
        self.assertTrue(GlobMatcher('123*').matches('123'))
        self.assertTrue(GlobMatcher(
            'more*stars*here').matches('more_stars___here'))

        self.assertFalse(GlobMatcher('*x').matches('abc'))
        self.assertFalse(GlobMatcher('x*').matches('abc'))
        self.assertFalse(GlobMatcher(
            '*some*test').matches('thisissomelongertestcase'))

    def test_group(self):
        self.assertTrue(GlobMatcher('{a,b}').matches('a'))
        self.assertTrue(GlobMatcher('{a,b}').matches('b'))
        self.assertTrue(GlobMatcher('some{a,b}here').matches('someahere'))
        self.assertTrue(GlobMatcher('{a,b}x{c,d}').matches('axd'))
        self.assertTrue(GlobMatcher('{a,b}x{c,d}').matches('bxc'))

        self.assertFalse(GlobMatcher('{a,b}').matches(''))
        self.assertFalse(GlobMatcher('{a,b}').matches('c'))
        self.assertFalse(GlobMatcher('{a,b}').matches('ac'))
        self.assertFalse(GlobMatcher('{a,b}').matches('ca'))
        self.assertFalse(GlobMatcher('{a,b}x{c,d}').matches('axe'))
        self.assertFalse(GlobMatcher('{a,b}x{c,d}').matches('exd'))

    def test_combined(self):
        self.assertTrue(GlobMatcher('a{,bc}').matches('a'))
        self.assertTrue(GlobMatcher('a{,bc}').matches('abc'))
        self.assertTrue(GlobMatcher('ab{c*d,ef}xz').matches('abcdxz'))
        self.assertTrue(GlobMatcher('ab{c*d,ef}xz').matches('abc1234dxz'))
        self.assertTrue(GlobMatcher('ab{c*d,ef}xz').matches('abefxz'))

        self.assertFalse(GlobMatcher('a{,bc}').matches('ab'))
        self.assertFalse(GlobMatcher('a{,bc}').matches('ax'))
        self.assertFalse(GlobMatcher('a{,bc}').matches('abcd'))
        self.assertFalse(GlobMatcher('ab{c*d,ef}xz').matches('abxz'))
        self.assertFalse(GlobMatcher('ab{c*d,ef}xz').matches('abcxz'))
        self.assertFalse(GlobMatcher('ab{c*d,ef}xz').matches('abdxz'))
        self.assertFalse(GlobMatcher('ab{c*d,ef}xz').matches('abxz'))
        self.assertFalse(GlobMatcher('ab{c*d,ef}xz').matches('abexz'))
        self.assertFalse(GlobMatcher('ab{c*d,ef}xz').matches('abfxz'))


if __name__ == '__main__':
    unittest.main()
