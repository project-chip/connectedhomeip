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

import distutils.util
import sys
import sysconfig

# TODO: pigweed virtualenv don't have pep425tags, will revisit it after figuring out with pigwee
# The below is temporary solution.
#from wheel.pep425tags import get_abbr_impl, get_impl_ver, get_abi_tag, get_platform
#print(get_abbr_impl() + get_impl_ver() + "-" + get_abi_tag() + "-" + get_platform())

abbr_ver = "cp"
impl_ver = str(sys.version_info[0]) + str(sys.version_info[1])
if sysconfig.get_config_var('SOABI') is None:
    abi_tag = abbr_ver + impl_ver
else:
    abi_tag = 'cp' + sysconfig.get_config_var('SOABI').split('-')[1]
platform_tag = distutils.util.get_platform().replace('.', '_').replace('-', '_')

print(abbr_ver + impl_ver + "-" + abi_tag + "-" + platform_tag)
