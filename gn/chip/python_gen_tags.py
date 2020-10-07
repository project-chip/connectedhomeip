#!/usr/bin/env python
# Copyright (c) 2020 Project CHIP Authors
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

# Utility script to genertae the python tag + the abi tag + platform tag for a Python

try:
  from setuptools import pep425tags
except ImportError:
  from wheel import pep425tags

try:
  platform_tag = pep425tags.get_platform(None)
except TypeError:
  platform_tag = pep425tags.get_platform()

print(pep425tags.get_abbr_impl() + pep425tags.get_impl_ver() + "-" +
      pep425tags.get_abi_tag() + "-" + platform_tag)
