#
# SPDX-FileCopyrightText: 2022 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

class ActionCreationError(Exception):
    def __init__(self, message):
        super().__init__(message)


class UnexpectedActionCreationError(Exception):
    def __init__(self, message):
        super().__init__(message)


class ValidationError(Exception):
    def __init__(self, message):
        super().__init__(message)
