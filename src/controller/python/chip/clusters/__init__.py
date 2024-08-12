#
#    Copyright (c) 2021 Project CHIP Authors
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

import contextlib

from . import Attribute, CHIPClusters, Command, Objects
from .ClusterObjects import Cluster
from .Objects import Globals

__all__ = ['Attribute', 'CHIPClusters', 'Command', 'Globals']

# The Objects.py file is auto-generated and contains all the cluster classes.
# We dynamically add all cluster classes to the module's namespace and the
# __all__ list for global imports (from chip.clusters import *).
for obj in Objects.__dict__.values():
    with contextlib.suppress(AttributeError):
        if Cluster in obj.mro():
            globals()[obj.__name__] = obj
            __all__.append(obj.__name__)
