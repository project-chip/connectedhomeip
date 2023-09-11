# SPDX-FileCopyrightText: 2023 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0

from chip.yaml.runner import ReplTestRunner
from matter_yamltests.adapter import TestAdapter


class Adapter(TestAdapter):
    def __init__(self, specifications):
        self._adapter = ReplTestRunner(specifications, None, None)

    def encode(self, request):
        return self._adapter.encode(request)

    def decode(self, response):
        # TODO We should provide more meaningful logs here, but to adhere to
        # abstract function definition we do need to return list here.
        logs = []
        decoded_response = self._adapter.decode(response)
        if len(decoded_response) == 0:
            decoded_response = [{}]
        return decoded_response, logs
