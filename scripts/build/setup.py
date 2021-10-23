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


"""The build_examples package."""

import setuptools  # type: ignore

setuptools.setup(
    name='build_examples',
    version='0.0.1',
    author='Project CHIP Authors',
    description='Generate build commands for the CHIP SDK Examples',
    packages=setuptools.find_packages(),
    package_data={'build_examples': ['py.typed']},
    zip_safe=False,
)
