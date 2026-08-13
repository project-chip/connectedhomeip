#
#    Copyright (c) 2020 Project CHIP Authors
#    Copyright (c) 2019 Google LLC.
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

"""Provides Python APIs for CHIP."""

# Allow Pigweed GN standalone Python tests to traverse split multi-directory PYTHONPATH packages
# (matter-core-module, matter-clusters-module, matter-testing-module) at load time.
#
# Pigweed python_runner.py explicitly mandates an __init__.py file to link folders into PYTHONPATH.
# Without extend_path, Python PEP 420 locks the namespace entirely to the first matching directory.
#
# TODO: Consolidate Sibling modular Python packages in BUILD.gn to avoid multi-directory namespace clobbering.
__path__ = __import__("pkgutil").extend_path(__path__, __name__)
