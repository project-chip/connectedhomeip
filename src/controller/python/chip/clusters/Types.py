#
# SPDX-FileCopyrightText: (c) 2020 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
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
