#
#    Copyright (c) 2020 Project CHIP Authors
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

class Nullable():
    def __repr__(self):
        return 'Null'

    def __eq__(self, other):
        if isinstance(other, Nullable):
            return True

        return False

    def __ne__(self, other):
        return not self.__eq__(other)

    def __lt__(self, other):
        return True


NullValue = Nullable()
